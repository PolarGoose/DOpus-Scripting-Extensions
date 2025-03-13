#pragma once
#include <stdexcept>
#include <AtlBase.h>
#include <format>
#include <comdef.h>
#include "Utils/StringUtils.h"

#define THROW_IF_FAILED_MSG(hr, ...) \
  do { \
    const auto& res = (hr); \
    if (FAILED(res)) { \
      throw HResultException(__LINE__, __FUNCTIONW__, res, std::format(__VA_ARGS__)); \
    } \
  } while (0)

#define THROW_IF_FAILED(hr) \
  do { \
    const auto& res = (hr); \
    if (FAILED(res)) { \
      throw HResultException(__LINE__, __FUNCTIONW__, res); \
    } \
  } while (0)

#define THROW_IF_VARIANT_TYPE_NOT(variant, variantType, ...) \
  do { \
    const auto& _var = (variant); \
    const auto& _vt = (variantType); \
    if (_var.vt != _vt) { \
      throw HResultException(__LINE__, __FUNCTIONW__, DISP_E_TYPEMISMATCH, std::format(__VA_ARGS__)); \
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
