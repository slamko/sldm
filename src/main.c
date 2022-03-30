#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include "xconfig-names.h"
#include "log-utils.h"
#include "command-names.h"

extern int errno;
char *add_entry_command;
char **entry_table_buf;

int write_exec_command(FILE *fp) {
    char *exec_line;
    char *exec_command;

    exec_line = (char *)calloc(EXEC_C_LENGTH + 1 + strlen(add_entry_command) + 1, sizeof(char));

    if (!exec_line) {
        fclose(fp);
        return 1;
    }

    exec_command = EXEC_C;
    strcpy(exec_line, exec_command);
    exec_line[EXEC_C_LENGTH] = ' ';
    strcpy(exec_line + EXEC_C_LENGTH + 1, add_entry_command);

    if (fputs(exec_line, fp) == EOF) {
        free(exec_line);
        return 1;
    }

    free(exec_line);
    return 0;
}

int copy_base_config(char *new_entry_path) {
    char xconfig_ch;
    FILE *xinitrc;
    FILE *new_entry;
    int res = 1;

    if (access(get_xconfig(), R_OK)) {
        new_entry = fopen(new_entry_path, "w");
        if (!new_entry)
            return res;

        res = write_exec_command(new_entry);

        fclose(new_entry);
        return res;
    }

    xinitrc = fopen(get_xconfig(), "r");
    new_entry = fopen(new_entry_path, "w");
    if (!new_entry)
        return res;

    while ((xconfig_ch = fgetc(xinitrc)) != EOF) {
        fputc(xconfig_ch, new_entry);
    }
    
    res = write_exec_command(new_entry);
    fclose(xinitrc);
    fclose(new_entry);

    return res;
}

int entry_invalid(char *new_entry) {
    return !new_entry || new_entry[0] == '\0';
}

int add_entry(char *new_entry) {
    char *new_entry_path;
    int res = 1;

    if (entry_invalid(new_entry)) 
        return res;

    new_entry_path = sldm_config_append(new_entry);
    if (!new_entry_path)
        return res;

    if (access(new_entry_path, W_OK) == 0) {
        error("Entry with name '%s' already exists.", new_entry);
        return res;
    }
    
    res = copy_base_config(new_entry_path);
    if (!res) 
        printf("Added new entry with name '%s'\n", new_entry);

    free(new_entry_path);
    return res;
}

int remove_entry(char *entry_name) {
    char *remove_entry_path;
    int res = 1;
    
    if (entry_invalid(entry_name)) 
        return res;

    remove_entry_path = sldm_config_append(entry_name);
    if (!remove_entry_path)
        return res;

    res = remove(remove_entry_path);
    if (!res) 
        printf("Entry with name '%s' was removed\n", entry_name);
    else if(errno == 2)
        error("No entry found with name: '%s'", entry_name);
        
    free(remove_entry_path);
    return res;
}

int list_entries(char *entry_name) {
    char *list_entry_path;
    int res = 1;

    if (entry_invalid(entry_name))
        return execl("/bin/ls", "/bin/ls", get_sldm_config_dir(), NULL);
    
    list_entry_path = sldm_config_append(entry_name);
    res = execl("/bin/ls", "/bin/ls", list_entry_path, NULL);
    free(list_entry_path);
    return res;
}

void start_x(char *entry_name) {
    char *entry_config_path;
    int pid;

    entry_config_path = sldm_config_append(entry_name);

    if (!entry_config_path)
        exit(1);

    if (access(entry_config_path, R_OK)) {
        printf("pasth: %s", entry_config_path);
        error("No entry found with a given name\n");
        exit(1);
    }

    pid = fork();
    if (pid == -1) 
        exit(1);

    if (pid == 0) 
        execl("/bin/startx", "/bin/startx", entry_config_path, NULL);
    else {
        free(entry_config_path);
        if (entry_table_buf)
            free(entry_table_buf);

        exit(0);
    }
}

void force_runx_x() {
    start_x(entry_table_buf[0]);
    exit(0);
}

int timer = 10 + 1;

void update_timer() {
    printf("\rEnter an entry number (%ds): ", timer - 1);
    timer--;
}

void prompt_number(int entry_count) {
    int selected_entry;
    int timer_pid;
    timer_pid = fork();

    if (timer_pid == 0) {
        int timer = 10;
        for (int i = timer + 1; i > 0; i--) {
            kill(getppid(), SIGUSR2);
            sleep(1);
        }
        printf("\n");
        kill(getppid(), SIGUSR1);
    } else {
        struct sigaction sa1 = { 0 };
        struct sigaction sa2 = { 0 };

        //sa1.sa_flags = SA_RESTART;
        sa1.sa_handler = &force_runx_x;
        sigaction(SIGUSR1, &sa1, NULL);

        //sa2.sa_flags = SA_RESTART;
        sa2.sa_handler = &update_timer;
        sigaction(SIGUSR2, &sa2, NULL);

        sleep(10);
        int match = scanf("%d", &selected_entry);

        if (match != 1 || selected_entry > entry_count || selected_entry < 0) {
            error("Invalid entry number");
            kill(timer_pid, SIGKILL);
            return prompt_number(entry_count);
        } else if (selected_entry > 0) {
            printf("%s", entry_table_buf[selected_entry - 1]);
            start_x(entry_table_buf[selected_entry - 1]);
        }
        kill(timer_pid, SIGKILL);
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
    char tmp_buf[ENTRY_BUF_SIZE];
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
    ls_command = concat("/bin/ls --sort=time --time=creation -tr ", get_sldm_config_dir());

    ls = popen(ls_command, "r");
    if (!ls) 
        return res;

    printf("Choose an entry (default: 1):\n");

    while (fgets(tmp_buf, ENTRY_BUF_SIZE, ls) != 0) {
        printf("(%d) %s", entry_count + 1, tmp_buf);
        strcpy(entry_table_buf[entry_count], tmp_buf);
        for (int i = strlen(entry_table_buf[entry_count]) - 1; i < ENTRY_BUF_SIZE; i++) {
            entry_table_buf[entry_count][i] = '\0';
        }
        
        entry_count++;
    }

    pclose(ls); 
    free(ls_command);

    printf("(0) Exit\n");

    prompt_number(entry_count);
    for (int i = 0; i < entry_count; i++) {
        free(entry_table_buf[i]);
    }
    free(entry_table_buf);
    return 0;
}