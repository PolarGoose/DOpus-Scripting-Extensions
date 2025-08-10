#pragma once

#define CATCH_ALL_EXCEPTIONS() \
  catch (const HResultException& ex) { \
    AtlReportError(GetObjectCLSID(), ex.LMessage(), __uuidof(IUnknown), ex.HResult()); \
    return ex.HResult(); \
  } \
  catch (const WException& ex) { \
    AtlReportError(GetObjectCLSID(), ex.LMessage(), __uuidof(IUnknown), E_FAIL); \
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

#define THROW_WEXCEPTION(...) \
  do { \
    throw WException(std::format(__VA_ARGS__), LINE_INFO); \
  } while (0)

#define THROW_HRESULT(hr, ...) \
  do { \
    throw HResultException((hr), std::format(__VA_ARGS__), LINE_INFO); \
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
  WException(const std::wstring_view msg, const std::wstring_view lineInfo)
    : msg(std::format(L"{}: {}", lineInfo, msg)) { }

  auto LMessage() const { return msg.data(); }

private:
  std::wstring msg;
};

class HResultException : public WException
{
public:
  HResultException(const HRESULT res, const std::wstring_view msg, const std::wstring_view lineInfo) :
    WException(
      std::format(L"{}. HRESULT=0x{:08X}({}): {}", msg, static_cast<unsigned long>(res), static_cast<unsigned long>(res), _com_error(res).ErrorMessage()),
      lineInfo),
    res(res) { }

  auto HResult() const { return res; }

private:
  HRESULT res;
};
