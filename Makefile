CC=gcc
LCFLAGS=-Wall -Werror -g
CFLAGS=$(LCFLAGS) -c -o 
NAME=sldm
SLDM=build/sldm.o
MAIN=build/main.o
ENTRY_PROMPT=build/entry-prompt.o
LOG=build/log-utils.o
NAMES=build/config-names.o
OBJS=$(SLDM) $(MAIN) $(LOG) $(ENTRY_PROMPT) $(NAMES)

MAIN_H=src/config-names.h src/log-utils.h
SLDM_H=src/main.h $(MAIN_H)

all: $(OBJS)
	$(CC) $(LCFLAGS) $(OBJS) -o $(NAME)

$(SLDM): src/sldm.c $(SLDM_H)
	$(CC) $(CFLAGS) $(SLDM) src/sldm.c

$(MAIN): src/main.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) $(MAIN) src/main.c

$(ENTRY_PROMPT): src/entry-prompt.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) $(ENTRY_PROMPT) src/entry-prompt.c

$(LOG): src/log-utils.c src/config-names.h
	$(CC) $(CFLAGS) $(LOG) src/log-utils.c

$(NAMES): src/config-names.c
	$(CC) $(CFLAGS) $(NAMES) src/config-names.c

clean: 
	rm -rf $(OBJS) $(NAME)