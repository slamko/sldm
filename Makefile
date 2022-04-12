CC=gcc
LCFLAGS=-Wall -Werror
CFLAGS=$(LCFLAGS) -g -c -o
NAME=sldm
SLDM=build/sldm.o
MAIN=build/main.o
ENTRY_PROMPT=build/nentry-prompt.o
LOG=build/log-utils.o
NAMES=build/config-names.o
BINP=/usr/local/bin/sldm
OBJS=$(SLDM) $(MAIN) $(LOG) $(ENTRY_PROMPT) $(NAMES)

MAIN_H=src/config-names.h src/log-utils.h
SLDM_H=src/main.h $(MAIN_H)

all: $(OBJS)
	$(CC) $(LCFLAGS) $(OBJS) -o $(NAME)  -lncurses

$(SLDM): src/sldm.c $(SLDM_H)
	$(CC) $(CFLAGS) $(SLDM) src/sldm.c

$(MAIN): src/main.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) $(MAIN) src/main.c

$(ENTRY_PROMPT): src/nentry-prompt.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) $(ENTRY_PROMPT) src/nentry-prompt.c

$(LOG): src/log-utils.c src/config-names.h
	$(CC) $(CFLAGS) $(LOG) src/log-utils.c

$(NAMES): src/config-names.c
	$(CC) $(CFLAGS) $(NAMES) src/config-names.c

install: all
	cp ./sldm $(BINP)

clean: 
	rm -rf $(OBJS) $(NAME)

uninstall: 
	rm -rf $(BINP)
