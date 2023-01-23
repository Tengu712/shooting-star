#pragma once

const char *skd_get_window_error_message(int res);
int skd_create_window(unsigned short width, unsigned short height);
int skd_do_window_events();
void skd_terminate_window();
