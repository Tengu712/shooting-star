#include "../window.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#include <string.h>

static Display *g_display;
static Window g_window;
static Atom g_atom_protocols;
static Atom g_atom_delete_window;
static int g_fd = -1;
static int32_t g_input_states[NUM_OF_KEYCODES];

static SsKeycode convert_keycode_xcb_to_ss(KeySym key_sym) {
    switch (key_sym) {
        case XK_a: return KEYCODE_A;
        case XK_b: return KEYCODE_B;
        case XK_c: return KEYCODE_C;
        case XK_d: return KEYCODE_D;
        case XK_e: return KEYCODE_E;
        case XK_f: return KEYCODE_F;
        case XK_g: return KEYCODE_G;
        case XK_h: return KEYCODE_H;
        case XK_i: return KEYCODE_I;
        case XK_j: return KEYCODE_J;
        case XK_k: return KEYCODE_K;
        case XK_l: return KEYCODE_L;
        case XK_m: return KEYCODE_M;
        case XK_n: return KEYCODE_N;
        case XK_o: return KEYCODE_O;
        case XK_p: return KEYCODE_P;
        case XK_q: return KEYCODE_Q;
        case XK_r: return KEYCODE_R;
        case XK_s: return KEYCODE_S;
        case XK_t: return KEYCODE_T;
        case XK_u: return KEYCODE_U;
        case XK_v: return KEYCODE_V;
        case XK_w: return KEYCODE_W;
        case XK_x: return KEYCODE_X;
        case XK_y: return KEYCODE_Y;
        case XK_z: return KEYCODE_Z;
        case XK_0: return KEYCODE_0;
        case XK_1: return KEYCODE_1;
        case XK_2: return KEYCODE_2;
        case XK_3: return KEYCODE_3;
        case XK_4: return KEYCODE_4;
        case XK_5: return KEYCODE_5;
        case XK_6: return KEYCODE_6;
        case XK_7: return KEYCODE_7;
        case XK_8: return KEYCODE_8;
        case XK_9: return KEYCODE_9;
        case XK_Up: return KEYCODE_UP;
        case XK_Left: return KEYCODE_LEFT;
        case XK_Down: return KEYCODE_DOWN;
        case XK_Right: return KEYCODE_RIGHT;
        case XK_Return: return KEYCODE_ENTER;
        case XK_space: return KEYCODE_SPACE;
        case XK_Shift_L:
        case XK_Shift_R: return KEYCODE_SHIFT;
        case XK_Tab: return KEYCODE_TAB;
        case XK_Control_L:
        case XK_Control_R: return KEYCODE_CONTROL;
        case XK_Escape: return KEYCODE_ESCAPE;
        default: return NO_KEYCODE;
    }
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


    memset(g_input_states, 0, sizeof(int32_t) * NUM_OF_KEYCODES);
    XSelectInput(g_display, g_window, KeyPressMask | KeyReleaseMask);
    if ((g_fd = open("/dev/input/js0", O_RDONLY | O_NONBLOCK)) < 0)
        ss_info("joystick not detected.");
    else
        ss_info("joystick detected.");

    XMapWindow(g_display, g_window);
    XFlush(g_display);

    ss_dedent_logger();
    ss_info("xlib window initialization succeeded.");
    return SS_SUCCESS;
}

int32_t do_window_events(void) {
    const size_t changes_cnt = NUM_OF_KB_KEYS + NUM_OF_JS_BUTTONS;
    char input_state_changes[changes_cnt];
    memset(input_state_changes, 0, sizeof(char) * changes_cnt);

    // do events and get key input state changes
    XEvent event;
    while (XPending(g_display)) {
        XNextEvent(g_display, &event);
        switch (event.type) {
            case ClientMessage:
                if (event.xclient.message_type == g_atom_protocols && event.xclient.data.l[0] == g_atom_delete_window)
                    return 1;
            case KeyPress: {
                const KeySym key_sym = XkbKeycodeToKeysym(g_display, event.xkey.keycode, 0, 0);
                const SsKeycode ss_keycode = convert_keycode_xcb_to_ss(key_sym);
                input_state_changes[ss_keycode] = 1;
                break;
            }
            case KeyRelease: {
                const KeySym key_sym = XkbKeycodeToKeysym(g_display, event.xkey.keycode, 0, 0);
                const SsKeycode ss_keycode = convert_keycode_xcb_to_ss(key_sym);
                input_state_changes[ss_keycode] = 2;
                break;
            }
        }
    }

    // monitor joystick
    if (g_fd >= 0) {
        struct js_event jevent;
        const size_t js_event_size = sizeof(struct js_event);
        ssize_t len = read(g_fd, &jevent, js_event_size);
        if (len == js_event_size) {
            if ((jevent.type & JS_EVENT_BUTTON) && jevent.number < NUM_OF_JS_BUTTONS)
                input_state_changes[KEYCODE_JS_BUTTON_0 + jevent.number] = jevent.value == 1 ? 1 : 2;
            else if ((jevent.type & JS_EVENT_AXIS) && jevent.number < NUM_OF_JS_AXES)
                // NOTE: jevent.value is not 0/1.
                // NOTE: while the joystick is tilted, the event will continue to occur.
                g_input_states[KEYCODE_JS_AXIS_0 + jevent.number] = jevent.value;
        }
    }

    // update input states
    for (int i = 1; i < changes_cnt; ++i) {
        if (input_state_changes[i] == 1 && g_input_states[i] < 1)
            g_input_states[i] = 1;
        else if (input_state_changes[i] == 2)
            g_input_states[i] = -1;
        else if (g_input_states[i] > 0)
            g_input_states[i] += 1;
        else if (g_input_states[i] < 0)
            g_input_states[i] = 0;
    }

    // finish
    return 0;
}

void terminate_window(void) {}

int32_t get_input_state(SsKeycode keycode) {
    return g_input_states[keycode];
}
