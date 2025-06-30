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

  STDMETHOD(GetI)(UINT streamKind, UINT streamNumber, UINT parameter, UINT infoKind, BSTR* res) override try {
    *res = Copy(
      mi.Get(static_cast<MediaInfoLib::stream_t>(streamKind), streamNumber,
             parameter, static_cast<MediaInfoLib::info_t>(infoKind)));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Count_Get)(UINT streamKind, UINT streamNumber, UINT64* res) override try {
    if (streamNumber == UINT_MAX) {
      *res = mi.Count_Get(static_cast<MediaInfoLib::stream_t>(streamKind));
      return S_OK;
    }

    *res = mi.Count_Get(static_cast<MediaInfoLib::stream_t>(streamKind), streamNumber);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Option)(BSTR option, BSTR value, BSTR* res) override try {
    *res = Copy(mi.Option(option, value));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Close)() override try {
    mi.Close();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Inform)(BSTR* res) override try {
    *res = Copy(mi.Inform());
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Option_Static)(BSTR option, BSTR value, BSTR* res) override try {
    *res = Copy(MediaInfoLib::MediaInfo::Option_Static(option, value));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(SetLanguage)(BSTR languageName) override try {
    const auto& languageFileContent = GetLanguageFileContent(languageName);
    mi.Option(L"Language", languageFileContent);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  std::wstring GetLanguageFileContent(const std::wstring_view& languageName) {
    const auto& languageFilePath = boost::dll::this_line_location().parent_path() / L"MediaInfoLanguages" / std::format(L"{}.csv", languageName);

    if(!boost::filesystem::exists(languageFilePath)) {
      THROW_WEXCEPTION(L"Language '{}' is not supported. The '{}' file doesn't exist", languageName, languageFilePath);
    }

    std::wifstream file(languageFilePath.c_str(), std::ios::binary);
    if (!file) {
      std::error_code ec(errno, std::generic_category());
      THROW_WEXCEPTION(L"Failed to open language file '{}'. Error message: {}", languageFilePath, ToWide(ec.message()));
    }

    return (std::wostringstream() << file.rdbuf()).str();
  }

  MediaInfoLib::MediaInfo mi;
};

OBJECT_ENTRY_AUTO(__uuidof(MediaInfoRetriever), CMediaInfoRetriever)


