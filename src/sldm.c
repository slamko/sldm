//#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CONFIG_HOME "$HOME/.config/sldm" 


void fail(char *err) {
    printf("error: %s", err);
}

void fail_not_in_tty() {
    fail("Only console users are allowed to run the X server");
}

int check_if_in_tty() {
    FILE* fp;
    char tty_output[256];

    fp = popen("/bin/tty", "r");
    if (fp == NULL) {
        return 1;
    }

    if (fgets(tty_output, sizeof(tty_output), fp) != 0) {
        char tty[9];
        strncpy(tty, tty_output, 8);
        tty[9] = '\0';
        char *tty_exp = "/dev/tty";
        return strcmp(tty, tty_exp);
    }
    return 1;
} 

char *get_base_x_config() {
    if(access("$HOME/.xinirc", R_OK)) {
        return "$HOME/.xinirc";
    }
}

int main(int args, char** argv) {
    if(check_if_in_tty()) {
        fail_not_in_tty();
        return 1;
    }


    return 0;
}