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
char *add_entry_command;

int partialcmp(const char *entry, const char *cmp) {
    int elen;
    int cmplen;
    
    if (!entry || !cmp)
        return 1;

    elen = strlen(entry);
    cmplen = strlen(cmp);

    if (elen == cmplen) 
        return strcmp(entry, cmp);
    
    for (int i = 0; i < (elen > cmplen ? cmplen : elen); i++)
    {
        if (entry[i] != cmp[i])
            return 1; 
    }
    return 0;
}

int write_exec_command(FILE *fp) {
    char *exec_line;
    char *exec_command;

    exec_line = (char *)calloc(EXEC_C_LENGTH + 1 + strlen(add_entry_command) + 1, sizeof(char));

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
    FILE *xinitrc;
    FILE *new_entry;
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

int entry_invalid(char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int add_entry(char *new_entry) {
    char *new_entry_path;
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

int remove_entry(char *entry_name) {
    char *remove_entry_path;
    int res = 1;
    
    if (entry_invalid(entry_name)) {
        error_invalid_entry();
        return res;
    }

    remove_entry_path = sldm_config_append(entry_name);
    if (!remove_entry_path)
        return res;

    res = remove(remove_entry_path);
    if (!res) 
        printf("Entry with name '%s' was removed\n", entry_name);
    else if(errno == 2)
        error("No entry found with name: '%s'", entry_name);
        
    free(remove_entry_path);
    return res;
}

int list_entries(char *entry_name) {
    char *list_entry_path;
    int res = 1;
    int exec = -1;

    if (entry_invalid(entry_name)) {
        exec = fork();
        if (exec == -1)
            return res;

        if (exec == 0) 
            execl(LS_BIN, LS_BIN, get_sldm_config_dir(), NULL);

        wait(&res);
        return res;
    }

    list_entry_path = sldm_config_append(entry_name);
    exec = fork();
    if (exec == -1)
        goto cleanup;

    if (exec == 0)
        execl(LS_BIN, LS_BIN, list_entry_path, NULL);

    wait(&res);

cleanup:
    free(list_entry_path);
    return res;
}

int show_entry(char *entry_name) {
    char *show_entry_path;
    int res;
    int exec;

    if (entry_invalid(entry_name)) {
        error_invalid_entry();
        return 1;
    }

    show_entry_path = sldm_config_append(entry_name);
    if ((res = access(show_entry_path, R_OK))) {
        error("Unable to access file at path: %s", show_entry_path);
        goto cleanup;
    }

    exec = fork();
    if (exec == -1)
        goto cleanup;

    if (exec == 0)
        execl(CAT_BIN, CAT_BIN, show_entry_path, NULL);

    wait(&res);
    printf("\n");

cleanup:
    free(show_entry_path);
    return res;
}

