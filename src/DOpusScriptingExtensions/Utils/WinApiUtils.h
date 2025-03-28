#pragma once

inline boost::filesystem::path ExpandPathWithEnvironmentVariables(const wchar_t* const path) {
  const auto len = ExpandEnvironmentStrings(path, nullptr, 0);

  if (!len) {
    THROW_WEXCEPTION(L"Failed to calculate length for expanding the path '{}'", path);
  }

  const auto buffer = std::make_unique<wchar_t[]>(len);

  if (!ExpandEnvironmentStrings(path, buffer.get(), len)) {
    THROW_WEXCEPTION(L"Failed to expand '{}'", path);
  }

  return boost::filesystem::path{ buffer.get() };
}
