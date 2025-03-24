#pragma once
#include "resource.h"
#include "ProcessRunner/ProcessRunnerResult.h"
#include "DOpusScriptingExtensions_i.h"
#include "Utils/ComUtils.h"
#include "Utils/WinApiUtils.h"

using namespace ATL;
class ATL_NO_VTABLE CProcessRunner :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CProcessRunner, &CLSID_ProcessRunner>,
  public IDispatchImpl<IProcessRunner, &IID_IProcessRunner, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0> {
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_PROCESSRUNNER)
  BEGIN_COM_MAP(CProcessRunner)
    COM_INTERFACE_ENTRY(IProcessRunner)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(Run)(BSTR executablePath, IDispatch* commandLineArgumentsJsArray, BSTR workingDirectory, IProcessRunnerResult** result) override try {
    std::vector<std::wstring> cmdWideArgs;
    if (commandLineArgumentsJsArray != 0)
    {
      cmdWideArgs = JsStringArrayToVector(*commandLineArgumentsJsArray);
    }

    const auto& cmdArgs = ToUtf8StringVector(cmdWideArgs);
    const auto& expandedPath = ExpandPathWithEnvironmentVariables(executablePath);
    const auto& [stdOut, stdErr, exitCode] = RunProcess(expandedPath, cmdArgs, workingDirectory);
    *result = CreateComObject<CProcessRunnerResult, IProcessRunnerResult>(
      [&](auto& obj) {
        obj.Init(CComBSTR(ToWide(stdOut).c_str()), CComBSTR(ToWide(stdErr).c_str()), exitCode);
      });
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  static std::tuple<std::string, std::string, int> RunProcess(const boost::filesystem::path& exePath,
                                                              std::vector<std::string> args,
                                                              std::wstring_view workingDirectory) {
    if (!boost::filesystem::exists(exePath))
    {
      THROW_WEXCEPTION(L"The executable not found '{}'", exePath);
    }

    if (!boost::process::v2::environment::detail::is_exec_type(exePath.c_str()))
    {
      THROW_WEXCEPTION(L"The file '{}' is not executable", exePath);
    }

    boost::asio::io_context ctx;
    boost::asio::streambuf outBuffer, errBuffer;
    boost::asio::readable_pipe stdOutPipe(ctx), stdErrPipe(ctx);
    boost::process::v2::process proc(
      ctx, boost::filesystem::canonical(exePath), args,
      boost::process::v2::process_stdio{ {}, stdOutPipe, stdErrPipe },
      boost::process::v2::windows::show_window_hide,
      boost::process::process_start_dir(workingDirectory));

    std::function<void()> readStdOut;
    readStdOut = [&]() {
      boost::asio::async_read(
        stdOutPipe, outBuffer,
        [&](const boost::system::error_code& ec, std::size_t /* bytes_transferred */) {
          if (!ec) {
            readStdOut();
          }
        }
      );
      };

    std::function<void()> readStdErr;
    readStdErr = [&]() {
      boost::asio::async_read(
        stdErrPipe, errBuffer,
        [&](const boost::system::error_code& ec, std::size_t /* bytes_transferred */) {
          if (!ec) {
            readStdErr();
          }
        }
      );
      };

    readStdOut();
    readStdErr();

    ctx.run();

    const auto& exitCode = proc.wait();

    return { std::string(
              boost::asio::buffers_begin(outBuffer.data()),
              boost::asio::buffers_end(outBuffer.data())),
            std::string(
              boost::asio::buffers_begin(errBuffer.data()),
              boost::asio::buffers_end(errBuffer.data())),
            exitCode };
  }
};

OBJECT_ENTRY_AUTO(__uuidof(ProcessRunner), CProcessRunner)
