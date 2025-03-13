#pragma once
#include <string>
#include <boost/locale.hpp>

inline std::string ToUtf8(std::wstring_view wideStr) {
  return boost::locale::conv::utf_to_utf<char>(
    wideStr.data(),
    wideStr.data() + wideStr.size()
  );
}

inline std::wstring ToWide(std::string_view utf8Str) {
  return boost::locale::conv::utf_to_utf<wchar_t>(
    utf8Str.data(),
    utf8Str.data() + utf8Str.size()
  );
}

inline std::vector<std::string> ToUtf8StringVector(const std::vector<std::wstring>& wideStrVector) {
  const auto& transformed = wideStrVector | std::views::transform(ToUtf8);
  return std::vector<std::string>(transformed.begin(), transformed.end());
}
