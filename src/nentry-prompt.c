#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <math.h>
#include <signal.h>
#include <ctype.h>
#include "config-names.h"
#include "nentry-prompt.h"
#include "log-utils.h"
#include "utils.h"

#define ENTRY_PROMPT "\rProvide an entry name or number (%s): "
#define ENTRY_PROMPT_DEFAULT "\rProvide an entry name or number (timeout: %ds): "
#define INITIMER_PID -1

#define NN(S) "\n"S"\n"

entryid entry_count = 0;
int timer_pid = INITIMER_PID;
char **entry_table_buf;
WINDOW *win;

void entry_table_buf_dealloc(void) {
    if (!entry_table_buf)
        return;

    for (entryid i = 0; i < entry_count; i++) {
        if (entry_table_buf[i])
            free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    entry_table_buf = NULL;
}

void ncleanup(void) {
    entry_table_buf_dealloc();
    wclear(win);
    refresh();
    endwin();
}

void killtimer(void) {
    if (timer_pid > 0) {
        kill(timer_pid, SIGKILL);
        timer_pid = INITIMER_PID;
    }
}

int distillstr(char *str) {
    if (!str)
        return 1;

    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == '\n' || str[i] == '\t') 
            str[i] = '\0';
    }
    return 0;
}

void clean_nline(void) {
    wdeleteln(win);
    refresh();
}

int start_x(char *entry_name) {
    char *entry_config_path = NULL;
    int pid;
    int res = 1;

    killtimer();
    entry_config_path = sldm_config_append(entry_name);

    if (!entry_config_path)
        return res;

    if (access(entry_config_path, R_OK)) {
        nerror("No entry found with a given name\n");
        goto cleanup;
        return res;
    }

    printw("\n\n\n\nRunning %s ...", entry_name);

    pid = fork();
    if (pid == -1) {
        goto cleanup;
        return res;
    }

    if (pid == 0)
        execl(STARTX, STARTX, entry_config_path, NULL);

    res = !pid;

cleanup:
    free(entry_config_path);
    entry_table_buf_dealloc();

    return res;
}

void force_runx() {
    int res = 1;
    sleep(1);

    if (default_entry > entry_count || default_entry <= 0) {
        nerror("Invalid default entry (%d)", default_entry);
        goto cleanup;
    }

    res = start_x(entry_table_buf[default_entry - 1]);

cleanup:
    ncleanup();
    exit(res);
}

int is_valid_entrynum(char *entrye) {
    for (int c = *entrye; (c = *entrye) != '\0'; entrye++) {
        if (!(c >= '0' && c <= '9'))
            return 0;
    }
    return 1;
}

void run_prompt_timer(void) {
    for (int i = prompt_timeout + 1; i > 0; i--)
        sleep(1);

    printw(NN("Timeout expired"));
    printw("Using default entry (%d)\n\n", default_entry);
    refresh();
    kill(getppid(), SIGUSR1);
    exit(0);
}

int handle_entrynum(void) {
    char ch;
    entryid selected_entry = default_entry;
    struct sigaction sa1 = {0};
    char read_buf[ENTRY_NAME_BUF_SIZE] = {'\0'};
    sa1.sa_handler = &force_runx;
    sigaction(SIGUSR1, &sa1, NULL);

    while (1) {
        ch = getch();
        for (size_t i = 0; i < sizeof(read_buf) - 1; i++) {
            if (ch == '\n') {
                read_buf[i] = ch;
                break;
            } else if (ch != '\n' && ch != EOF) {
                read_buf[i] = ch;
                ch = getch();
            } else {
                break;
            }
        }

        if (read_buf[0] == '\n') {
            return start_x(entry_table_buf[default_entry - 1]);
        }

        clean_nline();
        distillstr(read_buf);

        if (is_valid_entrynum(read_buf)) {
            selected_entry = strtoul(read_buf, NULL, 10);
            if (selected_entry == 0) {
                return 0;
            } else if (selected_entry > entry_count) {
                printw(ENTRY_PROMPT, "Invalid entry number");
            } else if (selected_entry > 0) {
                return start_x(entry_table_buf[selected_entry - 1]);
            }
        } else {
            int name_matches = 0, match_id = 0;
            for (entryid i = 0; i < entry_count; i++) {
                if (strcmp(entry_table_buf[i], read_buf) == 0) {
                   return start_x(entry_table_buf[i]);
                }
                if (partialcmp(read_buf, entry_table_buf[i]) == 0) {
                    name_matches++;
                    match_id = i;
                }
            }

            switch (name_matches) {
            case 0:
                printw(ENTRY_PROMPT, "Invalid entry name");
                break;
            case 1:
                return start_x(entry_table_buf[match_id]);
            default:
                printw(ENTRY_PROMPT, "Disambiguous between multiple entry names");
                break;
            }
        }

        killtimer();
        if (prompt_timeout > 0) {
            timer_pid = fork();
        }
        if (timer_pid == 0) {
            run_prompt_timer();
        }
    }
}

int nprompt_number() {
    clean_nline();
    if (prompt_timeout > 0) {
        printw(ENTRY_PROMPT_DEFAULT, prompt_timeout);
        refresh();
        timer_pid = fork();
    } else { 
        printw(ENTRY_PROMPT, "no timeout");
        refresh();
    }

    if (timer_pid == 0) {
        run_prompt_timer();
    } else {
        return handle_entrynum();
    }
    return 0;
}

int nprompt(char *entry_name) {
    int res = 1;
    struct dirent *centry = NULL;
    struct sorted_entries sentries;

    if (!entry_invalid(entry_name)) {
        return start_x(entry_name);
    }

    getdir_entries(&sentries);
    entry_count = sentries.entrycnt;

    entry_table_buf = (char **)calloc(entry_count, sizeof(*entry_table_buf));
    if (!entry_table_buf)
        return res;

    for (entryid i = 0; i < entry_count; i++) {
        entry_table_buf[i] = calloc(ENTRY_NAME_BUF_SIZE + 1, sizeof(**entry_table_buf));
        
        if (!entry_table_buf[i]) {
            free(entry_table_buf);
            return res;
        }
    }

    win = initscr();
    win->_scroll = true;
    printw("Choose an entry (default: %d):\n", default_entry);

    for (entryid eid = 1; (centry = iter_entry(&sentries)); eid++) {
        strncpy(entry_table_buf[eid - 1], centry->d_name, ENTRY_NAME_BUF_SIZE);
        printw_entry(centry->d_name, eid);
        free(centry);
    }

    destroy_dentries_iterator(&sentries);
    printw(NN("(0) Exit"));
    refresh();

    res = nprompt_number();
    killtimer();
    ncleanup();
    return res;
}
