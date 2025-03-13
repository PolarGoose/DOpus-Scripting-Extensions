#pragma once
#include <AtlBase.h>
#include "Utils/Exceptions.h"

inline CComVariant GetPropertyValue(IDispatch& obj, const std::wstring_view propName)
{
  DISPID dispId = 0;
  LPOLESTR lpNames[] = { const_cast<LPOLESTR>(propName.data()) };
  THROW_IF_FAILED_MSG(
    obj.GetIDsOfNames(IID_NULL, lpNames, 1, LOCALE_USER_DEFAULT, &dispId),
    L"iDispatchObj doesn't have a property '{}'", propName);

  CComVariant result;
  DISPPARAMS dispParams = { 0 };
  THROW_IF_FAILED_MSG(
    obj.Invoke(dispId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dispParams, &result, NULL, NULL),
    L"Failed to get the value of the property '{}'", propName);

  return result;
}

template<typename T>
CComObject<T>* CreateComObject()
{
  CComObject<T>* obj = nullptr;
  THROW_IF_FAILED_MSG(
    CComObject<T>::CreateInstance(&obj),
    L"Failed to create a COM object of type '{}'", ToWide(typeid(T).name()));
  return obj;
}

inline std::vector<std::wstring> JsStringArrayToVector(IDispatch& obj)
{
  std::vector<std::wstring> result;

  const auto& lengthVariant = GetPropertyValue(obj, L"length");
  THROW_IF_VARIANT_TYPE_NOT(lengthVariant, VT_I4, L"command line arguments 'length' property has a wrong variant type {}. Expected type is VT_I4", lengthVariant.vt);

  for (int i = 0; i < lengthVariant.intVal; i++) {
    const auto& arg = GetPropertyValue(obj, std::to_wstring(i));
    THROW_IF_VARIANT_TYPE_NOT(arg, VT_BSTR, L"the element with the index {} is not a string but a variant type #{}", i, arg.vt);

    result.emplace_back(arg.bstrVal);
  }

  return result;
}
