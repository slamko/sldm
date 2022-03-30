#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "config-names.h"
#include "log-utils.h"
#include "main.h"

int not_in_tty() {
    FILE* fp = NULL;
    int cmp = -1;
    char tty_output[128];

    fp = popen(TTY_BIN, "r");
    if (!fp) 
        return cmp;

    if (fgets(tty_output, sizeof(tty_output), fp) != 0) {
        char tty[9] = "";
        char *tty_exp;
        strncpy(tty, tty_output, TTY_DEVICE_NAME_BYTES);
        tty[9] = '\0';

        tty_exp = TTY_DEVICE;
        cmp = strcmp(tty, tty_exp);
    }

    pclose(fp);
    return cmp;
} 

enum target {
    ADD_ENTRY = 0,
    REMOVE_ENTRY = 1,
    LIST_ENTRIES = 2,
    PROMPT = 3
};

struct args {
    enum target target;
    char *entry_name;
};

void print_usage() {
    printf("usage: \n");
}

int cmp_command(const char *command, const char *cmp) {
    int cmlen;
    int cmplen;

    if (!command || !cmp)
        return 1;

    cmlen = strlen(command);
    cmplen = strlen(cmp);

    if (cmlen > cmplen)
        return 1;
    else if (cmlen == cmplen) 
        return strcmp(command, cmp);
    
    for (int i = 0; i < strlen(command); i++)
    {
        if (command[i] != cmp[i])
            return 1; 
    }
    return 0;
}

int parse_args(int argc, char **argv, struct args *args) {
    char *target;
    args->target = PROMPT;
    args->entry_name = NULL;

    if (argc >= 2){
        if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h")) {
            print_usage();
            return 1;
        }
    }

    if (argc == 1) {
        args->target = PROMPT;
    } else {
        target = argv[1];
        if (!cmp_command(target, ADD_ENTRY_ARG)) {
            if (argc != 4) {
                print_usage();
                return 1;
            }
    
            args->target = ADD_ENTRY;
            args->entry_name = argv[2];
            add_entry_command = argv[3];
        } else if (!cmp_command(target, REMOVE_ENTRY_ARG)) {
            if (argc != 3) {
                print_usage();
                return 1;
            }
    
            args->target = REMOVE_ENTRY;
            args->entry_name = argv[2];
        } else if (!cmp_command(target, LIST_ENTRIES_ARG)) {
            args->target = LIST_ENTRIES;
            args->entry_name = argv[2];
        } else {
            args->target = PROMPT;
            args->entry_name = argv[1];
        }
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
    int res = 1;

    args = (struct args *)malloc(sizeof(struct args));
    if (parse_args(argc, argv, args)) {
        clean(args);
        return res;
    }

    switch (args->target)
    {
    case ADD_ENTRY:
        res = add_entry(args->entry_name);
        break;
    case REMOVE_ENTRY:
        res = remove_entry(args->entry_name);
        break;
    case LIST_ENTRIES:
        res = list_entries(args->entry_name);
        break;
    case PROMPT:
        if (not_in_tty()) {
            error_not_in_tty();
            clean(args);
            return res;
        }
        res = prompt(args->entry_name);
        break;
    }
    
    clean(args);
    return res;
}