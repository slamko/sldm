#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wordexp.h>
#include <stdbool.h>
#include "config-names.h"
#include "log-utils.h"
#include "main.h"

#include "../config.h"

#ifdef PROMPT_TIMEOUT
int prompt_timeout = PROMPT_TIMEOUT;
#else
int prompt_timeout = _PROMPT_TIMEOUT;
#endif

#ifdef DEFAULT_ENTRY
int default_entry = DEFAULT_ENTRY;
#else
int default_entry = _DEFAULT_ENTRY;
#endif

#ifdef BASE_XCONFIG
char *base_xconfig = BASE_XCONFIG;
#else
char *base_xconfig = NULL;
#define CONFIG_UNDEFINED 
#endif

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
        int opt;

        while((opt = getopt(argc, argv, "r:")) != -1) 
        { 
            switch(opt) 
            { 
            case 'r':
                args->target = PROMPT;
                args->entry_name = optarg;
                return 0;
                break; 
            case '?':
                print_usage();
                return 1;
                break;
            }
        } 

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

    return 0;
}

int check_prompt_config() {
    if (default_entry <= 0) {
        error("Invalid default entry (%d)", default_entry);
        return 1;
    }

    if (prompt_timeout < 0) {
        error("Invalid prompt timeout (%d)", prompt_timeout);
        return 1;
    }
    return 0;
}

int check_xconfig() {
    wordexp_t exp_result;
    char *expanded_config;

    #ifdef CONFIG_UNDEFINED
    base_xconfig = XINITRC_L;
    #endif
    wordexp(base_xconfig, &exp_result, 0);
    expanded_config = strdup(exp_result.we_wordv[0]);
    wordfree(&exp_result);

    if (access(expanded_config, R_OK)){
        #ifndef CONFIG_UNDEFINED
        error("Unable to acces xinitrc at path %s", exp_result.we_wordv[0]);
        return 1;
        #else
        base_xconfig = NULL;
        return 0;
        #endif
    }

    base_xconfig = expanded_config;
    return 0;
}

void clean(struct args *arg) {
    cleanup_names();
    if (base_xconfig)
        free(base_xconfig);
        
    free(arg);
}

int main(int argc, char** argv) {
    struct args *args;
    int res = 1;

    args = (struct args *)malloc(sizeof(struct args));
    if (parse_args(argc, argv, args))
        goto clean_and_exit;

    switch (args->target)
    {
    case ADD_ENTRY:
        if (check_xconfig())
            goto clean_and_exit;

        res = add_entry(args->entry_name);
        break;
    case REMOVE_ENTRY:
        res = remove_entry(args->entry_name);
        break;
    case LIST_ENTRIES:
        res = list_entries(args->entry_name);
        break;
    case PROMPT:
        if (check_prompt_config())
            goto clean_and_exit;

        if (not_in_tty()) {
            error_not_in_tty();
            goto clean_and_exit;
        }
        res = prompt(args->entry_name);
        break;
    }

clean_and_exit:  
    clean(args);
    return res;
}