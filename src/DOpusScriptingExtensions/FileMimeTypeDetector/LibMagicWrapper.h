#pragma once
#include <Utils/NonCopyableAndNonMovable.h>

class LibMagicWrapper : NonCopyableAndNonMovable {
public:
  explicit LibMagicWrapper(int flags, std::wstring_view magicMagFileFullName) : magicCookie(magic_open(flags)) {
    if (magicCookie == nullptr) {
      THROW_HRESULT_MSG(E_FAIL, L"Failed to initialize magic_t handle");
    }
    if (magic_load(magicCookie, ToUtf8(magicMagFileFullName).c_str()) != 0) {
      THROW_HRESULT_MSG(E_FAIL, L"Failed to load magic file '{}'. Error: {}", magicMagFileFullName, GetError());
    }
  }

  std::wstring DetectFileType(std::wstring_view fileFullName) const {
    const char* mimeCStr = magic_file(magicCookie, ToUtf8(fileFullName).c_str());
    if (!mimeCStr) {
      THROW_HRESULT_MSG(E_FAIL, L"Failed to detect type of a file '{}'. Error message: {}", fileFullName, GetError());
    }
    return ToWide(mimeCStr);
  }

  ~LibMagicWrapper() {
      magic_close(magicCookie);
  }

private:
  std::wstring GetError() const {
    const auto& err = magic_error(magicCookie);
    if (err == nullptr) {
      return L"";
    }
    return ToWide(magic_error(magicCookie));
  }

  magic_t magicCookie;
};


