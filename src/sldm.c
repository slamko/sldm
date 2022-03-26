//#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xconfig-names.h"
#include "log-utils.h"
#include "main.h"

#define ADD_ENTRY_ARG "add-entry"
#define REMOVE_ENTRY_ARG "add-entry"

int not_in_tty() {
    FILE* fp;
    int cmp = -1;
    char tty_output[128];

    fp = popen(TTY_BIN, "r");
    if (!fp) 
        return cmp;

    if (fgets(tty_output, sizeof(tty_output), fp) != 0) {
        char tty[9];
        strncpy(tty, tty_output, TTY_DEVICE_NAME_BYTES);
        tty[9] = '\0';

        char *tty_exp = TTY_DEVICE;
        cmp = strcmp(tty, tty_exp);
    }

    pclose(fp);
    return cmp;
} 

enum target {
    ADD_ENTRY = 0,
    REMOVE_ENTRY = 1,
    PROMPT = 2
};

struct args {
    enum target target;
};

void print_usage() {

}

struct args *parse_args(int argc, char **argv) {
    char *target;
    struct args *args;

    if (argc == 1) {
        print_usage();
        exit(0);
    }

    if (strcmp(argv[1], "--help") || strcmp(argv[1], "-h")) {
        print_usage();
        exit(0);
    }

    args = (struct args *)malloc(sizeof(args));

    target = argv[1];
    if (strcmp(target, ADD_ENTRY_ARG)) {
        args->target = ADD_ENTRY;
    } else if (strcmp(target, REMOVE_ENTRY)) {
        args->target = REMOVE_ENTRY;
    } else {
        args->target = PROMPT;
    }

    for (int argi = 2; argi < argc; argi++) {

    }
}

void clean(struct args *arg) {
    cleanup_names();
    free(arg);
}

int main(int argc, char** argv) {
    struct args *args = parse_args(argc, argv);

    switch (args->target)
    {
    case ADD_ENTRY:
        add_entry("");
        break;
    case REMOVE_ENTRY:
        remove_entry("");
        break;
    case PROMPT:
        if (not_in_tty()) {
            fail_not_in_tty();
            goto cleanup;
            return 1;
        }
        prompt();
        break;
    }

    
    cleanup:
        clean(args);
        
    return 0;
}