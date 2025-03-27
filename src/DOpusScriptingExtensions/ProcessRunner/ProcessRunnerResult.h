#pragma once

using namespace ATL;

class ATL_NO_VTABLE CProcessRunnerResult :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CProcessRunnerResult, &__uuidof(ProcessRunnerResult)>,
  public IDispatchImpl<IProcessRunnerResult, &IID_IProcessRunnerResult, &LIBID_DOpusScriptingExtensionsLib, 1, 0> {
public:
  BEGIN_COM_MAP(CProcessRunnerResult)
    COM_INTERFACE_ENTRY(IProcessRunnerResult)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(get_StdOut)(BSTR* val) override try {
    *val = Copy(_stdOut);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_StdErr)(BSTR* val) override try {
    *val = Copy(_stdErr);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_ExitCode)(long* val) override try {
    *val = _exitCode;
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  void Init(std::wstring stdOut, std::wstring stdErr, const long exitCode) {
    _stdOut = std::move(stdOut);
    _stdErr = std::move(stdErr);
    _exitCode = exitCode;
  }

private:
  std::wstring _stdOut;
  std::wstring _stdErr;
  long _exitCode;
};
