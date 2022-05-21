#ifndef NENTRYPROMPT_H
#define NENTRYPROMPT_H

#include <dirent.h>

extern char **entry_table_buf;

int nprompt(const char *entry_name);

typedef unsigned long entryid;

struct sorted_entries {
    struct dirent **sentries;
    int entrycnt;
};

#endif