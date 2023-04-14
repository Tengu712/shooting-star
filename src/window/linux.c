#include "../window.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XInput.h>
#include <string.h>

static Display *g_display;
static Window g_window;
static Atom g_atom_protocols;
static Atom g_atom_delete_window;
static XDevice *g_device;
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

    ss_debug("aaa");
    int opcode, event, error;
    if (!XQueryExtension(g_display, "XInputExtension", &opcode, &event, &error))
        ss_error("failed to enable XInput extension.");
    ss_debug("bbb");
    int info_cnt;
    const XDeviceInfoPtr info = XListInputDevices(g_display, &info_cnt);
    if (info == NULL)
        ss_error("failed to enumerate input devices.");
    for (int i = 0; i < info_cnt; ++i) {
        if (info[i].use == IsXKeyboard) {
            ss_debug("%s", info[i].name);
            g_device = XOpenDevice(g_display, info[i].id);
            break;
        }
    }
    XFreeDeviceList(info);

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
    }
    return 0;
}

void terminate_window(void) {}

int32_t get_input_state(SsKeyCode key_code) {
    return g_key_states[key_code];
}
