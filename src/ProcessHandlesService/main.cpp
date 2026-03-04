#include "pch.h"
#include <gen/ProcessHandlesService.pb.h>
#include <gen/ProcessHandlesService.grpc.pb.h>
#include "Utils/StringUtils.h"
#include "Utils/Logger.h"
#include "Utils/Exceptions.h"
#include "Utils/WinApiUtils.h"
#include "Utils/ScopedHandle.h"
#include "DevicePathToDrivePathConverter.h"
#include "NtDll.h"
#include "ProcExp152Driver.h"
#include "LockedFilesProvider.h"
#include "ProcessHandlesServiceGrpcServer.h"

static constexpr wchar_t SERVICE_NAME[] = L"ProcessHandlesGrpcSvc";
static SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
static absl::Notification g_stopEvent;

static void SetState(DWORD state, DWORD win32ExitCode = NO_ERROR, DWORD waitHintMs = 0) {
  SERVICE_STATUS st{};
  st.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  st.dwCurrentState = state;
  st.dwControlsAccepted = (state == SERVICE_RUNNING) ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN) : 0;
  st.dwWin32ExitCode = win32ExitCode;
  st.dwServiceSpecificExitCode = 0;
  st.dwCheckPoint = 0;
  st.dwWaitHint = waitHintMs;
  SetServiceStatus(g_statusHandle, &st);
}

static DWORD WINAPI ServiceCtrlHandlerEx(DWORD control, DWORD /*eventType*/, void* /*eventData*/, void* /*context*/)
{
  switch (control) {
  case SERVICE_CONTROL_STOP:
  case SERVICE_CONTROL_SHUTDOWN:
    g_stopEvent.Notify();
    return NO_ERROR;
  default:
    return NO_ERROR;
  }
}

static void WINAPI ServiceMain(DWORD /* argc */, LPWSTR* /* argv */)
{
  try {
    grpc::ServerBuilder builder;
    ProcessHandlesServiceGrpc processHandlesServiceGrpc;
    builder.AddListeningPort("127.0.0.1:43786", grpc::InsecureServerCredentials());
    builder.RegisterService(&processHandlesServiceGrpc);
    const auto grpcServer = builder.BuildAndStart();
  } catch(const std::exception& ex) {
    DEBUG_LOG("Failed to start gRPC server: {}", ex.what());
    SetState(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR);
    return;
  }

  g_statusHandle = RegisterServiceCtrlHandlerExW(SERVICE_NAME, ServiceCtrlHandlerEx, nullptr);
  if (!g_statusHandle) return;

  SetState(SERVICE_RUNNING);

  g_stopEvent.WaitForNotification();

  SetState(SERVICE_STOPPED);
}

int main()
{
  SERVICE_TABLE_ENTRYW table[] = {
      { const_cast<LPWSTR>(SERVICE_NAME), ServiceMain },
      { nullptr, nullptr }
  };

  if (!StartServiceCtrlDispatcherW(table)) {
    return static_cast<int>(GetLastError());
  }

  return 0;
}
