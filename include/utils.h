
#ifndef SLDM_UTILS_F
#define SLDM_UTILS_F

#include <dirent.h>
#include "command-names.h"
#include <ncurses.h>

typedef unsigned long entryid;

struct sorted_entries {
    struct dirent **sentries;
    int entrycnt;
};

int entry_invalid(const char *new_entry);

int not_in_tty(void);

int partialcmp(const char *entry, const char *cmp);

int is_regfile(const struct dirent *finfo);

int sort_entries(const struct dirent **entry, const struct dirent **next);

void printw_indent(int indenty, int indentx, const char *msg, ...);

void vprintw_indent(int indenty, int indentx, const char *msg, va_list val);

void printw_entry(const char *entry_name, const entryid entrid);

#define NEW_LINE() printw_indent(1, true, "\n")

void printw_indent_next_line(const char *msg, ...);

void printf_entry(const char *entry_name, const entryid entrid);

void destroy_dentries_iterator(struct sorted_entries *sentries);

struct dirent *iter_entry(struct sorted_entries *sentries);

int getdir_entries(struct sorted_entries *sentries);

int check_xconfig(char **xconfig);

#endif
