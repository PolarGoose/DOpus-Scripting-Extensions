#pragma once

class LockedFilesProvider final : boost::noncopyable {
  ProcExp152Driver _procExp152Driver;
  NtDll _ntdll;

public:
  void GetAllLockedFiles(ProcessHandlesService::LockedFilesAndProcessInfos& out) const {
    const DevicePathToDrivePathConverter devicePathToDrivePathConverter;

    for (const auto& handle : GetAllHandles()) {
      try {
        auto handleType = _procExp152Driver.GetHandleType(handle);
        if (handleType != L"File") {
          continue;
        }

        const auto& handleName = _procExp152Driver.GetHandleName(handle);
        if (!handleName) {
          continue;
        }

        auto path = devicePathToDrivePathConverter.GetDriveLetterBasedFullName(handleName.value());
        if (!path) {
          continue;
        }

        // Add `\\` at the end of the path if it is a directory. Ignore errors.
        std::error_code ec;
        if (std::filesystem::is_directory(path.value(), ec) && !path.value().ends_with(L'\\')) {
          path.value() += L"\\";
        }

        (*out.mutable_locked_files())[ToUtf8(path.value())].add_pids(handle.UniqueProcessId);

        if ((*out.mutable_processes()).contains(handle.UniqueProcessId)) {
          continue;
        }

        const auto& processFullName = getProcessFullName(handle.UniqueProcessId);
        if (!processFullName) {
          continue;
        }

        (*out.mutable_processes())[handle.UniqueProcessId] = ToUtf8(processFullName.value_or(L""));
      }
      catch (...) {
        /* ignore exceptions for an individual handle */
      }
    }
  }

private:
  std::generator<SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX> GetAllHandles() const {
    const auto allHandles = _ntdll.QuerySystemHandleInformation();
    for (ULONG_PTR i = 0; i < allHandles->NumberOfHandles; i++) {
      co_yield allHandles->Handles[i];
    }
  }

  std::optional<std::wstring> getProcessFullName(ULONG_PTR pid) const {
    if (pid == 0) {
      return L"System Idle Process";
    }

    if (pid == 4) {
      return L"System";
    }

    const auto& openedProcess = _procExp152Driver.OpenProcess(pid);
    if(openedProcess == nullptr || openedProcess.get() == INVALID_HANDLE_VALUE) {
      return {};
    }

    std::array<wchar_t, 40 * 1000> buf;
    DWORD resBufSize = static_cast<DWORD>(buf.size());
    const auto res = QueryFullProcessImageName(/* hProcess  */ openedProcess.get(),
                                               /* dwFlags   */ 0,
                                               /* lpExeName */ buf.data(),
                                               /* lpdwSize  */ &resBufSize);
    if (!res) {
      return {};
    }

    return std::wstring(buf.data(), resBufSize);
  }
};
