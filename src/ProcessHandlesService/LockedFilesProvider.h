#pragma once

#include <grpc_generated/ProcessHandlesService.pb.h>
#include "Shared/Utils/StringUtils.h"
#include "ProcessHandlesService/ProcExp152Driver.h"
#include "ProcessHandlesService/NtDll.h"
#include "ProcessHandlesService/DevicePathToDrivePathConverter.h"

class LockedFilesProvider final : boost::noncopyable {
  ProcExp152Driver _procExp152Driver;
  NtDll _ntdll;

public:
  void GetLockingProcessInfos(ProcessHandlesService::LockingProcessInfos& out) const {
    DevicePathToDrivePathConverter devicePathToDrivePathConverter;

    std::optional<USHORT> fileObjectTypeIndex;
    auto processInfos = out.mutable_process_infos();
    for (const auto& handle : GetAllHandles()) {
      try {
        if (!processInfos->contains(handle.UniqueProcessId)) {
          processInfos->emplace(handle.UniqueProcessId, GetProcessInfo(handle.UniqueProcessId));
        }

        if(fileObjectTypeIndex && handle.ObjectTypeIndex != *fileObjectTypeIndex) {
          continue;
        }

        if (_procExp152Driver.GetHandleType(handle) != L"File") {
          continue;
        }

        fileObjectTypeIndex = handle.ObjectTypeIndex;

        auto handleName = _procExp152Driver.GetHandleName(handle);
        if (!handleName) {
          continue;
        }

        auto path = devicePathToDrivePathConverter.GetDriveLetterBasedFullName(*handleName);
        if (!path) {
          continue;
        }

        // Add `\\` at the end of the path if it is a directory. Ignore errors.
        std::error_code ec;
        if (std::filesystem::is_directory(*path, ec) && !path->ends_with(L'\\')) {
          path->push_back(L'\\');
        }

        processInfos->at(handle.UniqueProcessId).add_locked_files(ToUtf8(*path));
      }
      catch (const std::exception&) {
      }
     }
  }

private:
  std::generator<SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX> GetAllHandles() const {
    const auto& allHandles = _ntdll.QuerySystemHandleInformation();
    for (ULONG_PTR i = 0; i < allHandles->NumberOfHandles; i++) {
      co_yield allHandles->Handles[i];
    }
  }

  static std::generator<std::wstring> GetProcessModules(const HANDLE openedProcess) {
    for (const auto& moduleHandle : GetProcessModuleHandles(openedProcess)) {
      if (auto name = GetProcessModuleName(openedProcess, moduleHandle)) {
        co_yield std::move(*name);
      }
    }
  }

  auto GetProcessInfo(const ULONG_PTR pid) const {
    ProcessHandlesService::ProcessInfo processInfo;
    processInfo.set_process_id(pid);

    const auto& openedProcess = _procExp152Driver.OpenProcess(pid);
    if (openedProcess == nullptr || openedProcess.get() == INVALID_HANDLE_VALUE) {
      return processInfo;
    }

    auto processFullName = GetProcessFullName(openedProcess.get());
    if (processFullName) {
      processInfo.set_executable_full_name(ToUtf8(*processFullName));
    }

    auto userAndDomainName = GetUserAndDomainName(openedProcess.get());
    if (userAndDomainName) {
      const auto& [userName, domainName] = *userAndDomainName;
      processInfo.set_user_name(ToUtf8(userName));
      processInfo.set_domain_name(ToUtf8(domainName));
    }

    for(const auto& module : GetProcessModules(openedProcess.get())) {
      *processInfo.add_modules() = ToUtf8(module);
    }

    return processInfo;
  }

  static std::optional<std::wstring> GetProcessFullName(const HANDLE openedProcess) {
    for (DWORD capacity = 1024; ; capacity *= 2) {
      auto buffer = std::make_unique_for_overwrite<wchar_t[]>(capacity);
      DWORD size = capacity;

      if (!QueryFullProcessImageNameW(/* hProcess  */ openedProcess,
                                      /* dwFlags   */ 0,
                                      /* lpExeName */ buffer.get(),
                                      /* lpdwSize  */ &size)) {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
          return std::nullopt;
        }
        continue;
      }

      return std::wstring(buffer.get(), size);
    }
  }

  static std::optional<std::pair<std::wstring, std::wstring>> GetUserAndDomainName(const HANDLE openedProcess) {
    HANDLE token;
    if (!OpenProcessToken(/* ProcessHandle */ openedProcess,
                          /* DesiredAccess */ TOKEN_QUERY,
                          /* TokenHandle   */ &token)) {
      return std::nullopt;
    }
    
    ScopedHandle scopedToken{ token };
    
    DWORD tokenSize = 0;
    if (!GetTokenInformation(/* TokenHandle            */ scopedToken.get(),
                             /* TokenInformationClass  */ TokenUser,
                             /* TokenInformation       */ nullptr,
                             /* TokenInformationLength */ 0,
                             /* ReturnLength           */ &tokenSize)
         && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      return std::nullopt;
    }
    
    auto buf = std::make_unique_for_overwrite<std::byte[]>(tokenSize);
    if (!GetTokenInformation(/* TokenHandle            */ scopedToken.get(),
                             /* TokenInformationClass  */ TokenUser,
                             /* TokenInformation       */ buf.get(),
                             /* TokenInformationLength */ tokenSize,
                             /* ReturnLength           */ &tokenSize)) {
      return std::nullopt;
    }
    
    const auto* userInfo = reinterpret_cast<const TOKEN_USER*>(buf.get());
    
    DWORD userSize = 0;
    DWORD domainSize = 0;
    SID_NAME_USE sidName{};
    if (!LookupAccountSid(/* lpSystemName            */ nullptr,
                          /* Sid                     */ userInfo->User.Sid,
                          /* Name                    */ nullptr,
                          /* cchName                 */ &userSize,
                          /* ReferencedDomainName    */ nullptr,
                          /* cchReferencedDomainName */ &domainSize,
                          /* peUse                   */ &sidName)
        && GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      return std::nullopt;
    }
    
    auto user = std::make_unique_for_overwrite<wchar_t[]>(userSize);
    auto domain = std::make_unique_for_overwrite<wchar_t[]>(domainSize);
    
    if (!LookupAccountSid(/* lpSystemName            */ nullptr,
                          /* Sid                     */ userInfo->User.Sid,
                          /* Name                    */ user.get(),
                          /* cchName                 */ &userSize,
                          /* ReferencedDomainName    */ domain.get(),
                          /* cchReferencedDomainName */ &domainSize,
                          /* peUse                   */ &sidName)) {
      return std::nullopt;
    }
    
    return std::pair{ std::wstring(user.get()), std::wstring(domain.get()) };
  }

  static std::vector<HMODULE> GetProcessModuleHandles(const HANDLE openedProcess) {
    std::vector<HMODULE> modules(1024);
    for (;;) {
      DWORD bufferSizeNeededInBytes = 0;
      if (!EnumProcessModules(/* hProcess   */ openedProcess,
                              /* lphModule  */ modules.data(),
                              /* cb         */ static_cast<DWORD>(modules.size() * sizeof(HMODULE)),
                              /* lpcbNeeded */ &bufferSizeNeededInBytes)) {
        return {};
      }

      const auto& requiredNumberOfModules = bufferSizeNeededInBytes / sizeof(HMODULE);

      if (bufferSizeNeededInBytes <= modules.size() * sizeof(HMODULE)) {
        modules.resize(requiredNumberOfModules);
        return modules;
      }

      modules.resize(requiredNumberOfModules);
    }
  }

  static std::optional<std::wstring> GetProcessModuleName(const HANDLE openedProcess, const HMODULE processModuleHandle) {
    for (DWORD capacity = 1024; ; capacity *= 2) {
      auto buffer = std::make_unique_for_overwrite<wchar_t[]>(capacity);

      const auto& size = GetModuleFileNameExW(/* hProcess   */ openedProcess,
                                              /* hModule    */ processModuleHandle,
                                              /* lpFilename */ buffer.get(),
                                              /* nSize      */ capacity);

      if (size == 0) {
        return std::nullopt;
      }

      if (size == capacity) {
        continue;
      }

      return std::wstring(buffer.get(), size);
    }
  }
};
