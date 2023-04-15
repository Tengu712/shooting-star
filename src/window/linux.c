#include "../window.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput2.h>
#include <string.h>

static Display *g_display;
static Window g_window;
static Atom g_atom_protocols;
static Atom g_atom_delete_window;
static int g_xi_opcode;
static int32_t g_key_states[NUM_OF_KEY_CODES];

// TODO:
int32_t convert_keycode_xcb_to_ss(int xk) {
    switch (xk) {
        case 0: return KEY_CODE_A;
    }
    return -1;
}

void create_window_param(WindowParam *out) {
    out->xlib.display = g_display;
    out->xlib.window = g_window;
}

warn_t create_window(const char *title, uint32_t width, uint32_t height) {
    ss_info("initializing xlib g_window ...");
    ss_indent_logger();

    g_display = XOpenDisplay(NULL);
    if (g_display == NULL)
        ss_error("failed to open g_display.");
    // TODO: error handling
    g_window = XCreateSimpleWindow(g_display, RootWindow(g_display, 0), 0, 0, width, height, 0, 0, WhitePixel(g_display, 0));

    XStoreName(g_display, g_window, title);

    XSizeHints size_hints;
    memset(&size_hints, 0, sizeof(XSizeHints));
    size_hints.flags = PMinSize | PMaxSize;
    size_hints.max_width = (int32_t)width;
    size_hints.min_width = (int32_t)width;
    size_hints.max_height = (int32_t)height;
    size_hints.min_height = (int32_t)height;
    XSetWMNormalHints(g_display, g_window, &size_hints);

    g_atom_protocols = XInternAtom(g_display, "WM_PROTOCOLS", False);
    g_atom_delete_window = XInternAtom(g_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g_display, g_window, &g_atom_delete_window, 1);

    int dummy1, dummy2;
    if (XQueryExtension(g_display, "XInputExtension", &g_xi_opcode, &dummy1, &dummy2) == False)
        ss_error("failed to enable xinput.");
    int mask_len = XIMaskLen(XI_LASTEVENT);
    unsigned char mask[mask_len];
    memset(mask, 0, sizeof(mask));
    XISetMask(mask, XI_KeyPress);
    XIEventMask event_mask = {
        XIAllDevices,
        mask_len,
        mask,
    };
    XISelectEvents(g_display, DefaultRootWindow(g_display), &event_mask, 1);

    XMapWindow(g_display, g_window);
    XFlush(g_display);

    ss_dedent_logger();
    ss_info("xlib window initialization succeeded.");
    return SS_SUCCESS;
}

int32_t do_window_events(void) {
    XEvent event;
    while (XPending(g_display)) {
        XNextEvent(g_display, &event);
        switch (event.type) {
            case ClientMessage:
                if (event.xclient.message_type == g_atom_protocols && event.xclient.data.l[0] == g_atom_delete_window)
                    return 1;
                break;
        }
        XGenericEventCookie *cookie = &event.xcookie;
        if (cookie->type == GenericEvent && cookie->extension == g_xi_opcode && XGetEventData(g_display, cookie)) {
            XIDeviceEvent *xi_event = cookie->data;
            if (xi_event->evtype == XI_KeyPress) ss_debug("%d", xi_event->detail);
            XFreeEventData(g_display, cookie);
        }
    }
    return 0;
}

void terminate_window(void) {}

int32_t get_input_state(SsKeyCode key_code) {
    return g_key_states[key_code];
}
