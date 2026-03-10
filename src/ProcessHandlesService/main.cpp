#include "pch.h"
#include "ProcessHandlesService/ProcessHandlesServiceGrpcServer.h"

static const auto& g_serviceName = L"DOpusScriptingExtensions.ProcessHandlesService";
static absl::Notification g_stopEvent;

static void SetState(const SERVICE_STATUS_HANDLE serviceStatusHandle, const DWORD state, const DWORD win32ExitCode = NO_ERROR, const DWORD waitHintMs = 0) {
  SERVICE_STATUS st{ .dwServiceType = SERVICE_WIN32_OWN_PROCESS,
                     .dwCurrentState = state,
                     .dwControlsAccepted = (state == SERVICE_RUNNING) ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN) : (DWORD) 0,
                     .dwWin32ExitCode = win32ExitCode,
                     .dwServiceSpecificExitCode = 0,
                     .dwCheckPoint = 0,
                     .dwWaitHint = waitHintMs };
  SetServiceStatus(serviceStatusHandle, &st);
}

static DWORD WINAPI ServiceCtrlHandlerEx(const DWORD control, const DWORD /* eventType */, void* /* eventData */, void* /* context */) {
  switch (control) {
  case SERVICE_CONTROL_STOP:
  case SERVICE_CONTROL_SHUTDOWN:
    g_stopEvent.Notify();
    return NO_ERROR;
  default:
    return NO_ERROR;
  }
}

static void WINAPI ServiceMain(const DWORD /* argc */, LPWSTR* /* argv */) {
  const auto& serviceStatusHandle = RegisterServiceCtrlHandlerExW(g_serviceName, ServiceCtrlHandlerEx, nullptr);
  if (!serviceStatusHandle) return;

  SetState(serviceStatusHandle, SERVICE_RUNNING);

  g_stopEvent.WaitForNotification();

  SetState(serviceStatusHandle, SERVICE_STOPPED);
}

int main() {
  grpc::ServerBuilder builder;
  ProcessHandlesServiceGrpc processHandlesServiceGrpc;
  builder.AddListeningPort("127.0.0.1:43786", grpc::InsecureServerCredentials());
  builder.RegisterService(&processHandlesServiceGrpc);
  const auto grpcServer = builder.BuildAndStart();

  SERVICE_TABLE_ENTRYW table[] = { { const_cast<LPWSTR>(g_serviceName), ServiceMain },
                                   { nullptr, nullptr } };

  if (!StartServiceCtrlDispatcherW(table)) {
    return static_cast<int>(GetLastError());
  }

  return 0;
}
