#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <math.h>
#include <signal.h>
#include "config-names.h"
#include "log-utils.h"
#include "command-names.h"

#define ENTRY_PROMPT "\rProvide an entry name or number (%s): "
#define ENTRY_PROMPT_DEFAULT "\rProvide an entry name or number (timeout: %ds): "
#define INITIMER_PID -1

int entry_count = 0;
int timer_pid = INITIMER_PID;
char **entry_table_buf;
WINDOW *win;

void entry_table_buf_dealloc(void) {
    if (!entry_table_buf)
        return;

    for (int i = 0; i < entry_count; i++) {
        if (entry_table_buf[i])
            free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    entry_table_buf = NULL;
}

void ncleanup(void) {
    entry_table_buf_dealloc();
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

void clean_nline() {
    printw("\r\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");
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

int nprompt_number() {
    int selected_entry = default_entry;

    if (prompt_timeout > 0) {
        printw(ENTRY_PROMPT_DEFAULT, prompt_timeout);
        refresh();
        timer_pid = fork();
    } else { 
        printw(ENTRY_PROMPT, "no timeout");
        refresh();
    }

    if (timer_pid == 0) {
        for (int i = prompt_timeout + 1; i > 0; i--)
            sleep(1);

        printw("\nTimeout expired\n");
        printw("Using default entry (%d)\n\n", default_entry);
        refresh();
        kill(getppid(), SIGUSR1);
        exit(0);
    } else {
        char ch;
        int match;
        struct sigaction sa1 = {0};
        char read_buf[ENTRY_NAME_BUF_SIZE] = {0};
        sa1.sa_handler = &force_runx;
        sigaction(SIGUSR1, &sa1, NULL);

        while (1) {
            ch = getch();
            for (size_t i = 0; i < sizeof(read_buf); i++) {
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

            match = sscanf(read_buf, "%d", &selected_entry);
            clean_nline();

            if (match != 1) {
                int name_matches = 0, match_id = 0;
                distillstr(read_buf);
                for (int i = 0; i < entry_count; i++) {
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
            } else if (selected_entry == 0) {
                return 0;
            } else if (selected_entry > entry_count || selected_entry < 0) {
                printw(ENTRY_PROMPT, "Invalid entry number");
            } else if (selected_entry > 0) {
                return start_x(entry_table_buf[selected_entry - 1]);
            }
        }
    }
    return 0;
}

int nprompt(char *entry_name) {
    int res = 1;
    FILE *lsp = NULL;
    DIR *edir = NULL;
    struct dirent *entry = NULL;
    char *ls_command = NULL;
    int initial_entry_count;

    if (!entry_invalid(entry_name)) {
        return start_x(entry_name);
    }

    edir = opendir(get_sldm_config_dir());
    if (!edir)
        return res;

    while ((entry = readdir(edir))) {
        if (entry->d_type == DT_REG)
            entry_count++;
    }
    initial_entry_count = entry_count;
    closedir(edir);

    entry_table_buf = (char **)calloc(entry_count, sizeof(*entry_table_buf));
    if (!entry_table_buf)
        return res;

    for (int i = 0; i < entry_count; i++) {
        entry_table_buf[i] = (char *)calloc(ENTRY_NAME_BUF_SIZE, sizeof(**entry_table_buf));
        
        if (!entry_table_buf[i]) {
            free(entry_table_buf);
            return res;
        }
    }

    entry_count = 0;
    ls_command = sappend(ENTRY_SORT, get_sldm_config_dir());

    lsp = popen(ls_command, "r");
    if (!lsp) 
        return res;

    win = initscr();

    printw("Choose an entry (default: %d):\n", default_entry);

    while (entry_count < initial_entry_count && 
        fgets(entry_table_buf[entry_count], ENTRY_NAME_BUF_SIZE, lsp) != 0) {
        printw("(%d) %s", entry_count + 1, entry_table_buf[entry_count]);
        distillstr(entry_table_buf[entry_count]);
        
        entry_count++;
    }

    pclose(lsp); 
    free(ls_command);

    printw("\n(0) Exit\n");
    refresh();

    res = nprompt_number();
    killtimer();
    ncleanup();
    return res;
}
