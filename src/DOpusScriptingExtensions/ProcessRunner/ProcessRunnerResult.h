#pragma once
#include "resource.h"
#include "DOpusScriptingExtensions_i.h"

using namespace ATL;

class ATL_NO_VTABLE CProcessRunnerResult :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CProcessRunnerResult, &__uuidof(ProcessRunnerResult)>,
  public IDispatchImpl<IProcessRunnerResult, &IID_IProcessRunnerResult, &LIBID_DOpusScriptingExtensionsLib, 1, 0>
{
public:
  BEGIN_COM_MAP(CProcessRunnerResult)
    COM_INTERFACE_ENTRY(IProcessRunnerResult)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(get_StdOut)(BSTR* pVal) override
  {
    *pVal = stdOut.Copy();
    return S_OK;
  }

  STDMETHOD(get_StdErr)(BSTR* pVal) override
  {
    *pVal = stdErr.Copy();
    return S_OK;
  }

  STDMETHOD(get_ExitCode)(long* pVal) override
  {
    *pVal = exitCode;
    return S_OK;
  }

  // A helper method to initialize the result object with data.
  void Init(const CComBSTR& bstrStdOut, const CComBSTR& bstrStdErr, const long returnCode)
  {
    stdOut = bstrStdOut;
    stdErr = bstrStdErr;
    exitCode = returnCode;
  }

private:
  CComBSTR stdOut; 
  CComBSTR stdErr;
  long exitCode;
};
