#pragma once
#include <stdexcept>
#include <AtlBase.h>
#include <format>
#include <comdef.h>
#include "Utils/StringUtils.h"

#define CATCH_ALL_EXCEPTIONS() \
  catch (const HResultException& ex) { \
    AtlReportError(GetObjectCLSID(), ex.LMessage().data(), __uuidof(IUnknown), ex.HResult()); \
    return ex.HResult(); \
  } \
  catch (const std::exception& ex) { \
    AtlReportError(GetObjectCLSID(), ex.what(), __uuidof(IUnknown), E_FAIL); \
    return E_FAIL; \
  }

#define DEBUG_LOG(...) \
  do { \
    ATLTRACE(std::format(__VA_ARGS__).c_str()); \
  } while (0)

#define THROW_HRESULT(hr) \
  do { \
    throw HResultException(__LINE__, __FUNCTIONW__, (hr)); \
  } while (0)

#define THROW_HRESULT_MSG(hr, ...) \
  do { \
    throw HResultException(__LINE__, __FUNCTIONW__, (hr), std::format(__VA_ARGS__)); \
  } while (0)

#define THROW_IF_FAILED_MSG(hr, ...) \
  do { \
    const auto& _res = (hr); \
    if (FAILED(_res)) { \
      THROW_HRESULT_MSG(_res, __VA_ARGS__); \
    } \
  } while (0)

#define THROW_IF_FAILED(hr) \
  do { \
    const auto& res = (hr); \
    if (FAILED(res)) { \
      THROW_HRESULT(res); \
    } \
  } while (0)

#define THROW_IF_VARIANT_TYPE_NOT(variant, variantType, ...) \
  do { \
    const auto& _var = (variant); \
    const auto& _vt = (variantType); \
    if (_var.vt != _vt) { \
      THROW_HRESULT_MSG(DISP_E_TYPEMISMATCH, __VA_ARGS__); \
    } \
  } while (0)

class WException : public std::runtime_error
{
public:
  WException(std::wstring_view msg) : std::runtime_error(ToUtf8(msg)), msg(msg) { }

  std::wstring_view LMessage() const { return msg; }

private:
  std::wstring msg;
};

class HResultException : public WException
{
public:
  HResultException(int lineNumber, std::wstring_view funcName, HRESULT res) :
    WException(std::format(L"{}:{}: HRESULT=0x{:X}: {}", funcName, lineNumber, static_cast<unsigned long>(res), _com_error(res).ErrorMessage())),
    res(res) {
  }

  HResultException(int lineNumber, std::wstring_view funcName, HRESULT res, std::wstring_view msg) :
    WException(std::format(L"{}:{}: {}. HRESULT=0x{:X}: {}", funcName, lineNumber, msg, static_cast<unsigned long>(res), _com_error(res).ErrorMessage())),
    res(res) {
  }

  HRESULT HResult() const { return res; }

private:
  HRESULT res;
};
