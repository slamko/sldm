#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>
#include <stdbool.h>
#include <errno.h>
#include "config-names.h"
#include "log-utils.h"
#include "nentry-prompt.h"
#include "main.h"
#include "utils.h"

#include "config.h"

#ifdef PROMPT_TIMEOUT
int prompt_timeout = PROMPT_TIMEOUT;
#else
int prompt_timeout = -1;
#endif

#ifdef DEFAULT_ENTRY
entryid default_entry = DEFAULT_ENTRY;
#else
entryid default_entry = _DEFAULT_ENTRY;
#endif

#ifdef BASE_XCONFIG
char *base_xconfig = BASE_XCONFIG;
#else
char *base_xconfig = NULL;
#define CONFIG_UNDEFINED 
#endif

enum target {
    ADD_ENTRY = 0,
    REMOVE_ENTRY = 1,
    LIST_ENTRIES = 2,
    SHOW_ENTRY = 3,
    PROMPT = 4
};

struct args {
    enum target target;
    char *entry_name;
};

static void print_usage() {
    puts(
    "\n Usage: \n"
    "\tsldm add <entry> <exec>\n"
    "\tsldm remove <entry>\n"
    "\tsldm list [entry]\n"
    "\tsldm show <entry>\n"
    "\tsldm [options] [entry] - Enter the menu screen\n"
    "\n options: \n"
    "\t-r    force run xorg if an entry name in the same as the commands above\n");
}

static int parse_args(int argc, char **argv, struct args *args) {
    char *target = NULL;
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
        int opt;

        while((opt = getopt(argc, argv, "hr:")) != -1) 
        { 
            switch(opt) 
            { 
            case 'r':
                args->target = PROMPT;
                args->entry_name = optarg;
                return 0;
                break; 
            case 'h':
                print_usage();
                return 1;
            case '?':
                print_usage();
                return 1;
            }
        } 

        target = argv[1];
        if (!partialcmp(target, ADD_ENTRY_ARG)) {
            if (argc < 4) {
                print_usage();
                return 1;
            }
    
            args->target = ADD_ENTRY;
            args->entry_name = argv[2];
            add_entry_command = argv[3];
        } else if (!partialcmp(target, REMOVE_ENTRY_ARG)) {
            if (argc < 3) {
                print_usage();
                return 1;
            }
    
            args->target = REMOVE_ENTRY;
            args->entry_name = argv[2];
        } else if (!partialcmp(target, LIST_ENTRIES_ARG)) {
            args->target = LIST_ENTRIES;
            args->entry_name = argv[2];
        } else if (!partialcmp(target, SHOW_ENTRY_ARG)) {
            if (argc < 3) {
                print_usage();
                return 1;
            }

            args->target = SHOW_ENTRY;
            args->entry_name = argv[2];
        } else {
            args->target = PROMPT;
            args->entry_name = argv[1];
        }
    }

    return 0;
}

static int check_prompt_config() {
    if (default_entry <= 0) {
        error("Invalid default entry (%d)", default_entry);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void clean(struct args *arg) {
    cleanup_names();
    if (base_xconfig) {
        free(base_xconfig);
    }
        
    free(arg);
}

int main(int argc, char** argv) {
    struct args *args = NULL;
    int res = EXIT_FAILURE;

    args = malloc(sizeof(*args));
    if (parse_args(argc, argv, args))
        goto cleanup;

    switch (args->target)
    {
    case ADD_ENTRY:
        if (check_xconfig(&base_xconfig)) {
            error("Unable to acces .xinitrc at path %s", base_xconfig); 
            goto cleanup;
        }

        res = add_entry(args->entry_name);
        break;
    case REMOVE_ENTRY:
        res = remove_entry(args->entry_name);
        break;
    case LIST_ENTRIES:
        res = list_entries(args->entry_name);
        break;
    case SHOW_ENTRY:
        res = show_entry(args->entry_name);
        break;
    case PROMPT:
        if (check_prompt_config())
            goto cleanup;

        if (not_in_tty()) {
            error_not_in_tty();
            goto cleanup;
        }
        res = nprompt(args->entry_name);
        break;
    }

cleanup:  
    clean(args);
    return res;
}

