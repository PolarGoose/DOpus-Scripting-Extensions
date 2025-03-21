#pragma once

#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>
#include <atlsafe.h>
#include <atlstr.h>
#include <string>
#include <thread>
#include <sstream>
#include <format>
#include <utility>
#include <ranges>
#include <filesystem>
#include <boost/process/v2/process.hpp>
#include <boost/process/v2/stdio.hpp>
#include <boost/process/v2/windows/show_window.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include <boost/asio.hpp>

