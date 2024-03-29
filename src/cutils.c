#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <stdarg.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ncurses.h>
#include <errno.h>
#include <wordexp.h>
#include "nentry-prompt.h"
#include "config-names.h"
#include "command-names.h"
#include "utils.h"
#include "log-utils.h"

int entry_invalid(const char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int not_in_tty(void) {
    char tty_output[TTY_MIN_NAME_LEN];
    int cmp = 1;

    if (ttyname_r(STDIN_FILENO, tty_output, sizeof(tty_output)) == 0) {
        cmp = strncmp(tty_output, TTY_DEVICE, TTY_DEVICE_NAME_BYTES);
    } else if (errno != ENODEV) {
        perror(ERR_PREF);
    }

    return cmp;
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

static int is_regfile_stat(const char *ename) {
    char *fpath = sldm_config_append(ename);
    struct stat fst = {0};

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
        return is_regfile_stat(finfo->d_name);
    default:
        return false;
    }
    #endif
    return is_regfile_stat(finfo->d_name);
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

void printw_indent(int indenty, int indentx, const char *msg, ...) {
	va_list val;

	va_start(val, msg);
	move(y_line, indentx);
    vw_printw(win, msg, val);

	y_line += indenty;
    va_end(val);
}

void vprintw_indent(int indenty, int indentx, const char *msg, va_list val) {
	move(y_line, indentx);
    vw_printw(win, msg, val);
	y_line += indenty;
}

void printw_entry(const char *entry_name, const entryid entrid) {
    printw_indent(1, 1,  "(%lu) %s\n", entrid, entry_name);
}

void printw_indent_next_line(const char *msg, ...) {
	va_list val;
	va_start(val, msg);
	vprintw_indent(1, 1, msg, val);
	va_end(val);
}

void printf_entry(const char *entry_name, const entryid entrid) {
    printf("(%lu) %s\n", entrid, entry_name);
}

void destroy_dentries_iterator(struct sorted_entries *sentries) {
    if (sentries->sentries) {
        free(sentries->sentries);
        sentries->sentries = NULL;
    }
}

struct dirent *iter_entry(struct sorted_entries *sentries) {
    sentries->entrycnt--;
    if (sentries->entrycnt >= 0) 
        return sentries->sentries[sentries->entrycnt];
    

    return NULL;
}

int getdir_entries(struct sorted_entries *sentries) {
    struct dirent **eentries = NULL;
    int entrcount;

    entrcount = scandir(get_sldm_config_entries(), &eentries, &is_regfile, &sort_entries);
    if (entrcount == -1)
        return 1;
    
    sentries->sentries = eentries;
    sentries->entrycnt = entrcount;
    return 0;
}

int check_xconfig(char **xconfig) {
    wordexp_t exp_result;

    if (!*xconfig) {
        return 0;
    }
    wordexp(*xconfig, &exp_result, 0);

    if (access(exp_result.we_wordv[0], R_OK)){
        wordfree(&exp_result);
        return EXIT_FAILURE;
    }

    *xconfig = strdup(exp_result.we_wordv[0]);
    wordfree(&exp_result);
    return !*xconfig;
}

