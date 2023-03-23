#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

warn_t init_logger(void) {
#ifndef NOLOG
    if (setvbuf(stdout, NULL, _IOFBF, 8388608) != 0) {
        log_warning("failed to setvbuf() for stdout.\n");
        return WARNING;
    }
    if (setvbuf(stderr, NULL, _IOFBF, 8388608) != 0) {
        log_warning("failed to setvbuf() for stderr.\n");
        return WARNING;
    }
#endif
    return SUCCESS;
}

void error(const char *msg) {
    fprintf(stderr, "[ ERROR ] %s\n", msg);
    fflush(stderr);
    exit(1);
}

void log_warning(const char *msg) {
#ifndef NOLOG
    fprintf(stderr, "[ WARNING ] %s\n", msg);
    fflush(stderr);
#endif
}

void log_info(const char *msg) {
#ifndef NOLOG
    fprintf(stdout, "[ INFO ] %s\n", msg);
    fflush(stdout);
#endif
}

void log_debug(const char *format, ...) {
#ifndef NOLOG
    fprintf(stdout, "[ DEBUG ] ");
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    fprintf(stdout, "\n");
    fflush(stdout);
#endif
}
