//#define _POSIX_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include "xconfig-names.h"
#include "log-utils.h"

char *home = NULL;

char *get_home(void) {
    if (!home)
        home = getenv("HOME");
    return home;
}

char *concat(const char *base, const char *append)
{
    const size_t len1 = strlen(base);
    const size_t len2 = strlen(append);
    char *result = malloc(len1 + len2 + 1);

    if (!result) 
        die("Out of memory");
    
    memcpy(result, base, len1);
    memcpy(result + len1, append, len2 + 1);
    return result;
}

char *home_append(const char *append) {
    const char *home = get_home();
    return concat(home, append);
}

char *xinitrc = NULL;

char *get_xinitrc_l(void) {
    if (!xinitrc) 
        xinitrc = home_append(XINITRC_L);

    return xinitrc;
}

char *sldm_config = NULL;

char *get_sldm_config_dir(void) {
    if (!sldm_config) 
        sldm_config = home_append(SLDM_CONFIG);

    return sldm_config;
}

char *get_xconfig(void) {
    return get_xinitrc_l();
}

void cleanup_names(void) {
    if (xinitrc)
        free(xinitrc);
    
    if (sldm_config)
        free(sldm_config);
}
