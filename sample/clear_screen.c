#include "sstar.h"

int main() {
    ss_init_logger();
    ss_init("clear screen", 640, 480, 0);
    while (1) {
        if (ss_should_close()) break;
        ss_render(0.2f, 0.2f, 0.2f, NULL, 0);
    }
    ss_terminate();
    return 0;
}
