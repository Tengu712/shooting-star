#include <stdio.h>

#include "src/window.h"

int main() {
    int res = skd_create_window(640, 480);
    if (res != 0) {
        fprintf(
            stderr,
            "failed to create a window: %s\n",
            skd_get_window_error_message(res)
        );
        return res;
    }
    while (1) {
        if (skd_do_window_events() == 1) {
            skd_terminate_window();
            return 0;
        }
    }
    return 0;
}
