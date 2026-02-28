#pragma once

class LockedFilesProvider final : boost::noncopyable {
  ProcExp152Driver _procExp152Driver;
  NtDll _ntdll;

public:
   GetAllLockedFiles() const {
    const device_name_to_drive_letter_map& path_conversion_map =
      create_device_name_to_drive_letter_convertion_map();

    if (!std::filesystem::exists(p)) {
      THROW(L"Path '{}' doesn't exist", p);
    }

    p = normalize_path(p);
    auto path_in_capital_letters = to_upper(p);

    std::map<ULONG_PTR, process_info> processes;
    const auto all_handles = ntdll.QuerySystemHandleInformation();
    for (ULONG_PTR i = 0; i < all_handles->NumberOfHandles; i++) {
      try {
        process_handle_if_it_is_a_file(processes, all_handles->Handles[i],
                                       path_in_capital_letters);
      } catch (const app_exception&) {
        /* ignore the exception */
      }
    }

    return to_vector(processes);
  }

private:
  void process_handle_if_it_is_a_file(
      std::map<ULONG_PTR, process_info>& processes,
      const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& handle,
      const std::wstring_view& path_in_all_capital_letters) const {
    auto handle_type = drv.get_handle_type(handle);
    if (handle_type != L"File") {
      return;
    }
    auto handle_path = get_handle_path(handle);
    auto handle_path_in_all_capital_letters = to_upper(handle_path);

    if (!handle_path_in_all_capital_letters.starts_with(
            path_in_all_capital_letters)) {
      return;
    }

    const auto& pid = handle.UniqueProcessId;
    if (!processes.contains(pid)) {
      const auto& proc =
          drv.open_process_handle_with_duplicate_handle_access_right(pid);
      processes.emplace(pid, get_process_info(proc.get(), pid));
    }

    processes[pid].locked_paths.emplace_back(handle_path);
  }

  [[nodiscard]] std::wstring
  get_handle_path(const SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX& handle_info) const {
    const auto& handle_name = drv.get_handle_name(handle_info);
    const auto& handle_path =
        get_drive_letter_based_full_name(path_conversion_map, handle_name);
    return normalize_path(handle_path);
  }

  std::vector<process_info> static to_vector(
      const std::map<ULONG_PTR, process_info>& processes) {
    std::vector<process_info> res;
    for (const auto& [_, process_info] : processes) {
      res.emplace_back(process_info);
    }
    return res;
  }

  inline auto open_process_token_with_query_access(const HANDLE opened_process) {
    HANDLE token{ nullptr };
    if (!OpenProcessToken(opened_process, TOKEN_QUERY, &token)) {
      THROW_WINAPI_FUNC_FAILED(OpenProcessToken);
    }
    return scoped_handle{ token };
  }

  inline auto get_token_user_information(HANDLE token) {
    DWORD tokenSize = 0;
    if (!GetTokenInformation(token, TokenUser, nullptr, 0, &tokenSize) &&
      GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      THROW_WINAPI_FUNC_FAILED(GetTokenInformation);
    }

    auto buf = std::make_unique<std::byte[]>(tokenSize);
    if (!GetTokenInformation(token, TokenUser, buf.get(), tokenSize,
      &tokenSize)) {
      THROW_WINAPI_FUNC_FAILED(GetTokenInformation);
    }

    return std::unique_ptr<TOKEN_USER>(
      reinterpret_cast<TOKEN_USER*>(buf.release()));
  }

  inline auto get_user_and_domain(const PSID sid) {
    DWORD userSize = 0;
    DWORD domainSize = 0;
    SID_NAME_USE sidName;
    if (!LookupAccountSid(nullptr, sid, nullptr, &userSize, nullptr, &domainSize,
      &sidName) &&
      GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
      THROW_WINAPI_FUNC_FAILED(LookupAccountSid);
    }

    auto user = std::make_unique<wchar_t[]>(userSize);
    auto domain = std::make_unique<wchar_t[]>(domainSize);
    if (!LookupAccountSid(nullptr, sid, user.get(), &userSize, domain.get(),
      &domainSize, &sidName)) {
      THROW_WINAPI_FUNC_FAILED(LookupAccountSid);
    }

    return std::pair{ std::wstring(user.get()), std::wstring(domain.get()) };
  }

  inline auto get_user_and_domain_name(const HANDLE opened_process) {
    const auto& token =
      priv::open_process_token_with_query_access(opened_process);
    const auto& user_info = priv::get_token_user_information(token.get());
    return priv::get_user_and_domain(user_info->User.Sid);
  }

  inline auto get_process_full_name(HANDLE opened_process) {
    const DWORD buf_size = 16'384;
    auto buf = std::make_unique<wchar_t[]>(buf_size);

    DWORD res_buf_size = buf_size;
    const auto res =
      QueryFullProcessImageName(opened_process, 0, buf.get(), &res_buf_size);
    if (res == 0 || res_buf_size == buf_size) {
      THROW_WINAPI_FUNC_FAILED(QueryFullProcessImageName);
    }
    return std::filesystem::path(buf.get());
  }

  inline process_info get_process_info(const HANDLE opened_proc,
    const UINT_PTR& pid) {
    if (pid == 0) {
      return process_info{ .pid = pid,
                          .process_full_name = L"System Idle Process",
                          .user = L"SYSTEM",
                          .domain = L"NT AUTHORITY" };
    }

    if (pid == 4) {
      return process_info{ .pid = pid,
                          .process_full_name = L"System",
                          .user = L"SYSTEM",
                          .domain = L"NT AUTHORITY" };
    }

    const auto& [user, domain] = priv::get_user_and_domain_name(opened_proc);
    const auto& name = priv::get_process_full_name(opened_proc);
    return process_info{
        .pid = pid, .process_full_name = name, .user = user, .domain = domain };
  }
};
