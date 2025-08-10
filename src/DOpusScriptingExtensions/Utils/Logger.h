#pragma once

#define LINE_INFO std::format(L"{}:{}:{}", \
  std::filesystem::path(__builtin_FILE()).filename(), \
  ToUtf16(__builtin_FUNCTION()), \
  __builtin_LINE())

#ifdef DEBUG
  #define DEBUG_LOG(...) \
    do { \
      OutputDebugStringW( \
        std::format(L"DOpusScriptingExtensions(threadId={}):{}: {}", \
           std::this_thread::get_id(), \
           LINE_INFO, \
           std::format(__VA_ARGS__).c_str()).c_str()); \
    } while (0)
#else
  #define DEBUG_LOG(...) (void)(__VA_ARGS__)
#endif
