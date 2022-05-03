#include <dirent.h>
#include "command-names.h"

int entry_invalid(const char *new_entry);

int partialcmp(const char *entry, const char *cmp);

int is_regfile(const struct dirent *finfo);

int sort_entries(const struct dirent **entry, const struct dirent **next);

typedef void (*onprint_cb)(const struct dirent *dentry);

int printdir_entries(struct sorted_entries *sentries, const char *entry_name, onprint_cb onprint);

int getdir_entries(struct sorted_entries *sentries);