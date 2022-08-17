#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include "config-names.h"
#include "log-utils.h"

static char *home;
static char *xinitrc;
static char *xprofile;
static char *config_dir;
static char *sldm_config_dir;
static char *sldm_config_entries;

char *get_home(void) {
    if (!home)
        home = getenv(HOME);

    if (!home)
        home = getenv(XDG_HOME);

    if (!home)
       die("Can not find home directory");

    return home;
}

char *home_path_append(const char *appends) {
    if (!appends)
        return NULL;

    const char *home = get_home();
    return sappend(home, appends);
}

char *get_config_dir(void) {
    if (!config_dir)
        config_dir = home_path_append(getenv(XDG_CONFIG_HOME));

    if (!config_dir)
        config_dir = home_path_append(DEF_CONFIG_D);

   return config_dir;
}

char *config_dir_append(const char *appends) {
    if (!appends)
        return NULL;

    const char *config = get_config_dir();
    return sappend(config, appends);
}

char *sappend(const char *base, const char *appends) {
    if (!appends || !base)
        return NULL;

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
    if (!appends || !base)
        return NULL;

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

char *get_xinitrc_l(void) {
    if (!xinitrc) 
        xinitrc = home_path_append(XINITRC_L);

    return xinitrc;
}

char *get_xprofile(void) {
    if (!xprofile) 
        xprofile = home_path_append(XPROFILE_L);

    return xprofile;
}

char *get_sldm_config_dir(void) {
    if (!sldm_config_dir) {
        struct stat sb;

        sldm_config_dir = config_dir_append(SLDM_CONFIG_D);

        if (stat(sldm_config_dir, &sb) && !S_ISDIR(sb.st_mode)) {
            mkdir(get_config_dir(), S_IRWXU);
            mkdir(sldm_config_dir, S_IRWXU);
        }
    }

    return sldm_config_dir;
}

char *get_sldm_config_entries(void) {
    if (!sldm_config_entries) {
        sldm_config_entries = sappend(get_sldm_config_dir(), SLDM_CONFIG_ENTRIES);

        struct stat sb;
        if (stat(sldm_config_entries, &sb) && !S_ISDIR(sb.st_mode)) {
            mkdir(sldm_config_entries, S_IRWXU);
        }
    }

    return sldm_config_entries;
}

char *sldm_config_append(const char *appends) {
    if (!appends)
        return NULL;

    return sappend(get_sldm_config_entries(), appends);
}

char *get_xconfig(void) {
    return base_xconfig;
}

void cleanup_names(void) {
    free(xinitrc);
    free(config_dir);
    free(sldm_config_dir);
    free(sldm_config_entries);
}
