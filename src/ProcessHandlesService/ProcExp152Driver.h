#pragma once

class ProcExp152Driver : boost::noncopyable {
private:
  ScopedHandle _driverFile;
  enum class IoctlCommand : DWORD {
    OpenProtectedProcessHandle = 2201288764, // 0x8335003C
    GetHandleName = 2201288776,              // 0x83350048
    GetHandleType = 2201288780               // 0x8335004C
  };

public:
  ProcExp152Driver() {
    // Load `PROCEXP152.SYS` driver
    // If it happens the first time, we use `CreateService`.
    // If the service already registered we use `OpenService`

    ScopedServiceHandle serviceManager{ OpenSCManager(/* lpMachineName   */ nullptr,
                                                      /* lpDatabaseName  */ nullptr,
                                                      /* dwDesiredAccess */ SC_MANAGER_CREATE_SERVICE) };
    if (!serviceManager) {
      THROW_WINAPI_EX(OpenSCManager);
    }

    const auto& serviceName = L"PROCEXP152";
    ScopedServiceHandle service{ CreateService(/* hSCManager         */ serviceManager.get(),
                                               /* lpServiceName      */ serviceName,
                                               /* lpDisplayName      */ L"Process Explorer",
                                               /* dwDesiredAccess    */ SERVICE_ALL_ACCESS,
                                               /* dwServiceType      */ SERVICE_KERNEL_DRIVER,
                                               /* dwStartType        */ SERVICE_DEMAND_START,
                                               /* dwErrorControl     */ SERVICE_ERROR_NORMAL,
                                               /* lpBinaryPathName   */ ExpandPathWithEnvironmentVariables(L"%WINDIR%\\System32\\drivers\\PROCEXP152.SYS").c_str(),
                                               /* lpLoadOrderGroup   */ nullptr,
                                               /* lpdwTagId          */ nullptr,
                                               /* lpDependencies     */ nullptr,
                                               /* lpServiceStartName */ nullptr,
                                               /* lpPassword         */ nullptr) };
    if (!service) {
      if (GetLastError() != ERROR_SERVICE_EXISTS) {
        THROW_WINAPI_EX(CreateService);
      }

      service.reset(OpenService(/* hSCManager      */ serviceManager.get(),
                                /* lpServiceName   */ serviceName,
                                /* dwDesiredAccess */ SERVICE_ALL_ACCESS));
      if (!service) {
        THROW_WINAPI_EX(OpenService);
      }
    }

    StartService(/* hService            */ service.get(),
                 /* dwNumServiceArgs    */ 0,
                 /* lpServiceArgVectors */ nullptr);

    _driverFile.reset(CreateFile(/* lpFileName            */ std::format(L"\\\\.\\{}", serviceName).c_str(),
                                 /* dwDesiredAccess       */ GENERIC_ALL,
                                 /* dwShareMode           */ 0,
                                 /* lpSecurityAttributes  */ nullptr,
                                 /* dwCreationDisposition */ OPEN_EXISTING,
                                 /* dwFlagsAndAttributes  */ FILE_ATTRIBUTE_NORMAL,
                                 /* hTemplateFile         */ nullptr));
    if (_driverFile.get() == INVALID_HANDLE_VALUE) {
      THROW_WINAPI_EX(CreateFile);
    }
  }

  std::optional<std::wstring> GetHandleName(const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& handle_info) const {
    return GetHandleNameOrType(IoctlCommand::GetHandleName, handle_info);
  }

  std::wstring GetHandleType(const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& handle_info) const {
    return GetHandleNameOrType(IoctlCommand::GetHandleType, handle_info).value_or(L"");
  }

  // Opens process using permissions:
  // * GENERIC_ALL for normal processes
  // * PROCESS_QUERY_LIMITED_INFORMATION for protected processes
  ScopedHandle OpenProcess(const ULONGLONG pid) const {
    HANDLE openedProcessHandle{};
    DWORD bytesReturned{};
    DeviceIoControl(/* hDevice         */ _driverFile.get(),
                    /* dwIoControlCode */ (DWORD)IoctlCommand::OpenProtectedProcessHandle,
                    /* lpInBuffer      */ (LPVOID)&pid,
                    /* nInBufferSize   */ sizeof(pid),
                    /* lpOutBuffer     */ &openedProcessHandle,
                    /* nOutBufferSize  */ sizeof(openedProcessHandle),
                    /* lpBytesReturned */ &bytesReturned,
                    /* lpOverlapped    */ nullptr);
    return ScopedHandle{ openedProcessHandle };
  }

private:
  std::optional<std::wstring> GetHandleNameOrType(const IoctlCommand getNameOrTypeIoControlCode, const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& handleInfo) const {
    struct PROCEXP_DATA_EXCHANGE {
      ULONGLONG Pid;
      void* ObjectAddress;
      ULONGLONG Size;
      HANDLE Handle;
    };

    PROCEXP_DATA_EXCHANGE data{ .Pid = handleInfo.UniqueProcessId,
                                .ObjectAddress = handleInfo.Object,
                                .Size = 0,
                                .Handle = handleInfo.HandleValue };

    wchar_t handleNameOrType[40 * 1000];
    DWORD bytesReturned{};
    const auto res = DeviceIoControl(/* hDevice         */ _driverFile.get(),
                                     /* dwIoControlCode */ (DWORD)getNameOrTypeIoControlCode,
                                     /* lpInBuffer      */ (LPVOID)&data,
                                     /* nInBufferSize   */ sizeof(data),
                                     /* lpOutBuffer     */ handleNameOrType,
                                     /* nOutBufferSize  */ sizeof(handleNameOrType),
                                     /* lpBytesReturned */ &bytesReturned,
                                     /* lpOverlapped    */ nullptr);

    // If the handle doesn't have a name, the driver returns 8 bytes. It indicates that it returned an empty string.
    if (!res || bytesReturned == 8) {
      return {};
    }

    return std::wstring{ handleNameOrType + 2 };
  }

  inline void EnablePrivilege(std::wstring_view privilege_name) {
    HANDLE raw_token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &raw_token)) {
      THROW_WINAPI_EX(OpenProcessToken);
    }

    ScopedHandle access_token{ raw_token };

    LUID privilege_id;
    if (!LookupPrivilegeValue(nullptr, privilege_name.data(), &privilege_id)) {
      THROW_WINAPI_EX(LookupPrivilegeValue);
    }

    const LUID_AND_ATTRIBUTES attributes{ .Luid = privilege_id, .Attributes = SE_PRIVILEGE_ENABLED };
    TOKEN_PRIVILEGES token{ .PrivilegeCount = 1 };
    token.Privileges[0] = attributes;

    if (!AdjustTokenPrivileges(access_token.get(), FALSE, &token, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr)) {
      THROW_WINAPI_EX(AdjustTokenPrivileges);
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
      THROW_WINAPI_EX(AdjustTokenPrivileges);
    }
  }
};
