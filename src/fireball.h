#pragma once

#ifdef _WIN32
#define EXPORT __declspec(dllexport) __stdcall
#else
// TODO: for Linux
#define EXPORT
#endif

#include <stdint.h>

// ================================================================================================================= //
//     Logger                                                                                                        //
// ================================================================================================================= //

typedef int warn_t;
#define FB_WARN 0
#define FB_SUCCESS 1

EXPORT warn_t fb_init_logger(void);
EXPORT void fb_indent_logger(void);
EXPORT void fb_dedent_logger(void);
EXPORT void fb_error(const char *msg);
EXPORT warn_t fb_warning(const char *msg);
EXPORT void fb_info(const char *msg);
EXPORT void fb_debug(const char *format, ...);

// ================================================================================================================= //
//     Fireball                                                                                                      //
// ================================================================================================================= //

// A function to initialize Fireball.
// This parameters are:
//   1. title: window title
//   2. width: window width
//   3. height: window height
//   4. max_image_num: the max number of image textures
EXPORT warn_t fb_init(const char *title, uint16_t width, uint16_t height, uint32_t max_image_num);

// A function to terminate Fireball.
EXPORT void fb_terminate();

// A function to process window event and check if the app should be closed.
EXPORT warn_t fb_should_close();

// A function to render entities.
// This parameters are:
//   1. r: red for clearing the screen
//   2. g: green for clearing the screen
//   3. b: blue for clearing the screen
// TODO:
EXPORT warn_t fb_render(float r, float g, float b);
