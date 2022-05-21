#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "config-names.h"
#include "log-utils.h"
#include "utils.h"
#include "nentry-prompt.h"

char *add_entry_command = NULL;

static int write_exec_command(FILE *fp) {
    char *exec_line = NULL;
    int addcmd_len = strnlen(add_entry_command, ENTRY_CMD_BUF_LEN);

    exec_line = calloc(EXEC_C_LENGTH + 1 + addcmd_len + 1, sizeof(*exec_line));

    if (!exec_line) {
        return 1;
    }

    strcpy(exec_line, EXEC_C);
    exec_line[EXEC_C_LENGTH] = ' ';
    strncpy(exec_line + EXEC_C_LENGTH + 1, add_entry_command, addcmd_len);

    if (fputs(exec_line, fp) == EOF) {
        free(exec_line);
        return 1;
    }

    free(exec_line);
    return 0;
}

static int copy_base_config(char *new_entry_path) {
    FILE *xinitrc = NULL;
    FILE *new_entry = NULL;
    char xconfig_ch;
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
    if (!xinitrc) {
        return res;
    }

    new_entry = fopen(new_entry_path, "w");
    if (!new_entry) {
        goto close_xinitrc;
    }

    while ((xconfig_ch = fgetc(xinitrc)) != EOF) {
        fputc(xconfig_ch, new_entry);
    }
    
    res = write_exec_command(new_entry);

    fclose(new_entry);
close_xinitrc:
    fclose(xinitrc);
    return res;
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
        perror(ERR_PREF);
    }

    free(remove_entry_path);
    return res;
}

static void conditional_prinentry(const struct dirent *entry, const char *entry_name, entryid entrid) {
    if (entry_name) {
        if (strcmp(entry->d_name, entry_name) == 0) {
            printf_entry(entry_name, entrid);
        }
    } else {
        printf_entry(entry->d_name, entrid);
    }
}

int list_entries(const char *entry_name) {
    struct sorted_entries sentries = {0};
    struct dirent *centry = NULL;
    int res = 1;

    if (entry_invalid(entry_name)) {
        entry_name = NULL;
    }

    res = getdir_entries(&sentries);
    if (res) {
        perror(ERR_PREF);
        return res;
    }

    for (entryid eid = 1; (centry = iter_entry(&sentries)); eid++) {
        conditional_prinentry(centry, entry_name, eid);
        free(centry);
    }

    destroy_dentries_iterator(&sentries);
    return 0;
}

int show_entry(const char *entry_name) {
    char *show_entry_path = NULL;
    FILE *entryfp = NULL;
    struct stat entry_st;
    char *entrybuf = NULL;
    int res = 1;

    if (entry_invalid(entry_name)) {
        error_invalid_entry();
        return 1;
    }

    show_entry_path = sldm_config_append(entry_name);
    entryfp = fopen(show_entry_path, "r");

    if (!entryfp) {
        if (errno == ENOENT) {
            err_noentry_found(entry_name);
        } else { 
            perror(ERR_PREF);
        }
        
        goto cleanup_entryp;
    }

    if (stat(show_entry_path, &entry_st) == -1) {
        perror(ERR_PREF);
        goto cleanup;
    }

    entrybuf = calloc(entry_st.st_size, sizeof(*entrybuf));
    if (!entrybuf) {
        perror(ERR_PREF);
        goto cleanup;
    }

    fread(entrybuf, sizeof(*entrybuf), entry_st.st_size, entryfp);        
    res = write(STDOUT_FILENO, entrybuf, entry_st.st_size) != -1;
    fputc('\n', stdout);

cleanup:
    fclose(entryfp);
cleanup_entryp:
    free(show_entry_path);
    return res;
}

