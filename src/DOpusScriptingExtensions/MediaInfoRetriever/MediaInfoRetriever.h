#pragma once

using namespace ATL;
class ATL_NO_VTABLE CMediaInfoRetriever :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CMediaInfoRetriever, &CLSID_MediaInfoRetriever>,
  public IDispatchImpl<IMediaInfoRetriever, &IID_IMediaInfoRetriever, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_MediaInfoRetriever)
  BEGIN_COM_MAP(CMediaInfoRetriever)
    COM_INTERFACE_ENTRY(IMediaInfoRetriever)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(Open)(BSTR mediaFileFullName) override try {
    if (!std::filesystem::exists(mediaFileFullName)) {
      THROW_WEXCEPTION(L"File '{}' does not exist", mediaFileFullName);
    }

    if (mi.Open(mediaFileFullName) == 0) {
      THROW_WEXCEPTION(L"Failed to open a file '{}'", mediaFileFullName);
    }

    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Get)(UINT streamKind, UINT streamNumber, BSTR parameter, UINT infoKind, UINT searchKind, BSTR* res) override try {
    *res = Copy(
      mi.Get(static_cast<MediaInfoLib::stream_t>(streamKind), streamNumber,
             parameter, static_cast<MediaInfoLib::info_t>(infoKind), static_cast<MediaInfoLib::info_t>(searchKind)));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Close)() override try {
    mi.Close();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  MediaInfoLib::MediaInfo mi;
};

OBJECT_ENTRY_AUTO(__uuidof(MediaInfoRetriever), CMediaInfoRetriever)


