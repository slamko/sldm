#include <stdarg.h>
#include <stdbool.h>
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

#define ENTRY_PROMPT "Provide an entry name or number (%s): "
#define ENTRY_PROMPT_DEFAULT "Provide an entry name or number (timeout: %ds): "
#define INITIMER_PID -1
#define READ_USR_ENTRY 1

#define NN(S) "\n"S"\n"

entryid entry_count = 0;
int timer_pid = INITIMER_PID;
char **entry_table_buf;
char *bufval;
WINDOW *win;
int y_line = 1;

void printw_indent(int indentx, bool indenty, const char *msg, ...) {
	va_list val;

	va_start(val, msg);
	move(y_line, indentx);
    vw_printw(win, msg, val);

    if (indenty)
		y_line++;

    va_end(val);
}

void vprintw_indent(int indentx, int indenty, const char *msg, va_list val) {
	move(y_line, indentx);
    vw_printw(win, msg, val);
	y_line += indenty;
}

void printw_entry(const char *entry_name, const entryid entrid) {
    printw_indent(1, true,  "(%lu) %s\n", entrid, entry_name);
}

#define NEW_LINE() printw_indent(1, true, "\n")

void printw_indent_next_line(const char *msg, ...) {
	va_list val;
	va_start(val, msg);
	vprintw_indent(1, true, msg, val);
	va_end(val);
}

static void entry_table_buf_dealloc(void) {
    if (!entry_table_buf)
        return;

    free(bufval);
    free(entry_table_buf);
    bufval = NULL;
    entry_table_buf = NULL;
}

static void ncleanup(void) {
    entry_table_buf_dealloc();
    wclear(win);
    refresh();
    endwin();
}

static void killtimer(void) {
    if (timer_pid > 0) {
        kill(timer_pid, SIGKILL);
        timer_pid = INITIMER_PID;
    }
}

static int distillstr(char *str) {
    if (!str)
        return 1;

    for (size_t i = 0; i < strlen(str); i++) {
        if (str[i] == '\n' || str[i] == '\t') 
            str[i] = '\0';
    }
    return 0;
}

static void clean_nline(void) {
	move(y_line, 1);
	clrtoeol();
	refresh();
}

static int start_x(const char *entry_name) {
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

	NEW_LINE();
	NEW_LINE();
	NEW_LINE();
    printw_indent_next_line("Running %s ...", entry_name);

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

static void force_runx() {
    int res = 1;
    sleep(1);

    if (default_entry > entry_count || default_entry <= 0) {
        nerror("Invalid default entry (%lu)", default_entry);
        goto cleanup;
    }

    res = start_x(entry_table_buf[default_entry - 1]);

cleanup:
    ncleanup();
    exit(res);
}

static int is_valid_entrynum(char *entrye) {
    if (!entrye)
        return 0;

    for (int c = *entrye; (c = *entrye) != '\0'; entrye++) {
        if (!isdigit(c))
            return 0;
    }
    return 1;
}

static void run_prompt_timer(void) {
    for (int i = prompt_timeout + 1; i > 0; i--)
        sleep(1);

	NEW_LINE();
    printw_indent_next_line("Timeout expired");
	NEW_LINE();
    printw_indent_next_line("Using default entry (%lu)\n", default_entry);
	NEW_LINE();
	
    refresh();
    kill(getppid(), SIGUSR1);
    exit(0);
}

static void read_user_entry(char *read_buf, size_t readbuf_siz) {
    char ch;

    ch = getch();
    for (size_t i = 0; i < readbuf_siz - 1; i++) {
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
}

static int handle_entrynum(void) {
    char read_buf[ENTRY_NAME_BUF_SIZE] = {'\0'};
    struct sigaction timer_sig = {0};

    timer_sig.sa_handler = &force_runx;
    sigaction(SIGUSR1, &timer_sig, NULL);

    while (READ_USR_ENTRY) {
        read_user_entry(read_buf, sizeof(read_buf));

        if (read_buf[0] == '\n') {
            return start_x(entry_table_buf[default_entry - 1]);
        }

        clean_nline();
        distillstr(read_buf);

        if (is_valid_entrynum(read_buf)) {
            entryid selected_entry = strtoul(read_buf, NULL, 10);
            if (selected_entry == 0) {
                return 0;
            } else if (selected_entry > entry_count) {
                printw_indent(1, false, ENTRY_PROMPT, "Invalid entry number");
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
                printw_indent(1, false, ENTRY_PROMPT, "Invalid entry name");
                break;
            case 1:
                return start_x(entry_table_buf[match_id]);
            default:
                printw_indent(1, false, ENTRY_PROMPT, "Disambiguous between multiple entry names");
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

static int nprompt_number() {
    clean_nline();
    if (prompt_timeout > 0) {
        printw_indent(1, false, ENTRY_PROMPT_DEFAULT, prompt_timeout);
        refresh();
        timer_pid = fork();
    } else { 
        printw_indent(1, false, ENTRY_PROMPT, "no timeout");
        refresh();
    }

    if (timer_pid == 0) {
        run_prompt_timer();
    } else {
        return handle_entrynum();
    }
    return 0;
}

static void print_entry_menu(struct sorted_entries *sentries) {
    struct dirent *centry = NULL;

    printw_indent_next_line("Choose an entry (default: %lu):", default_entry);
	
    for (entryid eid = 1; (centry = iter_entry(sentries)); eid++) {
        strncpy(entry_table_buf[eid - 1], centry->d_name, ENTRY_NAME_BUF_SIZE);
        entry_table_buf[eid - 1][ENTRY_NAME_BUF_SIZE - 1] = '\0';
        printw_entry(centry->d_name, eid);
        free(centry);
    }

    destroy_dentries_iterator(sentries);

	NEW_LINE();
    printw_indent_next_line("(0) Exit\n");
	NEW_LINE();
	
    refresh();
}

static int alloc_entry_buf(void) {
    bufval = NULL;

    entry_table_buf = (char **)calloc(entry_count, sizeof(*entry_table_buf));
    if (!entry_table_buf)
        return 1;

    bufval = calloc(ENTRY_NAME_BUF_SIZE * entry_count, sizeof(**entry_table_buf));
    if (!bufval) {
        free(entry_table_buf);
        entry_table_buf = NULL;
        return 1;
    }

    for (entryid i = 0; i < entry_count; i++)
        entry_table_buf[i] = bufval + (ENTRY_NAME_BUF_SIZE * i);
    
    return 0;
}

void setup_screen(void) {
    win = initscr();
    win->_scroll = true;
	cbreak();
	box(win, 0, 0);
}

int nprompt(const char *entry_name) {
    struct sorted_entries sentries = {0};
    int res = 1;

    if (!entry_invalid(entry_name)) {
        return start_x(entry_name);
    }

    getdir_entries(&sentries);
    entry_count = sentries.entrycnt;

    if (alloc_entry_buf()) 
        return res;

    setup_screen();
	printw_indent_next_line("default: %d", default_entry);

    print_entry_menu(&sentries);

    res = nprompt_number();
    
    killtimer();
    ncleanup();

    return res;
}
