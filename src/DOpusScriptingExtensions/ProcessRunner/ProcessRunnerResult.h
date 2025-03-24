#pragma once
#include "resource.h"
#include "DOpusScriptingExtensions_i.h"

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

  STDMETHOD(get_StdOut)(BSTR* val) override {
    *val = _stdOut.Copy();
    return S_OK;
  }

  STDMETHOD(get_StdErr)(BSTR* val) override {
    *val = _stdErr.Copy();
    return S_OK;
  }

  STDMETHOD(get_ExitCode)(long* val) override {
    *val = _exitCode;
    return S_OK;
  }

  void Init(const CComBSTR& stdOut, const CComBSTR& stdErr, const long exitCode) {
    _stdOut = stdOut;
    _stdErr = stdErr;
    _exitCode = exitCode;
  }

private:
  CComBSTR _stdOut; 
  CComBSTR _stdErr;
  long _exitCode;
};
