#pragma once

#define LINE_INFO std::format(L"{}:{}:{}", \
                              std::filesystem::path(__builtin_FILE()).filename(), \
                              ToUtf16(__builtin_FUNCTION()), \
                              __builtin_LINE())
