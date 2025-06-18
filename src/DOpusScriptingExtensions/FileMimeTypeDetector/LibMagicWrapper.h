#pragma once

class LibMagicWrapper : boost::noncopyable {
public:
  static const LibMagicWrapper* GetSingleInstance() {
    static const LibMagicWrapper instance{ MAGIC_MIME, boost::dll::this_line_location().parent_path() / L"magic.mgc" };
    return &instance;
  }

  std::wstring DetectFileType(const std::wstring_view fileFullName) const {
    const char* mimeCStr = magic_file(magicCookie, ToUtf8(fileFullName).c_str());
    if (!mimeCStr) {
      THROW_WEXCEPTION(L"Failed to detect type of a file '{}'. Error message: {}", fileFullName, GetError());
    }
    return ToWide(mimeCStr);
  }

  ~LibMagicWrapper() {
      magic_close(magicCookie);
  }

private:
  explicit LibMagicWrapper(const int flags, const boost::filesystem::path& magicMgcFile) : magicCookie(magic_open(flags)) {
    if (magicCookie == nullptr) {
      THROW_WEXCEPTION(L"Failed to initialize magic_t handle");
    }

    // This allows the std library to accept unicode file names.
    // It is needed for because LibMagic uses 'fopen' to open files.
    setlocale(LC_ALL, ".utf8");

    if (magic_load(magicCookie, ToUtf8(magicMgcFile.c_str()).c_str()) != 0) {
      magic_close(magicCookie);
      THROW_WEXCEPTION(L"Failed to load magic file '{}'. Error: {}", magicMgcFile, GetError());
    }
  }

  std::wstring GetError() const {
    const auto& err = magic_error(magicCookie);
    if (err == nullptr) {
      return L"";
    }
    return ToWide(magic_error(magicCookie));
  }

  magic_t magicCookie;
};


