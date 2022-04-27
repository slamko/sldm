#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "config-names.h"
#include "log-utils.h"

char *home = NULL;

char *get_home(void) {
    if (!home)
        home = getenv("HOME");        
    
    return home;
}

char *sappend(const char *base, const char *appends) {
    const size_t len1 = strlen(base);
    const size_t len2 = strlen(appends);
    char *result = calloc(len1 + len2 + 1, sizeof(*result));

    if (!result) 
        fatal();

    strncpy(result, base, len1 + 1);
    strncat(result + len1, appends, len2 + 1);
    return result;
}

char *slash_append(const char *base, const char *appends) {
    const size_t len1 = strlen(base);
    const size_t len2 = strlen(appends);
    char *result = calloc(len1 + len2 + 2, sizeof(*result));

    if (!result) 
        fatal();
    
    strncpy(result, base, len1 + 1);
    result[len1] = '/';
    strncat(result + len1 + 1, appends, len2 + 1);
    return result;
}

char *home_path_append(const char *appends) {
    const char *home = get_home();
    return sappend(home, appends);
}

char *xinitrc = NULL;

char *get_xinitrc_l(void) {
    if (!xinitrc) 
        xinitrc = home_path_append(XINITRC_L);

    return xinitrc;
}

char *sldm_config_dir = NULL;

char *get_sldm_config_dir(void) {
    if (!sldm_config_dir) 
        sldm_config_dir = home_path_append(SLDM_CONFIG_ENTRIES);

    struct stat sb;
    if (stat(sldm_config_dir, &sb) && !S_ISDIR(sb.st_mode)) 
        mkdir(sldm_config_dir, S_IRWXU);

    return sldm_config_dir;
}

char *sldm_config_append(const char *appends) {
    return sappend(get_sldm_config_dir(), appends);
}

char *get_xconfig(void) {
    return base_xconfig;
}

void cleanup_names(void) {
    if (xinitrc)
        free(xinitrc);
    
    if (sldm_config_dir)
        free(sldm_config_dir);
}
