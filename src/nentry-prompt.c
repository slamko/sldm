#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <ncurses.h>
#include <signal.h>
#include "config-names.h"
#include "log-utils.h"
#include "command-names.h"

int entry_count = 0;
char **entry_table_buf = NULL;

void entry_table_buf_dealloc() {
    if (!entry_table_buf)
        return;

    for (int i = 0; i < entry_count; i++) {
        if (entry_table_buf[i])
            free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    entry_table_buf = NULL;
}

void ncleanup() {
    entry_table_buf_dealloc();
    refresh();
    endwin();
}

int start_x(char *entry_name) {
    char *entry_config_path;
    int pid;
    int res = 1;

    entry_config_path = sldm_config_append(entry_name);

    if (!entry_config_path)
        return res;

    if (access(entry_config_path, R_OK)) {
        nerror("No entry found with a given name\n");
        goto cleanup;
        return res;
    }

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
    int timer_pid;

    timer_pid = fork();

    if (timer_pid == 0) {
        int timer = prompt_timeout;
        for (int i = timer + 1; i > 0; i--) {
            printw("\rEnter an entry number (%ds): ", i - 1);
            refresh();
            sleep(1);
        }
        printw("\n");
        refresh();
        kill(getppid(), SIGUSR1);
    } else {
        char ch;
        int match;
        struct sigaction sa1 = { 0 };
        char read_buf[16];
        sa1.sa_handler = &force_runx;
        sigaction(SIGUSR1, &sa1, NULL);

        ch = getch();
        for (int i = 0; i < sizeof(read_buf); i++) {
            if (ch == '\n') {
                read_buf[i] = ch;
                ch = getch();
                break;
            } else if (ch != '\n' && ch != EOF) {
                read_buf[i] = ch;
                ch = getch();
            } else {
                break;
            }
        }

        if (read_buf[0] == '\n') {
            kill(timer_pid, SIGKILL);
            return start_x(entry_table_buf[default_entry - 1]);
        }
        match = sscanf(read_buf, "%d", &selected_entry);

        kill(timer_pid, SIGKILL);
        if (match != 1 || selected_entry > entry_count || selected_entry < 0) {
            nerror("Invalid entry number");
            return nprompt_number();
        } else if (selected_entry > 0) {
            return start_x(entry_table_buf[selected_entry - 1]);
        }
    }
    return 0;
}

int nprompt(char *entry_name) {
    int res = 1;

    if (!entry_invalid(entry_name)) {
        return start_x(entry_name);
    }

    FILE *lsp;
    DIR *edir;
    struct dirent *entry; 
    char *ls_command; 
    int initial_entry_count;

    edir = opendir(get_sldm_config_dir());
    if (!edir)
        return res;

    while ((entry = readdir(edir))) {
        if (entry->d_type == DT_REG)
            entry_count++;
    }
    initial_entry_count = entry_count;
    closedir(edir);

    entry_table_buf = (char **)calloc(entry_count, sizeof(char *));
    for (int i = 0; i < entry_count; i++) {
        entry_table_buf[i] = (char *)calloc(ENTRY_BUF_SIZE, sizeof(char));
    }

    if (!entry_table_buf)
        return res;

    entry_count = 0;
    ls_command = concat(ENTRY_SORT, get_sldm_config_dir());

    lsp = popen(ls_command, "r");
    if (!lsp) 
        return res;

    initscr();

    printw("Choose an entry (default: %d):\n", default_entry);

    while (entry_count < initial_entry_count && fgets(entry_table_buf[entry_count], ENTRY_BUF_SIZE, lsp) != 0) {
        printw("(%d) %s", entry_count + 1, entry_table_buf[entry_count]);
        for (int i = strlen(entry_table_buf[entry_count]) - 1; i < ENTRY_BUF_SIZE; i++) {
            entry_table_buf[entry_count][i] = '\0';
        }
        
        entry_count++;
    }

    pclose(lsp); 
    free(ls_command);

    printw("\n(0) Exit\n");
    refresh();

    res = nprompt_number();
    ncleanup();
    return res;
}