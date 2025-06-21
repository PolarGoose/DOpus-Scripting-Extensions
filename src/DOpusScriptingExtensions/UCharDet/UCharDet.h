#pragma once

class ATL_NO_VTABLE CUCharDet :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CUCharDet, &CLSID_UCharDet>,
  public IDispatchImpl<IUCharDet, &IID_IUCharDet, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_UCharDet)
  BEGIN_COM_MAP(CUCharDet)
    COM_INTERFACE_ENTRY(IUCharDet)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct() try {
    uCharDet = UCharDetWrapper::GetInstance();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(DetectFileEncoding)(BSTR fileFullName, UINT maxBytesToInspect, BSTR* fileEncoding) override try {
    *fileEncoding = Copy(
      uCharDet->DetectFileEncoding(
        std::filesystem::path(fileFullName), maxBytesToInspect));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  const UCharDetWrapper* uCharDet;
};

OBJECT_ENTRY_AUTO(__uuidof(UCharDet), CUCharDet)
