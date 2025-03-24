#pragma once
#include "resource.h"
#include "DOpusScriptingExtensions_i.h"
#include "Utils/ComUtils.h"
#include "FileMimeTypeDetector/FileMimeTypeDetectorResult.h"
#include "FileMimeTypeDetector/LibMagicWrapper.h"

using namespace ATL;
class ATL_NO_VTABLE CFileMimeTypeDetector :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CFileMimeTypeDetector, &CLSID_FileMimeTypeDetector>,
  public IDispatchImpl<IFileMimeTypeDetector, &IID_IFileMimeTypeDetector, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_FILEMIMETYPEDETECTOR)
  BEGIN_COM_MAP(CFileMimeTypeDetector)
    COM_INTERFACE_ENTRY(IFileMimeTypeDetector)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct() try {
    const auto& magicFileFullName = boost::dll::this_line_location().parent_path() / L"magic.mgc";
    magicCookie.emplace(MAGIC_MIME, magicFileFullName.wstring());
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(DetectMimeType)(BSTR fileFullName, IFileMimeTypeDetectorResult** res) override try {
    if (!std::filesystem::exists(fileFullName))
    {
      THROW_HRESULT_MSG(E_FAIL, L"The file not found '{}'", fileFullName);
    }
    const auto& mimeTypeAndEncoding = magicCookie->DetectFileType(fileFullName);
    const auto& [mimeType, encoding] = ParseMimeTypeAndEncoding(mimeTypeAndEncoding);
    *res = CreateComObject<CFileMimeTypeDetectorResult, IFileMimeTypeDetectorResult>(
      [&](auto& pObj) {
        pObj.Init(CComBSTR(mimeType.c_str()), CComBSTR(encoding.c_str()));
      });
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  std::pair<std::wstring, std::wstring> ParseMimeTypeAndEncoding(std::wstring mimeTypeAndEncoding) {
    // The mimeTypeAndEncoding has values like "text/plain; charset=us-ascii"

    std::wregex pattern{ LR"(([^;]+);\scharset=(.+))" };
    std::wsmatch match;
    if (!std::regex_search(mimeTypeAndEncoding, match, pattern)) {
      THROW_HRESULT_MSG(E_FAIL, L"Failed to parse the mime type and encoding '{}'", mimeTypeAndEncoding);
    }
    return { match[1].str(), match[2].str() };
  }

  std::optional<LibMagicWrapper> magicCookie;
};

OBJECT_ENTRY_AUTO(__uuidof(FileMimeTypeDetector), CFileMimeTypeDetector)
