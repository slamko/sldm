#define ADD_ENTRY_ARG "add-entry"
#define REMOVE_ENTRY_ARG "add-entry"
#define EXEC_C "\nexec"
#define EXEC_C_LENGTH 5

extern char *add_entry_command;

int add_entry(char *entry_name);

int remove_entry(char *entry_name);

int prompt(char *entry_name);