#ifndef CONFIG_NAMES
#define CONFIG_NAMES

#define XINITRC_ETC "/etc/X11/xinit/xinitrc"
#define XINITRC_ETC_D "/etc/X11/xinit/xinitrc.d"
#define DEF_CONFIG_D "/.config/"
#define SLDM_CONFIG_D "/sldm/"
#define SLDM_CONFIG_ENTRIES "/sldm/entries/"

// Default config values
#define XINITRC_L "~/.xinitrc"
#define _DEFAULT_ENTRY 1
#define _PROMPT_TIMEOUT -1

#include "nentry-prompt.h"

extern int prompt_timeout;
extern entryid default_entry;
extern char *base_xconfig;

char *sappend(const char *base, const char *appends);

char *home_path_append(const char *append);

char *sldm_config_append(const char *appends);

char *get_home(void);

char *get_xconfig(void);

char *get_sldm_config_entries(void);

void cleanup_names(void);

#endif
