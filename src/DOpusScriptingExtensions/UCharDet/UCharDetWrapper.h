#pragma once

class UCharDetWrapper : boost::noncopyable {
public:
  static UCharDetWrapper* GetInstance() {
    static UCharDetWrapper instance;
    return &instance;
  }

  // Returns an encoding name from the list https://www.freedesktop.org/wiki/Software/uchardet/
  // For example, "UTF-8", "ISO-8859-1", "Windows-1252", etc.
  std::wstring DetectFileEncoding(const std::filesystem::path& filePath, std::size_t maxBytesToInspect) const {
    const auto& fileBytes = ReadUpToMaxBytes(filePath, maxBytesToInspect);
    uchardet_reset(detector);
    uchardet_handle_data(detector, fileBytes.data(), fileBytes.size());
    uchardet_data_end(detector);
    return ToUtf16(uchardet_get_charset(detector));
  }

private:
  UCharDetWrapper() = default;

  ~UCharDetWrapper() {
    uchardet_delete(detector);
  }

  std::vector<char> ReadUpToMaxBytes(const std::filesystem::path& filePath, std::size_t maxBytesToRead) const {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
      std::error_code ec(errno, std::generic_category());
      THROW_WEXCEPTION(L"Unable to open file: '{}'. Error message: {}", filePath, ToUtf16(ec.message()));
    }

    std::vector<char> buffer(maxBytesToRead);
    file.read(buffer.data(), buffer.size());
    const auto& bytesRead = file.gcount();

    if (file.bad()) {
      std::error_code ec(errno, std::generic_category());
      THROW_WEXCEPTION(L"Error reading file: '{}'. Error message: {}", filePath, ToUtf16(ec.message()));
    }

    if (bytesRead == 0) {
      THROW_WEXCEPTION(L"File is empty: '{}'", filePath);
    }

    buffer.resize(bytesRead);
    return buffer;
  }

  uchardet_t detector = uchardet_new();
};
