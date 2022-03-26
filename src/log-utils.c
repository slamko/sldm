#include <stdio.h>

void fail(char *err) {
    printf("error: %s", err);
}

void fail_not_in_tty(void) {
    fail("Only console users are allowed to run the X server");
}

void die_s(char *err, int status) {
    fail(err);
    exit(status);
}

void die(char *err) {
    die_s(err, 1);
}
