#include "command-names.h"

extern char *add_entry_command;

int add_entry(char *entry_name);

int remove_entry(char *entry_name);

int list_entries(void);

int prompt(char *entry_name);