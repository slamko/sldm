#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "command-names.h"

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

    if (elen == cmplen) 
        return strncmp(entry, cmp, ENTRY_NAME_BUF_SIZE);
    
    for (int i = 0; i < (elen > cmplen ? cmplen : elen); i++)
    {
        if (entry[i] != cmp[i])
            return 1; 
    }
    return 0;
}

static int is_reffile_stat(const char *fpath) {
    struct stat fst;
    if (lstat(fpath, &fst) == -1)
        return false;

    return S_ISREG(fst.st_mode);
}

int is_regfile(struct dirent *finfo, const char *fpath) {
    #ifdef _DIRENT_HAVE_D_TYPE
    switch (finfo->d_type)
    {
    case DT_REG:
        return true;
    case DT_UNKNOWN:
        return is_reffile_stat(fpath);
    default:
        return false;
    }
    #endif
    return is_reffile_stat(fpath);
}