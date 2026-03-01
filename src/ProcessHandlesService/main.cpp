#include "pch.h"
#include <gen/ProcessHandlesService.pb.h>
#include <gen/ProcessHandlesService.grpc.pb.h>
#include "Utils/StringUtils.h"
#include "Utils/Logger.h"
#include "Utils/Exceptions.h"
#include "Utils/WinApiUtils.h"
#include "Utils/ScopedHandle.h"
#include "Utils/EnablePrivilege.h"
#include "DevicePathToDrivePathConverter.h"
#include "NtDll.h"
#include "ProcExp152Driver.h"
#include "LockedFilesProvider.h"
#include "ProcessHandlesServiceGrpcServer.h"

static SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
static SERVICE_STATUS g_status = {};
static HANDLE g_stopEvent = nullptr;
static HANDLE g_workerThread = nullptr;

void ReportStatus(DWORD currentState, DWORD win32ExitCode = NO_ERROR, DWORD waitHintMs = 0)
{
  g_status.dwCurrentState = currentState;
  g_status.dwWin32ExitCode = win32ExitCode;
  g_status.dwWaitHint = waitHintMs;

  if (currentState == SERVICE_START_PENDING) {
    g_status.dwControlsAccepted = 0;
  }
  else {
    g_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
  }

  static DWORD checkpoint = 1;
  g_status.dwCheckPoint = (currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED) ? 0 : checkpoint++;

  SetServiceStatus(g_statusHandle, &g_status);
}

DWORD WINAPI WorkerThread(LPVOID)
{
  // Do your service work here. Check g_stopEvent periodically.
  while (WaitForSingleObject(g_stopEvent, 1000) == WAIT_TIMEOUT) {
    // TODO: your periodic work
    // e.g., poll a queue, run maintenance, etc.
  }
  return 0;
}

DWORD WINAPI ServiceCtrlHandlerEx(DWORD control, DWORD, LPVOID, LPVOID)
{
  switch (control) {
  case SERVICE_CONTROL_STOP:
  case SERVICE_CONTROL_SHUTDOWN:
    ReportStatus(SERVICE_STOP_PENDING, NO_ERROR, 3000);
    if (g_stopEvent) SetEvent(g_stopEvent);
    return NO_ERROR;
  default:
    return NO_ERROR;
  }
}

void WINAPI ServiceMain(DWORD /*argc*/, LPWSTR* /*argv*/)
{
  g_statusHandle = RegisterServiceCtrlHandlerExW(L"DOpusScriptingExtensions.ProcessHandlesService", ServiceCtrlHandlerEx, nullptr);
  if (!g_statusHandle) return;

  g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  g_status.dwServiceSpecificExitCode = 0;

  ReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

  g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
  if (!g_stopEvent) {
    ReportStatus(SERVICE_STOPPED, GetLastError());
    return;
  }

  g_workerThread = CreateThread(nullptr, 0, WorkerThread, nullptr, 0, nullptr);
  if (!g_workerThread) {
    CloseHandle(g_stopEvent);
    ReportStatus(SERVICE_STOPPED, GetLastError());
    return;
  }

  ReportStatus(SERVICE_RUNNING);

  // Wait until the worker finishes (after stop event is set)
  WaitForSingleObject(g_workerThread, INFINITE);

  CloseHandle(g_workerThread);
  CloseHandle(g_stopEvent);
  g_workerThread = nullptr;
  g_stopEvent = nullptr;

  ReportStatus(SERVICE_STOPPED);
}

int main()
{
  SERVICE_TABLE_ENTRYW table[] = {
      { const_cast<LPWSTR>(kServiceName), ServiceMain },
      { nullptr, nullptr }
  };

  // When run by SCM, this blocks until the service stops.
  if (!StartServiceCtrlDispatcherW(table)) {
    // If you run the EXE from a console, this commonly fails with ERROR_FAILED_SERVICE_CONTROLLER_CONNECT.
    // For debugging, you can add a "/console" mode and call WorkerThread directly.
  }
  return 0;
}
