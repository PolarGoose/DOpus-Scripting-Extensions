#pragma once

class ATL_NO_VTABLE CTrId2FileTypeDetector :
  public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
  public ATL::CComCoClass<CTrId2FileTypeDetector, &CLSID_TrId2FileTypeDetector>,
  public ATL::IDispatchImpl<ITrId2FileTypeDetector, &IID_ITrId2FileTypeDetector, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_TrId2FileTypeDetector)
  BEGIN_COM_MAP(CTrId2FileTypeDetector)
    COM_INTERFACE_ENTRY(ITrId2FileTypeDetector)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(DetectFileType)(const BSTR fileFullName, const UINT numberOfCandidates, SAFEARRAY **fileTypeGuesses) override try {
    const auto extendedFileFullName = ExtendPathWithLongPathPrefix(std::wstring_view{ fileFullName });
    const auto candidates = TrId2Lib::detectFileType(extendedFileFullName, numberOfCandidates);

    ATL::CComSafeArray<VARIANT> fileTypeGuessesSafeArray;
    THROW_IF_FAILED_MSG(fileTypeGuessesSafeArray.Create(static_cast<ULONG>(candidates.size())), L"Failed to allocate a SafeArray");

    for (size_t i = 0; i < candidates.size(); ++i) {
      auto fileTypeInformation = CreateComObject<CTrId2TypeInformation, ITrId2TypeInformation>([&](auto& obj) { obj.Init(candidates[i]); });
      ATL::CComVariant var;
      var.vt = VT_DISPATCH;
      var.pdispVal = fileTypeInformation.Detach();
      THROW_IF_FAILED_MSG(fileTypeGuessesSafeArray.SetAt(static_cast<ULONG>(i), var), L"Failed to set a value at index {}", i);
    }

    *fileTypeGuesses = fileTypeGuessesSafeArray.Detach();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()
};

OBJECT_ENTRY_AUTO(__uuidof(TrId2FileTypeDetector), CTrId2FileTypeDetector)
