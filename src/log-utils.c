#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

void error(const char* err, ...) {
    va_list args;
    va_start(args, err);
    fprintf(stderr, "error: ");
    fprintf(stderr, err, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void error_not_in_tty(void) {
    error("Only console users are allowed to run the X server");
}

void die_s(char *err, int status) {
    error(err);
}

void die(char *err) {
    die_s(err, 1);
}
