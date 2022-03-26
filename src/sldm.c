#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "xconfig-names.h"
#include "log-utils.h"
#include "main.h"

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
    char *entry_name;
};

void print_usage() {

}

int parse_args(int argc, char **argv, struct args *args) {
    char *target;
    args->target = PROMPT;
    args->entry_name = NULL;

    if (argc == 2) {
        if (strcmp(argv[1], "--help") || strcmp(argv[1], "-h")) {
            print_usage();
            return 1;
        }

        target = argv[1];
        if (strcmp(target, ADD_ENTRY_ARG)) {
            if (argc != 4) {
                print_usage();
                return 1;
            }

            args->target = ADD_ENTRY;
            args->entry_name = argv[2];
            add_entry_command = argv[3];
        } else if (strcmp(target, REMOVE_ENTRY)) {
            if (argc != 3) {
                print_usage();
                return 1;
            }

            args->target = REMOVE_ENTRY;
            args->entry_name = argv[2];
        } 
    } else {
        args->target = PROMPT;

        if (argc == 2)
            args->entry_name = argv[1];
    }

    for (int argi = 3; argi < argc; argi++) {
        //parsing args
    }

    return 0;
}

void clean(struct args *arg) {
    cleanup_names();
    free(arg);
}

int main(int argc, char** argv) {
    struct args *args;
    int res;

    args = (struct args *)malloc(sizeof(struct args));
    if (parse_args(argc, argv, args)) {
        clean(args);
    }

    switch (args->target)
    {
    case ADD_ENTRY:
        res = add_entry(args->entry_name);
        break;
    case REMOVE_ENTRY:
        res = remove_entry(args->entry_name);
        break;
    case PROMPT:
        if (not_in_tty()) {
            fail_not_in_tty();
            clean(args);
            return 1;
        }
        res = prompt(args->entry_name);
        break;
    }
 
    return res;
}