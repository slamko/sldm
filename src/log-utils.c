#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ncurses.h>
#include "config-names.h"
#include "log-utils.h"

void error(const char* err_format, ...) {
    va_list args;
    char *err;

    va_start(args, err_format);
    err = sappend(ERR_PREF ": ", err_format);
    if (!err)
        return;

    vfprintf(stderr, err, args);
    vfprintf(stderr, "\n", args);
    free(err);
    va_end(args);
}

void nerror(const char* err_format, ...) {
    va_list args;
    char *err;

    va_start(args, err_format);
    err = sappend(ERR_PREF ": ", err_format);
    if (!err)
        return;

    printw(err, args);
    printw("\n");
    refresh();
    free(err);
    va_end(args);
}

void error_not_in_tty(void) {
    error("Only console users are allowed to run the X server");
}

void error_invalid_entry(void) {
    error("Invalid entry name");
}

void err_noentry_found(const char *entryname) {
    error("No entry found with name: '%s'", entryname);
}

void die_s(const char *err, int status) {
    error(err);
    exit(status);
}

void die(const char *err) {
    die_s(err, 1);
}

void fatal(void) {
    perror(FATAL_PREF);
    exit(1);
}
