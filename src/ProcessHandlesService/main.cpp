#include "pch.h"
#include "ProcessHandlesService/ProcessHandlesServiceGrpcImpl.h"

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
    SPDLOG_INFO("stop requested");
    g_stopEvent.Notify();
    return NO_ERROR;
  default:
    return NO_ERROR;
  }
}

static void WINAPI ServiceMain(const DWORD /* argc */, LPWSTR* /* argv */) {
  SPDLOG_INFO("ServiceMain start");
  const auto& serviceStatusHandle = RegisterServiceCtrlHandlerExW(g_serviceName, ServiceCtrlHandlerEx, nullptr);
  if (!serviceStatusHandle) return;

  SetState(serviceStatusHandle, SERVICE_RUNNING);

  SPDLOG_INFO("ServiceMain wait for stop");
  g_stopEvent.WaitForNotification();

  SPDLOG_INFO("ServiceMain stop signal received");
  SetState(serviceStatusHandle, SERVICE_STOPPED);
}

int main() {
  spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e][%s(%#)][thread %t] %v");
  spdlog::set_default_logger(spdlog::rotating_logger_mt(/* logger_name   */ "ProcessHandlesService logger",
                                                        /* filename      */ (boost::dll::this_line_location().parent_path() / "DOpusScriptingExtensions.ProcessHandlesService.log.txt").string(),
                                                        /* max_file_size */ 1024 * 1024, /* 1 MiB */
                                                        /* max_files     */ 2));
  SPDLOG_INFO("Start GRPC server on port 43786");
  try {
    grpc::ServerBuilder builder;
    ProcessHandlesServiceGrpcImpl processHandlesServiceGrpc;
    builder.AddListeningPort("127.0.0.1:43786", grpc::InsecureServerCredentials());
    builder.RegisterService(&processHandlesServiceGrpc);
    const auto grpcServer = builder.BuildAndStart();
    SPDLOG_INFO("GRPC server started");
  } catch (const std::exception& ex) {
    SPDLOG_ERROR("Failed to start GRPC server: {}", ex.what());
    return -1;
  }

  SERVICE_TABLE_ENTRYW table[] = { { const_cast<LPWSTR>(g_serviceName), ServiceMain },
                                   { nullptr, nullptr } };

  if (!StartServiceCtrlDispatcherW(table)) {
    return static_cast<int>(GetLastError());
  }

  SPDLOG_INFO("Exit service");
  return 0;
}
