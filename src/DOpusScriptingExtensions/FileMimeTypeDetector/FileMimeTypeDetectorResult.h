#pragma once

using namespace ATL;

class ATL_NO_VTABLE CFileMimeTypeDetectorResult :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CFileMimeTypeDetectorResult, &__uuidof(FileMimeTypeDetectorResult)>,
  public IDispatchImpl<IFileMimeTypeDetectorResult, &IID_IFileMimeTypeDetectorResult, &LIBID_DOpusScriptingExtensionsLib, 1, 0>
{
public:
  BEGIN_COM_MAP(CFileMimeTypeDetectorResult)
    COM_INTERFACE_ENTRY(IFileMimeTypeDetectorResult)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(get_MimeType)(BSTR* val) override try {
    *val = Copy(_mimeType);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_Encoding)(BSTR* val) override try {
    *val = Copy(_encoding);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  void Init(std::wstring mimeType, std::wstring encoding)
  {
    _mimeType = std::move(mimeType);
    _encoding = std::move(encoding);
  }

private:
  std::wstring _mimeType;
  std::wstring _encoding;
};
