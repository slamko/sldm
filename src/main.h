#define ADD_ENTRY_ARG "add-entry"
#define REMOVE_ENTRY_ARG "add-entry"

extern char *add_entry_command;

int add_entry(char *entry_name);

int remove_entry(char *entry_name);

int prompt(char *entry_name);