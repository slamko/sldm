CC=gcc
LCFLAGS=-Wall -Werror -O2
CFLAGS=$(LCFLAGS) -c -o
LIBS=-lncurses
NAME=sldm
SLDM=build/sldm.o
MAIN=build/main.o
ENTRY_PROMPT=build/nentry-prompt.o
LOG=build/log-utils.o
NAMES=build/config-names.o
BINP=/usr/local/bin
BIN=$(BINP)/sldm
OBJS=$(SLDM) $(MAIN) $(LOG) $(ENTRY_PROMPT) $(NAMES)

vpath %.h src
vpath %.c src

MAIN_H=config-names.h log-utils.h
SLDM_H=main.h $(MAIN_H)

$(NAME): $(OBJS) config.h
	$(CC) $(LCFLAGS) $(OBJS) -o $@ $(LIBS)

debug: LCFLAGS=-Wall -Werror -Wextra -O0 -g
debug: clean
debug: sldm

$(SLDM): sldm.c $(SLDM_H) config.h
	$(CC) $(CFLAGS) $@ $<

$(MAIN): main.c command-names.h $(MAIN_H)
	$(CC) $(CFLAGS) $@ $<

$(ENTRY_PROMPT): nentry-prompt.c command-names.h $(MAIN_H)
	$(CC) $(CFLAGS) $@ $<

$(LOG): log-utils.c config-names.h
	$(CC) $(CFLAGS) $@ $<

$(NAMES): config-names.c
	$(CC) $(CFLAGS) $@ $^

install: sldm
	mkdir -p $(BINP)
	cp -f ./sldm $(BIN)
	chmod 755 $(BIN)

clean: 
	rm -rf $(OBJS) $(NAME)

uninstall: 
	rm -f $(BIN)

