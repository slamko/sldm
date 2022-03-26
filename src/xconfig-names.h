#define TTY_DEVICE "/dev/tty"
#define TTY_DEVICE_NAME_BYTES 8
#define TTY_BIN "/bin/tty"
#define XINITRC_L "/.xinitrc"
#define XINITRC_ETC "/etc/X11/xinit/xinitrc"
#define XINITRC_ETC_D "/etc/X11/xinit/xinitrc.d"
#define SLDM_CONFIG "/.config/sldm"

char *home_path_append(const char *append);

char *get_home(void);

char *get_xinitrc(viod);

void cleanup_names(void);