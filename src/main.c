#include "xconfig-utils.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include "xconfig-names.h"
#include "log-utils.h"
#include "main.h"

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
    exec_line[EXEC_C_LENGTH] = " ";
    strcpy(exec_line + EXEC_C_LENGTH + 1, add_entry_command);
    fputs(exec_line, fp);
    free(exec_line);

    return 0;
}

int copy_base_config(char *new_entry_path) {
    char xconfig_ch;
    FILE *xinitrc;
    FILE *new_entry;

    if (access(get_xconfig(), R_OK)) {
        new_entry = fopen(new_entry_path, "w");
        if (write_exec_command(new_entry)) 
            return 1;

        fclose(new_entry);
        return 0;
    }

    xinitrc = fopen(get_xconfig(), "r");
    new_entry = fopen(new_entry_path, "rw");

    while ((xconfig_ch = fgetc(xinitrc)) != EOF) {
        fputc(xconfig_ch, new_entry);
    }
    
    write_exec_command(new_entry);
    fclose(xinitrc);
    fclose(new_entry);
}

int entry_invalid(char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int add_entry(char *new_entry) {
    char *new_entry_path;
    int res;

    if (entry_invalid(new_entry)) 
        return 1;

    new_entry_path = home_path_append(new_entry);
    if (!new_entry_path)
        return 1;

    if (access(new_entry_path, W_OK) == 0) {
        error("Entry with name '%s' already exists.", new_entry);
        return 1;
    }
    
    res = copy_base_config(new_entry_path);
    if (res) 
        pritnf("Added new entry with name '%s'", new_entry);

    return res;
}

int remove_entry(char *entry_name) {
    char *remove_entry_path;
    int res;
    
    if (entry_invalid(entry_name)) 
        return 1;

    remove_entry_path = home_path_append(entry_name);
    if (!remove_entry_path)
        return 1;

    res = remove(remove_entry_path);
    if (res) 
        printf("Entry was removed");

    return res;
}

int prompt(char *entry_name) {
    if (entry_invalid(entry_name)) 
        return 1;
}