#pragma once

#define WINDOW_ERROR_MESSAGE_CONNECT_X 1
#define WINDOW_ERROR_MESSAGE_GET_SETUP 2
#define WINDOW_ERROR_MESSAGE_GET_SCREEN 3
#define WINDOW_ERROR_MESSAGE_CREATE_WINDOW 4
#define WINDOW_ERROR_MESSAGE_CHANGE_PROPERTY 5

int skd_create_window(const char *title, unsigned short width, unsigned short height);
int skd_do_window_events();
void skd_terminate_window();
