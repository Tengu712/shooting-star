#pragma once

#ifdef __linux__
#  define VK_USE_PLATFORM_XLIB_KHR
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/keysym.h>
#  include <X11/XKBlib.h>
#  include <unistd.h>
#  include <fcntl.h>
#  include <linux/joystick.h>
#else
#  define VK_USE_PLATFORM_WIN32_KHR
#  include <windows.h>
#  include <xinput.h>
#endif

#include <vulkan/vulkan.h>
#include <string.h>

// a function wrapping stbi_load
unsigned char *image_file_to_bitmap(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
