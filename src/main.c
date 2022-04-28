#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "config-names.h"
#include "log-utils.h"
#include "command-names.h"

extern int errno;
char *add_entry_command = NULL;

int partialcmp(const char *entry, const char *cmp) {
    int elen;
    int cmplen;
    
    if (!entry || !cmp)
        return 1;

    elen = strnlen(entry, ENTRY_NAME_BUF_SIZE);
    cmplen = strnlen(cmp, ENTRY_NAME_BUF_SIZE);

    if (elen == cmplen) 
        return strncmp(entry, cmp, ENTRY_NAME_BUF_SIZE);
    
    for (int i = 0; i < (elen > cmplen ? cmplen : elen); i++)
    {
        if (entry[i] != cmp[i])
            return 1; 
    }
    return 0;
}

int write_exec_command(FILE *fp) {
    char *exec_line = NULL;
    char *exec_command = NULL;

    exec_line = calloc(EXEC_C_LENGTH + 1 + strlen(add_entry_command) + 1, sizeof(*exec_line));

    if (!exec_line) {
        fclose(fp);
        return 1;
    }

    exec_command = EXEC_C;
    strcpy(exec_line, exec_command);
    exec_line[EXEC_C_LENGTH] = ' ';
    strcpy(exec_line + EXEC_C_LENGTH + 1, add_entry_command);

    if (fputs(exec_line, fp) == EOF) {
        free(exec_line);
        return 1;
    }

    free(exec_line);
    return 0;
}

int copy_base_config(char *new_entry_path) {
    char xconfig_ch;
    FILE *xinitrc = NULL;
    FILE *new_entry = NULL;
    int res = 1;

    if (!get_xconfig() || access(get_xconfig(), R_OK)) {
        new_entry = fopen(new_entry_path, "w");
        if (!new_entry)
            return res;

        res = write_exec_command(new_entry);

        fclose(new_entry);
        return res;
    }

    xinitrc = fopen(get_xconfig(), "r");
    new_entry = fopen(new_entry_path, "w");
    if (!new_entry)
        return res;

    while ((xconfig_ch = fgetc(xinitrc)) != EOF) {
        fputc(xconfig_ch, new_entry);
    }
    
    res = write_exec_command(new_entry);
    fclose(xinitrc);
    fclose(new_entry);

    return res;
}

int entry_invalid(const char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int add_entry(const char *new_entry) {
    char *new_entry_path = NULL;
    int res = 1;

    if (entry_invalid(new_entry)) { 
        error_invalid_entry();
        return res;
    }

    new_entry_path = sldm_config_append(new_entry);
    if (!new_entry_path)
        return res;

    if (access(new_entry_path, W_OK) == 0) {
        error("Entry with name '%s' already exists.", new_entry);
        free(new_entry_path);
        return res;
    }
    
    res = copy_base_config(new_entry_path);
    if (!res) 
        printf("Added new entry with name '%s'\n", new_entry);

    free(new_entry_path);
    return res;
}

int remove_entry(const char *entry_name) {
    char *remove_entry_path = NULL;
    int res = 1;
    
    if (entry_invalid(entry_name)) {
        error_invalid_entry();
        return res;
    }

    remove_entry_path = sldm_config_append(entry_name);
    if (!remove_entry_path)
        return res;

    res = remove(remove_entry_path);
    if (!res) {
        printf("Entry with name '%s' was removed\n", entry_name);
    } else if(errno == ENOENT) {
        err_noentry_found(entry_name);
    } else { 
        fatal();
    }

    free(remove_entry_path);
    return res;
}

int readdir_entries(const char *entry_name) {
    DIR *edir;
    struct dirent *edirent = NULL;
    size_t entrid;

    edir = opendir(get_sldm_config_dir());
    if (!edir)
        return 1;

    for (entrid = 0; (edirent = readdir(edir)); ++entrid) {
        if (edirent->d_type == DT_REG) {
            if (entry_name) {
                if (strcmp(edirent->d_name, entry_name)) {
                    printf("(%lu) %s\n", entrid, entry_name);
                    break;
                }
            } else {
                printf("(%lu) %s\n", entrid, edirent->d_name);
            }
        }
    }
    
    return 0;
}

int list_entries(const char *entry_name) {
    int res;

    if (entry_invalid(entry_name)) {
        entry_name = NULL;
    }

    res = readdir_entries(entry_name);
    if (res)
        fatal();

    return res;
}

int show_entry(const char *entry_name) {
    FILE *entryf = NULL;
    char *show_entry_path = NULL;
    int res;

    if (entry_invalid(entry_name)) {
        error_invalid_entry();
        return 1;
    }

    show_entry_path = sldm_config_append(entry_name);
    entryf = fopen(show_entry_path, "r");

    if (entryf) {
        for(char cch = fgetc(entryf); cch != EOF; cch = fgetc(entryf))
            fputc(cch, stdout);
        
        fputc('\n', stdout);
    } else if (errno == ENOENT) {
        err_noentry_found(entry_name);
    } else {
        fatal();
    }

    free(show_entry_path);
    return res;
}

