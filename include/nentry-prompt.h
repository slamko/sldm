#ifndef NENTRYPROMPT_H
#define NENTRYPROMPT_H

#include <dirent.h>
#include <ncurses.h>

extern WINDOW *win;
extern char **entry_table_buf;
extern int y_line;

int nprompt(const char *entry_name);

#endif
