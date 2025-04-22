#pragma once

using namespace ATL;
class ATL_NO_VTABLE CProcessRunner :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CProcessRunner, &CLSID_ProcessRunner>,
  public IDispatchImpl<IProcessRunner, &IID_IProcessRunner, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0> {
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_ProcessRunner)
  BEGIN_COM_MAP(CProcessRunner)
    COM_INTERFACE_ENTRY(IProcessRunner)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(Run)(BSTR executablePath, IDispatch* commandLineArgumentsJsArray, BSTR workingDirectory, IProcessRunnerResult** result) override try {
    auto [stdOut, stdErr, exitCode] = RunProcess(
      ExpandPathWithEnvironmentVariables(executablePath),
      ToUtf8StringVector(JsStringArrayToVector(commandLineArgumentsJsArray)),
      ExpandPathWithEnvironmentVariables(workingDirectory));
    *result = CreateComObject<CProcessRunnerResult, IProcessRunnerResult>(
      [&](auto& obj) {
        obj.Init(std::move(stdOut), std::move(stdErr), exitCode);
      }).Detach();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  static std::tuple<std::wstring, std::wstring, int> RunProcess(const boost::filesystem::path& exePath,
                                                                const std::vector<std::string>& args,
                                                                const boost::filesystem::path& workingDirectory) {
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

    return { ToWide({ static_cast<const char*>(outBuffer.data().data()), outBuffer.size() }),
             ToWide({ static_cast<const char*>(errBuffer.data().data()), errBuffer.size() }),
            exitCode };
  }
};

OBJECT_ENTRY_AUTO(__uuidof(ProcessRunner), CProcessRunner)
