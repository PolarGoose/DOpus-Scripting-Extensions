#pragma once

inline bool IsDriveAbsolutePath(const std::wstring_view path) {
  return path.size() >= 3
    && ((path[0] >= L'a' && path[0] <= L'z') || (path[0] >= L'A' && path[0] <= L'Z'))
    && path[1] == L':'
    && path[2] == L'\\';
}

inline std::wstring ExtendPathWithLongPathPrefix(const std::wstring_view path) {
  static constexpr std::wstring_view longPathPrefix = L"\\\\?\\";
  static constexpr std::wstring_view longUncPathPrefix = L"\\\\?\\UNC\\";
  static constexpr std::wstring_view uncPathPrefix = L"\\\\";
  static constexpr std::wstring_view devicePathPrefix = L"\\\\.\\";
  static constexpr std::wstring_view ntPathPrefix = L"\\??\\";

  std::wstring normalizedPath{ path };
  for (auto& ch : normalizedPath) {
    if (ch == L'/') {
      ch = L'\\';
    }
  }

  if (normalizedPath.empty()
      || normalizedPath.starts_with(longPathPrefix)
      || normalizedPath.starts_with(devicePathPrefix)
      || normalizedPath.starts_with(ntPathPrefix)) {
    return normalizedPath;
  }

  if (normalizedPath.starts_with(uncPathPrefix)) {
    return std::wstring{ longUncPathPrefix } + normalizedPath.substr(uncPathPrefix.size());
  }

  if (IsDriveAbsolutePath(normalizedPath)) {
    return std::wstring{ longPathPrefix } + normalizedPath;
  }

  return normalizedPath;
}

inline std::filesystem::path ExtendPathWithLongPathPrefix(const std::filesystem::path& path) {
  return std::filesystem::path{ ExtendPathWithLongPathPrefix(std::wstring_view{ path.c_str() }) };
}

inline boost::filesystem::path ExtendPathWithLongPathPrefix(const boost::filesystem::path& path) {
  return boost::filesystem::path{ ExtendPathWithLongPathPrefix(std::wstring_view{ path.c_str() }) };
}

inline boost::filesystem::path ExpandPathWithEnvironmentVariables(const wchar_t* const path) {
  const auto len = ExpandEnvironmentStrings(path, nullptr, 0); // len is the number of TCHARs stored in the destination buffer, including the terminating null character

  if (!len) {
    THROW_WEXCEPTION(L"Failed to calculate length for expanding the path '{}'", path);
  }

  const auto buffer = std::make_unique<wchar_t[]>(len);

  if (!ExpandEnvironmentStrings(path, buffer.get(), len)) {
    THROW_WEXCEPTION(L"Failed to expand '{}'", path);
  }

  return boost::filesystem::path{ buffer.get(), buffer.get() + len - 1 }; // -1 to exclude the terminating null character
}
