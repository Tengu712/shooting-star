#include "../input.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

// HACK: to variable-length array
int fds_cnt = 0;
int fds[5] = {0};

warn_t init_input(void) {
    ss_info("Start input initialization...");
    ss_indent_logger();

    warn_t res = SS_SUCCESS;

    FILE *fp;
    if ((fp = popen("cat /proc/bus/input/devices", "r")) == NULL)
        ss_error("failed to cat /proc/bus/input/devices to get input devices");

    char event[32];
    char buf[512];
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        if (strncmp(buf, "H: Handlers=", 12) == 0) {
            memset(event, 0, sizeof(event));
            const char *event_ = strstr(buf, "event");
            for (int i = 0; i < strlen(event_); ++i) {
                if (event_[i] == ' ' || event_[i] == '\n') break;
                event[i] = event_[i];
            }
        } else if (strncmp(buf, "B: EV=", 6) == 0) {
            const int len = strlen(buf);
            if (buf[len - 3] == '1' && buf[len - 2] == '3') {
                char path[256] = "/dev/input/";
                strcat(path, event);
                const int fd = open(path, O_RDONLY);
                if (fd == -1)
                    res = ss_warning_fmt("failed to open %s", path);
                else
                    fds[fds_cnt++] = fd;
            }
        }
    }

    pclose(fp);

    ss_dedent_logger();
    ss_info("input initialization succeeded.");
    return res;
}

void input_test(void) {
    ss_debug("input test");
    return;
}
