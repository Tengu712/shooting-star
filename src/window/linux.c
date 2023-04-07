#include "../window.h"

#include <xcb/xcb.h>
#include <stdlib.h>
#include <string.h>

#define CHECK(p) if (xcb_request_check(g_connection, res) != NULL) ss_error((p));

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

static xcb_connection_t *g_connection;
static xcb_window_t g_window;
static xcb_intern_atom_reply_t *g_atom_delete_window;

void create_window_param(WindowParam *out) {
    out->xcb_window.connection = g_connection;
    out->xcb_window.window = g_window;
}

warn_t create_window(const char *title, uint16_t width, uint16_t height) {
    ss_info("start initialize xcb window ...");
    ss_indent_logger();
    xcb_void_cookie_t res;
    // X
    g_connection = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(g_connection)) ss_error("failed to get xcb connection.");
    // screen
    const xcb_setup_t *setup = xcb_get_setup(g_connection);
    if (setup == NULL) ss_error("failed to get xcb setup.");
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);
    xcb_screen_t *screen = iter.data;
    if (screen == NULL) ss_error("failed to get screen.");
    // window
    const uint32_t value_list[1] = {
        XCB_EVENT_MASK_EXPOSURE,
    };
    g_window = xcb_generate_id(g_connection);
    res = xcb_create_window(
        g_connection,
        XCB_COPY_FROM_PARENT,
        g_window,
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
    CHECK("failed to create xcb window.");
    // title
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        g_window,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(title),
        title
    );
    CHECK("failed to change window title.");
    // disable resize
    XSizeHints size_hints;
    memset(&size_hints, 0, sizeof(XSizeHints));
    size_hints.flags = (1 << 4) | (1 << 5);
    size_hints.max_width = (int32_t)width;
    size_hints.min_width = (int32_t)width;
    size_hints.max_height = (int32_t)height;
    size_hints.min_height = (int32_t)height;
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        g_window,
        XCB_ATOM_WM_NORMAL_HINTS,
        XCB_ATOM_WM_SIZE_HINTS,
        32,
        sizeof(XSizeHints) >> 2,
        &size_hints
    );
    CHECK("failed to make window unresizable.");
    // event
    xcb_intern_atom_cookie_t cookie_protocols = xcb_intern_atom(g_connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t *atom_protocols = xcb_intern_atom_reply(g_connection, cookie_protocols, 0);
    xcb_intern_atom_cookie_t cookie_delete_window = xcb_intern_atom(g_connection, 0, 16, "WM_DELETE_WINDOW");
    g_atom_delete_window = xcb_intern_atom_reply(g_connection, cookie_delete_window, 0);
    res = xcb_change_property(
        g_connection,
        XCB_PROP_MODE_REPLACE,
        g_window,
        atom_protocols->atom,
        XCB_ATOM_ATOM,
        32,
        1,
        &g_atom_delete_window->atom
    );
    CHECK("failed to make window close when exiting it.");
    free(atom_protocols);
    // finish
    xcb_map_window(g_connection, g_window);
    xcb_flush(g_connection);
    ss_dedent_logger();
    ss_info("xcb window initialization succeeded.");
    return SS_SUCCESS;
}

int32_t do_window_events(void) {
    while (1) {
        xcb_generic_event_t* event = xcb_poll_for_event(g_connection);
        if (event == NULL) {
            free(event);
            return 0;
        }
        uint8_t response = event->response_type & ~0x80;
        if (response == XCB_CLIENT_MESSAGE) {
            xcb_client_message_event_t *cme = (xcb_client_message_event_t *)event;
            if (cme->data.data32[0] == g_atom_delete_window->atom) {
                free(event);
                return 1;
            }
        }
        free(event);
    }
}

void terminate_window(void) {
    if (g_connection == NULL) {
        return;
    }
    xcb_disconnect(g_connection);
    free(g_atom_delete_window);
}
