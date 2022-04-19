CC=gcc
LCFLAGS=-Wall -Werror -O2
CFLAGS=$(LCFLAGS) -c -o
LIBS=-lncurses
NAME=sldm
BDIR=build

SLDM=$(BDIR)/sldm.o
MAIN=$(BDIR)/main.o
ENTRY_PROMPT=$(BDIR)/nentry-prompt.o
LOG=$(BDIR)/log-utils.o
NAMES=$(BDIR)/config-names.o
OBJS=$(SLDM) $(MAIN) $(LOG) $(ENTRY_PROMPT) $(NAMES)

BINP=/usr/local/bin
BIN=$(BINP)/sldm

vpath %.h src
vpath %.c src

.PHONY: debug install clean uninstall

MAIN_H=config-names.h log-utils.h
SLDM_H=main.h $(MAIN_H)

$(NAME): $(OBJS) config.h
	$(CC) $(LCFLAGS) $(OBJS) -o $@ $(LIBS)

debug: LCFLAGS=-Wall -Werror -Wextra -O0 -g
debug: clean
debug: $(NAME)

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

