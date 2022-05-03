#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ncurses.h>
#include "command-names.h"
#include "nentry-prompt.h"
#include "config-names.h"
#include "utils.h"

int entry_invalid(const char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int partialcmp(const char *entry, const char *cmp) {
    int elen;
    int cmplen;
    
    if (!entry || !cmp)
        return 1;

    elen = strnlen(entry, ENTRY_NAME_BUF_SIZE);
    cmplen = strnlen(cmp, ENTRY_NAME_BUF_SIZE);

    if (elen > cmplen)
        return 1;

    if (elen == cmplen) 
        return strncmp(entry, cmp, ENTRY_NAME_BUF_SIZE);
    
    for (int i = 0; i < elen; i++)
    {
        if (entry[i] != cmp[i])
            return 1; 
    }
    return 0;
}

static int is_reffile_stat(const char *ename) {
    char *fpath = sldm_config_append(ename);
    struct stat fst;
    if (lstat(fpath, &fst) == -1)
        return false;

    return S_ISREG(fst.st_mode);
}

int is_regfile(const struct dirent *finfo) {
    #ifdef _DIRENT_HAVE_D_TYPE
    switch (finfo->d_type)
    {
    case DT_REG:
        return true;
    case DT_UNKNOWN:
        return is_reffile_stat(finfo->d_name);
    default:
        return false;
    }
    #endif
    return is_reffile_stat(finfo->d_name);
}

int sort_entries(const struct dirent **entry, const struct dirent **next) {
    struct stat sentry = {0}, snext = {0};
    char *pentry = NULL, *pnext = NULL;

    pentry = sldm_config_append((*entry)->d_name);
    pnext = sldm_config_append((*next)->d_name);  

    if (stat(pentry, &sentry) == -1) {
        return 1;
    }

    if (stat(pnext, &snext) == -1) {
        return -1;
    }

    free(pentry);
    free(pnext);
    return (int)difftime(snext.st_ctim.tv_sec, sentry.st_mtim.tv_sec);
}

void printf_entry(const char *entry_name, const entryid entrid) {
    printf("(%lu) %s\n", entrid, entry_name);
}

void printw_entry(const char *entry_name, const entryid entrid) {
    printw("(%lu) %s\n", entrid, entry_name);
}



void printdir_entries(struct sorted_entries *sentries, const char *entry_name, onprint_cb onprint) {
    entryid entrid;
    int entrycnt = sentries->entrycnt;

    for (entrid = 0; entrycnt--; free(sentries->sentries[entrycnt])) {
        struct dirent *entry = sentries->sentries[entrycnt];

        onprint(entry);
        entrid++;

    }
}

int getdir_entries(struct sorted_entries *sentries) {
    struct dirent **eentries = NULL;
    int entrcount;

    entrcount = scandir(get_sldm_config_dir(), &eentries, &is_regfile, &sort_entries);
    if (entrcount == -1)
        return 1;
    
    sentries->sentries = eentries;
    sentries->entrycnt = entrcount;
    return 0;
}