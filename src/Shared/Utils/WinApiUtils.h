#pragma once

#include "Shared/Utils/Exceptions.h"

inline std::filesystem::path ExpandPathWithEnvironmentVariables(const wchar_t* const path) {
  // len is the number of TCHARs stored in the destination buffer, including the terminating null character
  const auto len = ExpandEnvironmentStrings(/* lpSrc */ path,
                                            /* lpDst */ nullptr,
                                            /* nSize */ 0);

  if (!len) {
    THROW_WEXCEPTION(L"Failed to calculate length for expanding the path '{}'", path);
  }

  const auto buffer = std::make_unique_for_overwrite<wchar_t[]>(len);
  if (!ExpandEnvironmentStrings(/* lpSrc */ path,
                                /* lpDst */ buffer.get(),
                                /* nSize */ len)) {
    THROW_WEXCEPTION(L"Failed to expand '{}'", path);
  }

  return std::filesystem::path{ buffer.get(), buffer.get() + len - 1 }; // -1 to exclude the terminating null character
}
