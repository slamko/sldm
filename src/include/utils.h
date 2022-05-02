#include <dirent.h>
#include "command-names.h"

int entry_invalid(const char *new_entry);

int partialcmp(const char *entry, const char *cmp);

int is_regfile(const struct dirent *finfo);

int sort_entries(const struct dirent **entry, const struct dirent **next);

int printdir_entries(struct sorted_entries sentries, const char *entry_name);