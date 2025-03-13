#pragma once
#include "resource.h"
#include "ProcessRunner/ProcessRunnerResult.h"
#include "DOpusScriptingExtensions_i.h"
#include "Utils/ComUtils.h"

using namespace ATL;
class ATL_NO_VTABLE CProcessRunner :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CProcessRunner, &CLSID_ProcessRunner>,
  public IDispatchImpl<IProcessRunner, &IID_IProcessRunner, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_PROCESSRUNNER)
  BEGIN_COM_MAP(CProcessRunner)
    COM_INTERFACE_ENTRY(IProcessRunner)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(Run)(BSTR executablePath, IDispatch* commandLineArgumentsJsArray, IProcessRunnerResult** result) override
  {
    try {
      const auto& cmdWideArgs = JsStringArrayToVector(*commandLineArgumentsJsArray);
      const auto& cmdArgs = ToUtf8StringVector(cmdWideArgs);
      const auto& [stdOut, stdErr, exitCode] = RunProcess(ToUtf8(executablePath), cmdArgs);
      const auto& pResult = CreateComObject<CProcessRunnerResult>();
      pResult->Init(CComBSTR(ToWide(stdOut).c_str()), CComBSTR(ToWide(stdErr).c_str()), exitCode);
      THROW_IF_FAILED(pResult->QueryInterface(IID_IProcessRunnerResult, reinterpret_cast<void**>(result)));
    }
    catch (const HResultException& ex) {
      AtlReportError(CLSID_ProcessRunner, ex.LMessage().data(), IID_IProcessRunner, ex.HResult());
      return ex.HResult();
    }
    catch (const std::exception& ex) {
      AtlReportError(CLSID_ProcessRunner, ex.what(), IID_IProcessRunner, E_FAIL);
      return E_FAIL;
    }

    return S_OK;
  }

private:
  static std::tuple<std::string, std::string, int> RunProcess(const std::string& exePath, std::vector<std::string> args) {
    boost::asio::io_context ctx;
    boost::asio::streambuf outBuffer, errBuffer;
    boost::asio::readable_pipe stdOutPipe(ctx), stdErrPipe(ctx);
    boost::process::v2::process proc(ctx, exePath, args, boost::process::v2::process_stdio{ {}, stdOutPipe, stdErrPipe });

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
