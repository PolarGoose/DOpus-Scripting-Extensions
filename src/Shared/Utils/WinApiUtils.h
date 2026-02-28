#pragma once

inline std::filesystem::path ExpandPathWithEnvironmentVariables(const wchar_t* const path) {
  const auto len = ExpandEnvironmentStrings(path, nullptr, 0); // len is the number of TCHARs stored in the destination buffer, including the terminating null character

  if (!len) {
    THROW_WEXCEPTION(L"Failed to calculate length for expanding the path '{}'", path);
  }

  const auto buffer = std::make_unique<wchar_t[]>(len);

  if (!ExpandEnvironmentStrings(path, buffer.get(), len)) {
    THROW_WEXCEPTION(L"Failed to expand '{}'", path);
  }

  return std::filesystem::path{ buffer.get(), buffer.get() + len - 1 }; // -1 to exclude the terminating null character
}
