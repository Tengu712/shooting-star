#include "../sstar.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static int indent = 0;

EXPORT warn_t ss_init_logger(void) {
    if (setvbuf(stdout, NULL, _IOFBF, 8388608) != 0)
        return ss_warning("failed to setvbuf() for stdout.\n");
    if (setvbuf(stderr, NULL, _IOFBF, 8388608) != 0)
        return ss_warning("failed to setvbuf() for stderr.\n");
    return SS_SUCCESS;
}

EXPORT void ss_indent_logger(void) {
    indent += 1;
}

EXPORT void ss_dedent_logger(void) {
    if (indent > 0) indent -= 1;
}

static void print_indent(FILE *out) {
    for (int i = 0; i < indent; ++i) {
        fprintf(out, "  ");
    }
}

EXPORT void ss_error(const char *msg) {
    fprintf(stderr, "[ ERROR ] ");
    print_indent(stderr);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
    exit(1);
}

EXPORT warn_t ss_warning(const char *msg) {
    fprintf(stderr, "[ WARN  ] ");
    print_indent(stderr);
    fprintf(stderr, "%s\n", msg);
    fflush(stderr);
    return SS_WARN;
}

EXPORT void ss_info(const char *msg) {
    fprintf(stdout, "[ INFO  ] ");
    print_indent(stdout);
    fprintf(stdout, "%s\n", msg);
    fflush(stdout);
}

EXPORT void ss_info_fmt(const char *format, ...) {
    fprintf(stdout, "[ INFO  ] ");
    print_indent(stdout);
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    fprintf(stdout, "\n");
    fflush(stdout);
}

EXPORT void ss_debug(const char *format, ...) {
    fprintf(stdout, "[ DEBUG ] ");
    print_indent(stdout);
    va_list va;
    va_start(va, format);
    vprintf(format, va);
    va_end(va);
    fprintf(stdout, "\n");
    fflush(stdout);
}
