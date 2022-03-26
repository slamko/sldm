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

int copy_base_config(char *new_entry_path) {
    FILE *xinitrc;
    FILE *new_entry;

    if (access(get_xconfig(), R_OK)) {
        fopen(new_entry_path, "w");
    }
}

int add_entry(char *new_entry) {
    if (!new_entry || new_entry[0] == '\0') 
        return 1;

    char *new_entry_path = home_path_append(new_entry);
    if (!new_entry_path)
        return 1;
    
    return copy_base_config(new_entry_path);
}

int remove_entry(char *entry_name) {
    
}

int prompt(char *entry_name) {
    if (entry_name) {

    }
}