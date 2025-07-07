#pragma once

inline std::string ToUtf8(std::wstring_view wideStr) {
  return boost::locale::conv::utf_to_utf<char>(
    wideStr.data(),
    wideStr.data() + wideStr.size()
  );
}

inline std::wstring ToUtf16(std::string_view utf8Str) {
  return boost::locale::conv::utf_to_utf<wchar_t>(
    utf8Str.data(),
    utf8Str.data() + utf8Str.size()
  );
}

inline std::vector<std::string> ToUtf8StringVector(const std::vector<std::wstring>& wideStrVector) {
  const auto& transformed = wideStrVector | std::views::transform(ToUtf8);
  return std::vector<std::string>(transformed.begin(), transformed.end());
}

inline const char* GetDataPointer(const boost::asio::streambuf& buf) {
  return static_cast<const char*>(buf.data().data());
}

// Allow std::format(L"..") to format std::filesystem::path and boost::filesystem::path
namespace std {
  template <>
  struct std::formatter<std::filesystem::path, wchar_t>
    : std::formatter<std::wstring, wchar_t> {
    auto format(const std::filesystem::path& path, wformat_context& ctx) const {
      return formatter<wstring, wchar_t>::format(path.c_str(), ctx);
    }
  };

  template <>
  struct std::formatter<boost::filesystem::path, wchar_t>
    : std::formatter<std::wstring, wchar_t> {
    auto format(const boost::filesystem::path& path, wformat_context& ctx) const {
      return formatter<wstring, wchar_t>::format(path.c_str(), ctx);
    }
  };
}
