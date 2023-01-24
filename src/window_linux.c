#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>

#include "window.h"

typedef struct {
    uint32_t flags;
    int32_t x, y;
    int32_t width, height;
    int32_t min_width, min_height;
    int32_t max_width, max_height;
    int32_t width_inc, height_inc;
    int32_t min_aspect_num, min_aspect_den;
    int32_t max_aspect_num, max_aspect_den;
    int32_t base_width, base_height;
    uint32_t win_gravity;
} XSizeHints;

xcb_connection_t *g_connection;
xcb_intern_atom_reply_t *g_atom_delete_window;

int skd_create_window(const char *title, unsigned short width, unsigned short height) {
    xcb_void_cookie_t res;
    // X
    g_connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(g_connection)) {
        return WINDOW_ERROR_MESSAGE_CONNECT_X;
    }
    // screen
    const xcb_setup_t *setup = xcb_get_setup(g_connection);
    if (setup == NULL) {
        return WINDOW_ERROR_MESSAGE_GET_SETUP;
    }
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    if (screen == NULL) {
        return WINDOW_ERROR_MESSAGE_GET_SCREEN;
    }
    // window
    const uint32_t value_list[1] = {
        XCB_EVENT_MASK_EXPOSURE,
    };
    xcb_window_t window = xcb_generate_id(g_connection);
    res = xcb_create_window(
        g_connection,
        XCB_COPY_FROM_PARENT,
        window,
        screen->root,
        0,
        0,
        width,
        height,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        XCB_CW_EVENT_MASK,
        value_list
    );
    if (xcb_request_check(g_connection, res) != NULL) {
        return WINDOW_ERROR_MESSAGE_CREATE_WINDOW;
    }
    // title
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(title),
        title
    );
    if (xcb_request_check(g_connection, res) != NULL) {
        return WINDOW_ERROR_MESSAGE_CHANGE_PROPERTY;
    }
    // disable resize
    XSizeHints size_hints;
    memset(&size_hints, 0, sizeof(XSizeHints));
    size_hints.flags = (1 << 4) | (1 << 5);
    size_hints.max_width = (int)width;
    size_hints.min_width = (int)width;
    size_hints.max_height = (int)height;
    size_hints.min_height = (int)height;
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        window,
        XCB_ATOM_WM_NORMAL_HINTS,
        XCB_ATOM_WM_SIZE_HINTS,
        32,
        sizeof(XSizeHints) >> 2,
        &size_hints
    );
    if (xcb_request_check(g_connection, res) != NULL) {
        return WINDOW_ERROR_MESSAGE_CHANGE_PROPERTY;
    }
    // event
    xcb_intern_atom_cookie_t cookie_protocols =
        xcb_intern_atom(g_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *atom_protocols =
        xcb_intern_atom_reply(g_connection, cookie_protocols, 0);
    xcb_intern_atom_cookie_t cookie_delete_window =
        xcb_intern_atom(g_connection, 0, 16, "WM_DELETE_WINDOW");
    g_atom_delete_window =
        xcb_intern_atom_reply(g_connection, cookie_delete_window, 0);
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        window,
        atom_protocols->atom,
        XCB_ATOM_ATOM,
        32,
        1,
        &g_atom_delete_window->atom
    );
    if (xcb_request_check(g_connection, res) != NULL) {
        return WINDOW_ERROR_MESSAGE_CHANGE_PROPERTY;
    }
    free(atom_protocols);
    // finish
    xcb_map_window(g_connection, window);
    xcb_flush(g_connection);
    return 0;
}

int skd_do_window_events() {
    while (1) {
        xcb_generic_event_t* event = xcb_poll_for_event(g_connection);
        if (event == NULL) {
            free(event);
            return 0;
        }
        uint8_t response = event->response_type & ~0x80;
        if (response == XCB_CLIENT_MESSAGE) {
            xcb_client_message_event_t *cme =
                (xcb_client_message_event_t *)event;
            if (cme->data.data32[0] == g_atom_delete_window->atom) {
                free(event);
                return 1;
            }
        }
        free(event);
    }
}

void skd_terminate_window() {
    if (g_connection == NULL) {
        return;
    }
    xcb_disconnect(g_connection);
    free(g_atom_delete_window);
}
