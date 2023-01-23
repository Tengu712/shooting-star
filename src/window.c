#include <xcb/xcb.h>
#include <stdlib.h>

#include "window.h"

xcb_connection_t *g_connection;
xcb_intern_atom_reply_t *g_atom_delete_window;

#define WINDOW_ERROR_MESSAGE_COUNT 5
const char *g_window_error_message[WINDOW_ERROR_MESSAGE_COUNT] = {
    "failed to connect with X server",
    "failed to get a setup",
    "failed to get a screen",
    "failed to create a window",
    "failed to change property for detect window closing event",
};

const char *skd_get_window_error_message(int res) {
    if (res <= 0 || res >= WINDOW_ERROR_MESSAGE_COUNT) {
        return "unexpected";
    } else {
        return g_window_error_message[res - 1];
    }
}

int skd_create_window(uint16_t width, uint16_t height) {
    xcb_void_cookie_t res;
    // X
    g_connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(g_connection)) {
        return 1;
    }
    // screen
    const xcb_setup_t *setup = xcb_get_setup(g_connection);
    if (setup == NULL) {
        return 2;
    }
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    if (screen == NULL) {
        return 3;
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
        return 4;
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
        4,
        32,
        1,
        &g_atom_delete_window->atom
    );
    if (xcb_request_check(g_connection, res) != NULL) {
        return 5;
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
    return 1;
}

void skd_terminate_window() {
    if (g_connection == NULL) {
        return;
    }
    xcb_disconnect(g_connection);
    free(g_atom_delete_window);
}
