#ifndef COMMAND_NAMES
#define COMMAND_NAMES

#define ADD_ENTRY_ARG "add-entry"
#define REMOVE_ENTRY_ARG "remove-entry"
#define LIST_ENTRIES_ARG "list-entries"
#define SHOW_ENTRY_ARG "show-entry"

#define TTY_DEVICE "/dev/tty"
#define TTY_DEVICE_NAME_BYTES (sizeof(TTY_DEVICE) - 1)
#define TTY_MIN_NAME "/dev/pts/"
#define TTY_MIN_NAME_LEN sizeof(TTY_MIN_NAME)
#define STARTX "/bin/startx"

#define EXEC_C "\nexec"
#define EXEC_C_LENGTH (sizeof(EXEC_C) - 1)
#define ENTRY_NAME_BUF_SIZE 256

#endif

