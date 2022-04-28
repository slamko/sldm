#include <dirent.h>
#include "command-names.h"

int entry_invalid(const char *new_entry);

int partialcmp(const char *entry, const char *cmp);

int is_regfile(struct dirent *finfo, const char *fpath);