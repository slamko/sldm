#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "xconfig-names.h"

void error(const char* err_format, ...) {
    va_list args;
    char *err;

    va_start(args, err_format);
    err = concat("error: ", err_format);
    if (!err)
        return;

    vfprintf(stderr, err, args);
    vfprintf(stderr, "\n", args);
    free(err);
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
