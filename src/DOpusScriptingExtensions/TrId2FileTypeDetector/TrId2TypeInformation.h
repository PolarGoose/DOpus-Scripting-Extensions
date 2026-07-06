#pragma once

class ATL_NO_VTABLE CTrId2TypeInformation :
  public ATL::CComObjectRootEx<ATL::CComSingleThreadModel>,
  public ATL::CComCoClass<CTrId2TypeInformation, &__uuidof(TrId2TypeInformation)>,
  public ATL::IDispatchImpl<ITrId2TypeInformation, &IID_ITrId2TypeInformation, &LIBID_DOpusScriptingExtensionsLib, 1, 0> {
public:
  BEGIN_COM_MAP(CTrId2TypeInformation)
    COM_INTERFACE_ENTRY(ITrId2TypeInformation)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  STDMETHOD(get_Name)(BSTR* val) override try {
    *val = Copy(_name);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_Description)(BSTR* val) override try {
    *val = Copy(_description);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_Url)(BSTR* val) override try {
    *val = Copy(_url);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_FileNameExtensions)(SAFEARRAY** val) override try {
    ATL::CComSafeArray<VARIANT> extensionsSafeArray;
    THROW_IF_FAILED_MSG(extensionsSafeArray.Create(static_cast<ULONG>(_fileNameExtensions.size())), L"Failed to allocate a SafeArray");

    for (size_t i = 0; i < _fileNameExtensions.size(); ++i) {
      ATL::CComVariant var(_fileNameExtensions[i].c_str());
      THROW_IF_FAILED_MSG(extensionsSafeArray.SetAt(static_cast<ULONG>(i), var), L"Failed to set a value at index {}", i);
    }

    *val = extensionsSafeArray.Detach();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_MimeType)(BSTR* val) override try {
    *val = Copy(_mimeType);
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(get_Probability)(double* val) override try {
    *val = _probability;
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  void Init(const TrId2Lib::FileType& fileType) {
    _name = ToUtf16(fileType.Name);
    _description = ToUtf16(fileType.Description);
    _url = ToUtf16(fileType.Url);
    _mimeType = ToUtf16(fileType.MimeType);
    _probability = fileType.Probability;

    _fileNameExtensions = fileType.FileNameExtensions
                          | std::views::transform([](const auto& extension) { return ToUtf16(extension); })
                          | std::ranges::to<std::vector<std::wstring>>();
  }

private:
  std::wstring _name;
  std::wstring _description;
  std::wstring _url;
  std::vector<std::wstring> _fileNameExtensions;
  std::wstring _mimeType;
  double _probability{};
};
