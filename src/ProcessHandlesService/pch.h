#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>
#include <winternl.h>
#include <wtypes.h>
#include <string>
#include <thread>
#include <sstream>
#include <format>
#include <utility>
#include <ranges>
#include <filesystem>
#include <regex>
#include <map>
#include <boost/preprocessor.hpp>
#include <boost/locale.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/filesystem.hpp>
