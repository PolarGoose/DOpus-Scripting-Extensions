#pragma once

inline ATL::CComVariant GetPropertyValue(IDispatch& obj, const std::wstring_view propName) {
  DISPID dispId = 0;
  LPOLESTR lpNames[] = { const_cast<LPOLESTR>(propName.data()) };
  THROW_IF_FAILED_MSG(
    obj.GetIDsOfNames(/* riid      */ IID_NULL,
                      /* rgszNames */ lpNames,
                      /* cNames    */ 1,
                      /* lcid      */ LOCALE_USER_DEFAULT,
                      /* rgDispId  */ &dispId),
    L"iDispatchObj doesn't have a property '{}'", propName);

  ATL::CComVariant result;
  DISPPARAMS dispParams = { 0 };
  THROW_IF_FAILED_MSG(
    obj.Invoke(/* dispIdMember */ dispId,
               /* riid         */ IID_NULL,
               /* lcid         */ LOCALE_USER_DEFAULT,
               /* wFlags       */ DISPATCH_PROPERTYGET,
               /* pDispParams  */ &dispParams,
               /* pVarResult   */ &result,
               /* pExcepInfo   */ NULL,
               /* puArgErr     */ NULL),
    L"Failed to get the value of the property '{}'", propName);

  return result;
}

template<typename CoClassType, typename QueryInterfaceType>
ATL::CComPtr<QueryInterfaceType> CreateComObject(std::function<void(CoClassType&)> initializer) {
  ATL::CComObject<CoClassType>* rawObj = nullptr;
  THROW_IF_FAILED_MSG(
    ATL::CComObject<CoClassType>::CreateInstance(&rawObj),
    L"Failed to create a COM object of type '{}'", ToUtf16(typeid(CoClassType).name()));

  // Wrap the object in CComPtr to ensure that it is released in case of an exception
  ATL::CComPtr<CoClassType> obj(rawObj);

  initializer(*obj);

  ATL::CComQIPtr<QueryInterfaceType> res(obj);
  if (!res) {
    THROW_HRESULT(
      E_NOINTERFACE,
      L"Failed to get the interface '{}' from '{}' class", ToUtf16(typeid(QueryInterfaceType).name()), ToUtf16(typeid(CoClassType).name()));
  }
  
  return res;
}

inline std::vector<std::wstring> JsStringArrayToVector(IDispatch* obj) {
  std::vector<std::wstring> result;

  if (obj == 0) {
    return result;
  }

  const auto& lengthVariant = GetPropertyValue(*obj, L"length");
  if (lengthVariant.vt != VT_I4) {
      THROW_WEXCEPTION(L"command line arguments 'length' property has a wrong variant type {}. Expected type is VT_I4", lengthVariant.vt);
  }

  for (int i = 0; i < lengthVariant.intVal; i++) {
    const auto& arg = GetPropertyValue(*obj, std::to_wstring(i));
    if (arg.vt != VT_BSTR) {
      THROW_WEXCEPTION(L"the element with the index {} is not a string but a variant type #{}", i, arg.vt);
    }
    result.emplace_back(arg.bstrVal);
  }

  return result;
}

inline BSTR Copy(std::wstring_view str) {
  const auto& res = SysAllocStringLen(str.data(), static_cast<UINT>(str.size()));
  if (res == nullptr) {
    THROW_WEXCEPTION(L"Failed to create a BSTR copy");
  }
  return res;
}
