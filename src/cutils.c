#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
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