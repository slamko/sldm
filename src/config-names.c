//#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *home = NULL;

char *get_home() {
    if (!home)
        home = getenv("HOME");
    return home;
}