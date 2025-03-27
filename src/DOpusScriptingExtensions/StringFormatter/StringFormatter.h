#pragma once

using namespace ATL;
class ATL_NO_VTABLE CStringFormatter :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CStringFormatter, &CLSID_StringFormatter>,
  public IDispatchImpl<IStringFormatter, &IID_IStringFormatter, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_STRINGFORMATTER)
  BEGIN_COM_MAP(CStringFormatter)
    COM_INTERFACE_ENTRY(IStringFormatter)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(Format)(BSTR formatString, BSTR arg1, BSTR arg2, BSTR arg3,
                    BSTR arg4, BSTR arg5, BSTR arg6, BSTR arg7, BSTR arg8, BSTR arg9,
                    BSTR arg10, BSTR arg11, BSTR arg12, BSTR* res) override try {
    const auto& formattedString = std::vformat(formatString, std::make_wformat_args(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11, arg12));
    *res = Copy(formattedString);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()
};

OBJECT_ENTRY_AUTO(__uuidof(StringFormatter), CStringFormatter)
