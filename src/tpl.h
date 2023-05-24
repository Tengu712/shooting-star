#pragma once

#ifdef __linux__
#  define VK_USE_PLATFORM_XLIB_KHR
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#else
#  define VK_USE_PLATFORM_WIN32_KHR
#  include <windows.h>
#endif

#include <vulkan/vulkan.h>
#include <string.h>
