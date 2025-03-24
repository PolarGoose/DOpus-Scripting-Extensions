#pragma once
#include "resource.h"
#include "DOpusScriptingExtensions_i.h"

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

  STDMETHOD(get_MimeType)(BSTR* val) override
  {
    *val = _mimeType.Copy();
    return S_OK;
  }

  STDMETHOD(get_Encoding)(BSTR* val) override
  {
    *val = _encoding.Copy();
    return S_OK;
  }

  void Init(const CComBSTR& mimeType, const CComBSTR& encoding)
  {
    _mimeType = mimeType;
    _encoding = encoding;
  }

private:
  CComBSTR _mimeType;
  CComBSTR _encoding;
};
