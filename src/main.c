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

static int print_entry(const char *entryp, FILE *entryf) {
    struct stat entry_st;
    char *entrybuf = NULL;
    int res = 1;
    
    if (stat(entryp, &entry_st) == -1) {
        perror(ERR_PREF);
        return res;
    }

    entrybuf = calloc(entry_st.st_size, sizeof(*entrybuf));
    if (!entrybuf) {
        perror(ERR_PREF);
        goto cleanup_ebuf;
    }

    fread(entrybuf, sizeof(*entrybuf), entry_st.st_size, entryf);        
    res = write(STDOUT_FILENO, entrybuf, entry_st.st_size) != -1;
    fputc('\n', stdout);

cleanup_ebuf:
    free(entrybuf);
    return res;
}

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

static int copy_base_config(FILE *fentry) {
    FILE *xinitrc = NULL;
    char xconfig_ch;
    int res = 0;

    if (!get_xconfig() || access(get_xconfig(), R_OK)) {
        return res;
    }

    xinitrc = fopen(get_xconfig(), "r");
    if (!xinitrc) {
        return res;
    }

    while ((xconfig_ch = fgetc(xinitrc)) != EOF) {
        if (fputc(xconfig_ch, fentry) == EOF) {
            goto close_xinitrc;
        }
    }
    printf("Copying .xinitrc file from HOME firectory.\n\n");
    
close_xinitrc:
    fclose(xinitrc);
    return res;
}

int add_entry(const char *new_entry) {
    char *new_entry_path = NULL;
    int res = 1;
    FILE *new_entryf = NULL;
    
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
    
    new_entryf = fopen(new_entry_path, "w");
    if (copy_base_config(new_entryf)) {
        perror(ERR_PREF);
        goto cleanup;
    }

    if ((res = write_exec_command(new_entryf))) {
        perror(ERR_PREF);
        goto cleanup;
    }
    
    new_entryf = freopen(new_entry_path, "r", new_entryf);
    if (new_entryf) {
        puts("***");
        print_entry(new_entry_path, new_entryf);
        puts("***\n");
    }
    
    if (res == 0) 
        printf("Added new entry with name '%s'\n", new_entry);

  cleanup:    
    free(new_entry_path);
    fclose(new_entryf);
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
        
        goto cleanup;
    }

    res = print_entry(show_entry_path, entryfp);
    
cleanup:
    fclose(entryfp);
    free(show_entry_path);
    return res;
}

