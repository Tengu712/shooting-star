#include "../fireball.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int indent = 0;

EXPORT warn_t fb_init_logger(void) {
#ifndef NOLOG
    if (setvbuf(stdout, NULL, _IOFBF, 8388608) != 0)
        return fb_warning("failed to setvbuf() for stdout.\n");
    if (setvbuf(stderr, NULL, _IOFBF, 8388608) != 0)
        return fb_warning("failed to setvbuf() for stderr.\n");
#endif
    return FB_SUCCESS;
}

EXPORT void fb_indent_logger(void) {
    indent += 1;
}

EXPORT void fb_dedent_logger(void) {
    if (indent > 0) indent -= 1;
}

static void print_indent(FILE *out) {
    for (int i = 0; i < indent; ++i) {
        fprintf(out, "  ");
    }
}

EXPORT void fb_error(const char *msg) {
    fprintf(stderr, "[ ERROR ] ");
    print_indent(stderr);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
    exit(1);
}

EXPORT warn_t fb_warning(const char *msg) {
#ifndef NOLOG
    fprintf(stderr, "[ WARN ] ");
    print_indent(stderr);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
#endif
    return FB_WARN;
}

EXPORT void fb_info(const char *msg) {
#ifndef NOLOG
    fprintf(stdout, "[ INFO ] ");
    print_indent(stdout);
    fprintf(stdout, "%s\n", msg);
    fflush(stdout);
#endif
}

EXPORT void fb_debug(const char *format, ...) {
#ifndef NOLOG
    fprintf(stdout, "[ DEBUG ] ");
    print_indent(stdout);
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    fprintf(stdout, "\n");
    fflush(stdout);
#endif
}
