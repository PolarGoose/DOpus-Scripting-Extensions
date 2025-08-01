#pragma once

class ATL_NO_VTABLE CExifTool :
  public CComObjectRootEx<CComSingleThreadModel>,
  public CComCoClass<CExifTool, &CLSID_ExifTool>,
  public IDispatchImpl<IExifTool, &IID_IExifTool, &LIBID_DOpusScriptingExtensionsLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{
public:
  DECLARE_REGISTRY_RESOURCEID(IDR_ExifTool)
  BEGIN_COM_MAP(CExifTool)
    COM_INTERFACE_ENTRY(IExifTool)
    COM_INTERFACE_ENTRY(IDispatch)
  END_COM_MAP()
  DECLARE_PROTECT_FINAL_CONSTRUCT()

  HRESULT FinalConstruct() try {
    exifToolWrapper = ExifToolWrapper::GetInstance();
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(GetInfoAsJson)(BSTR fileFullName, IDispatch* tagNamesJsArray, BSTR* infoAsJson) override try {
    *infoAsJson = Copy(
      exifToolWrapper->GetTagInfosJson(fileFullName, ToUtf8StringVector(JsStringArrayToVector(tagNamesJsArray))));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

  STDMETHOD(Run)(BSTR fileFullName, IDispatch* commandLineArgs, BSTR* result) override try {
    *result = Copy(
      exifToolWrapper->Run(fileFullName, ToUtf8StringVector(JsStringArrayToVector(commandLineArgs))));
    return S_OK;
  } CATCH_ALL_EXCEPTIONS()

private:
  ExifToolWrapper* exifToolWrapper;
};

OBJECT_ENTRY_AUTO(__uuidof(ExifTool), CExifTool)
