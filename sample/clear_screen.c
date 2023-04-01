#include "fireball.h"

int main() {
    fb_init_logger();
    fb_init("clear screen", 640, 480, 0);
    while (1) {
        if (fb_should_close()) break;
        fb_render(0.2f, 0.2f, 0.2f);
    }
    fb_terminate();
    return 0;
}
