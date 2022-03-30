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

char **entry_table_buf;

void start_x(char *entry_name) {
    char *entry_config_path;
    int pid;

    entry_config_path = sldm_config_append(entry_name);

    if (!entry_config_path)
        exit(1);

    if (access(entry_config_path, R_OK)) {
        error("No entry found with a given name\n");
        exit(1);
    }

    pid = fork();
    if (pid == -1) 
        exit(1);

    if (pid == 0) 
        execl(STARTX, STARTX, entry_config_path, NULL);
    else {
        free(entry_config_path);
        if (entry_table_buf)
            free(entry_table_buf);

        exit(0);
    }
}

void force_runx() {
    start_x(entry_table_buf[0]);
    exit(0);
}

void prompt_number(int entry_count) {
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
            return prompt_number(entry_count);
        } else if (selected_entry > 0) {
            start_x(entry_table_buf[selected_entry - 1]);
        }
    }
}

int prompt(char *entry_name) {
    int res = 1;

    if (!entry_invalid(entry_name)) {
        start_x(entry_name);
        return res;
    }

    FILE *ls;
    DIR *edir;
    struct dirent *entry; 
    char *ls_command;
    int entry_count = 0;   

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

    prompt_number(entry_count);
    for (int i = 0; i < entry_count; i++) {
        free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    return 0;
}