#pragma once

namespace Priv {
struct HandleDeleter {
  using pointer = HANDLE;

  void operator()(const HANDLE h) {
    if (h == nullptr || h == INVALID_HANDLE_VALUE) {
      return;
    }
    CloseHandle(h);
  }
};

struct ServiceHandleDeleter {
  using pointer = SC_HANDLE;

  void operator()(const SC_HANDLE h) {
    if (h == nullptr || h == INVALID_HANDLE_VALUE) {
      return;
    }
    CloseServiceHandle(h);
  }
};
}

using ScopedHandle = std::unique_ptr<HANDLE, Priv::HandleDeleter>;
using ScopedServiceHandle = std::unique_ptr<SC_HANDLE, Priv::ServiceHandleDeleter>;
