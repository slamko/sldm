#define XINITRC_ETC "/etc/X11/xinit/xinitrc"
#define XINITRC_ETC_D "/etc/X11/xinit/xinitrc.d"
#define SLDM_CONFIG "/.config/sldm/"
#define SLDM_CONFIG_ENTRIES "/.config/sldm/entries/"

// Default config values
#define XINITRC_L "~/.xinitrc"
#define _DEFAULT_ENTRY 1
#define _PROMPT_TIMEOUT 10

extern int prompt_timeout;
extern int default_entry;
extern char *base_xconfig;

char *concat(const char *base, const char *appends);

char *home_path_append(const char *append);

char *sldm_config_append(char *appends);

char *get_home(void);

char *get_xconfig(void);

char *get_sldm_config_dir(void);

void cleanup_names(void);