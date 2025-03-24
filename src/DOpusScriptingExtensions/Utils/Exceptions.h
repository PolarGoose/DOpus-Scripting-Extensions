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
  catch (const WException& ex) { \
    AtlReportError(GetObjectCLSID(), ex.LMessage().data(), __uuidof(IUnknown), E_FAIL); \
    return E_FAIL; \
  } \
  catch (const std::exception& ex) { \
    AtlReportError(GetObjectCLSID(), ex.what(), __uuidof(IUnknown), E_FAIL); \
    return E_FAIL; \
  } \
  catch (...) { \
    AtlReportError(GetObjectCLSID(), L"Unknown exception", __uuidof(IUnknown), E_FAIL); \
    return E_FAIL; \
  }

#define DEBUG_LOG(...) \
  do { \
    ATLTRACE(std::format(__VA_ARGS__).c_str()); \
  } while (0)

#define THROW_WEXCEPTION(...) \
  do { \
    throw WException(std::format(__VA_ARGS__)); \
  } while (0)

#define THROW_HRESULT(hr, ...) \
  do { \
    throw HResultException((hr), std::format(__VA_ARGS__)); \
  } while (0)

#define THROW_IF_FAILED_MSG(hr, ...) \
  do { \
    const auto& _res = (hr); \
    if (FAILED(_res)) { \
      THROW_HRESULT(_res, __VA_ARGS__); \
    } \
  } while (0)

class WException : public std::exception
{
public:
  WException(const std::wstring_view msg, const std::source_location& loc = std::source_location::current())
    : msg(std::format(L"{}:{}: {}", std::filesystem::path(loc.file_name()).filename(), loc.line(), msg)) { }

  std::wstring_view LMessage() const { return msg; }

private:
  std::wstring msg;
};

class HResultException : public WException
{
public:
  HResultException(const HRESULT res) :
    HResultException(res, L"Failed") { }

  HResultException(const HRESULT res, const std::wstring_view msg) :
    WException(std::format(L"{}. HRESULT=0x{:X}: {}", msg, static_cast<unsigned long>(res), _com_error(res).ErrorMessage())),
    res(res) { }

  HRESULT HResult() const { return res; }

private:
  HRESULT res;
};
