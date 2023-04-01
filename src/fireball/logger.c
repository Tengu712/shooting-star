#include "../fireball.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

EXPORT warn_t init_logger(void) {
#ifndef NOLOG
    if (setvbuf(stdout, NULL, _IOFBF, 8388608) != 0)
        return warning("failed to setvbuf() for stdout.\n");
    if (setvbuf(stderr, NULL, _IOFBF, 8388608) != 0)
        return warning("failed to setvbuf() for stderr.\n");
#endif
    return FB_SUCCESS;
}

EXPORT void error(const char *msg) {
    fprintf(stderr, "[ ERROR ] %s\n", msg);
    fflush(stderr);
    exit(1);
}

EXPORT warn_t warning(const char *msg) {
#ifndef NOLOG
    fprintf(stderr, "[ FB_WARN ] %s\n", msg);
    fflush(stderr);
#endif
    return FB_WARN;
}

EXPORT void log_info(const char *msg) {
#ifndef NOLOG
    fprintf(stdout, "[ INFO ] %s\n", msg);
    fflush(stdout);
#endif
}

EXPORT void log_debug(const char *format, ...) {
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
