#pragma once

using namespace ATL;
class ATL_NO_VTABLE CFileMimeTypeDetector :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CFileMimeTypeDetector, &CLSID_FileMimeTypeDetector>,
  public IDispatchImpl<IFileMimeTypeDetector, &IID_IFileMimeTypeDetector, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_FileMimeTypeDetector)
  BEGIN_COM_MAP(CFileMimeTypeDetector)
    COM_INTERFACE_ENTRY(IFileMimeTypeDetector)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct() try {
    magicCookie = LibMagicWrapper::GetSingleInstance();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(DetectMimeType)(BSTR fileFullName, IFileMimeTypeDetectorResult** res) override try {
    if (!std::filesystem::exists(fileFullName))
    {
      THROW_WEXCEPTION(L"The file not found '{}'", fileFullName);
    }
    const auto& mimeTypeAndEncoding = magicCookie->DetectFileType(fileFullName);
    const auto& [mimeType, encoding] = ParseMimeTypeAndEncoding(mimeTypeAndEncoding);
    *res = CreateComObject<CFileMimeTypeDetectorResult, IFileMimeTypeDetectorResult>(
      [&](auto& pObj) {
        pObj.Init(std::move(mimeType), std::move(encoding));
      }).Detach();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  std::pair<std::wstring, std::wstring> ParseMimeTypeAndEncoding(const std::wstring& mimeTypeAndEncoding) {
    // The mimeTypeAndEncoding has values like "text/plain; charset=us-ascii"

    static const std::wregex pattern{ LR"(([^;]+);\scharset=(.+))" };
    std::wsmatch match;
    if (!std::regex_search(mimeTypeAndEncoding, match, pattern)) {
      THROW_WEXCEPTION(L"Failed to parse the mime type and encoding '{}'", mimeTypeAndEncoding);
    }
    return { match[1].str(), match[2].str() };
  }

  const LibMagicWrapper* magicCookie;
};

OBJECT_ENTRY_AUTO(__uuidof(FileMimeTypeDetector), CFileMimeTypeDetector)
