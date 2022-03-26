//#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include "xconfig-names.h"
#include "log-utils.h"

int copy_base_config(char *new_entry) {
    FILE *xinitrc;
    xinitrc = fopen(get_xconfig(), "r");
    
    if (!xinitrc) {

    }
}

int list_sldm_entries(char *name) {
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir("");
    if (pDir == NULL) {
        if (ENOENT == errno){

        }
        printf("Cannot open directory '%s'\n", "");
        return 1;
    } 
    while ((pDirent = readdir(pDir)) != NULL) {
        printf("[%s]\n", pDirent->d_name);
    }

    closedir(pDir);
    return 0;    
}