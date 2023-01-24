#pragma once

// Getters
void *skd_get_connection(void);
unsigned int skd_get_window(void);

// A function to get error message.
const char *skd_get_window_error_message(int res);

// A function to create a window.
// It returns 0 if it succeeded.
int skd_create_window(const char *title, unsigned short width, unsigned short height);

// A function to do all queued window events.
// It returns 1 if program should be closed.
int skd_do_window_events(void);

// A terminator function.
void skd_terminate_window(void);
