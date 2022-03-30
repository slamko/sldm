#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include "config-names.h"
#include "log-utils.h"
#include "command-names.h"

int entry_count = 0;
char **entry_table_buf = NULL;

void entry_table_buf_dealloc() {
    for (int i = 0; i < entry_count; i++) {
        free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    entry_table_buf = NULL;
}

void startx_free(char *entry_config_path) {
    free(entry_config_path);
    if (entry_table_buf)
        entry_table_buf_dealloc();
}

int start_x(char *entry_name) {
    char *entry_config_path;
    int pid;

    entry_config_path = sldm_config_append(entry_name);

    if (!entry_config_path)
        return 1;

    if (access(entry_config_path, R_OK)) {
        error("No entry found with a given name\n");
        startx_free(entry_config_path);
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        startx_free(entry_config_path);
        return 1;
    }

    if (pid == 0) 
        execl(STARTX, STARTX, entry_config_path, NULL);
    else {
        startx_free(entry_config_path);
        return 0;
    }
    return 0;
}

void force_runx() {
    int st = start_x(entry_table_buf[0]);
    exit(st);
}

int prompt_number() {
    int selected_entry;
    int timer_pid;
    timer_pid = fork();

    if (timer_pid == 0) {
        int timer = 10;
        for (int i = timer + 1; i > 0; i--) {
            printf("\rEnter an entry number (%ds): ", i - 1);
            fflush(stdout);
            sleep(1);
        }
        printf("\n");
        kill(getppid(), SIGUSR1);
    } else {
        struct sigaction sa1 = { 0 };
        sa1.sa_handler = &force_runx;
        sigaction(SIGUSR1, &sa1, NULL);

        int match = scanf("%d", &selected_entry);

        kill(timer_pid, SIGKILL);
        if (match != 1 || selected_entry > entry_count || selected_entry < 0) {
            error("Invalid entry number");
            return prompt_number();
        } else if (selected_entry > 0) {
            return start_x(entry_table_buf[selected_entry - 1]);
        }
    }
    return 0;
}

int prompt(char *entry_name) {
    int res = 1;

    if (!entry_invalid(entry_name)) {
        return start_x(entry_name);
    }

    FILE *ls;
    DIR *edir;
    struct dirent *entry; 
    char *ls_command; 

    edir = opendir(get_sldm_config_dir());
    if (!edir)
        return res;

    while ((entry = readdir(edir))) {
        if (entry->d_type == DT_REG)
            entry_count++;
    }
    closedir(edir);

    entry_table_buf = (char **)calloc(entry_count, sizeof(char *));
    for (int i = 0; i < entry_count; i++) {
        entry_table_buf[i] = (char *)calloc(ENTRY_BUF_SIZE, sizeof(char));
    }

    if (!entry_table_buf)
        return res;

    entry_count = 0;
    ls_command = concat(ENTRY_SORT, get_sldm_config_dir());

    ls = popen(ls_command, "r");
    if (!ls) 
        return res;

    printf("Choose an entry (default: 1):\n");

    while (fgets(entry_table_buf[entry_count], ENTRY_BUF_SIZE, ls) != 0) {
        printf("(%d) %s", entry_count + 1, entry_table_buf[entry_count]);
        for (int i = strlen(entry_table_buf[entry_count]) - 1; i < ENTRY_BUF_SIZE; i++) {
            entry_table_buf[entry_count][i] = '\0';
        }
        
        entry_count++;
    }

    pclose(ls); 
    free(ls_command);

    printf("\n(0) Exit\n");

    res = prompt_number();
    entry_table_buf_dealloc();
    return res;
}