CC=gcc
LCFLAGS=-Wall -Werror -O2
CFLAGS=$(LCFLAGS) -c -o
LIBS=-lncurses
TARGET=sldm
BDIR=build
SRCD=src
OBJS=$(addprefix $(BDIR)/, sldm.o main.o nentry-prompt.o config-names.o log-utils.o)
HEADERS=$(wildcard $(SRCD)/*.h)

BINP=/usr/local/bin
BIN=$(BINP)/sldm

vpath %.h $(SRCD)
vpath %.c $(SRCD)

.PHONY: all debug install clean uninstall

all: $(TARGET)

$(TARGET): $(OBJS) config.h
	$(CC) $(LCFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

debug: LCFLAGS=-Wall -Werror -Wextra -O0 -g
debug: clean
debug: $(TARGET)

$(BDIR)/%.o: $(SRCD)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $@ $<

install: $(TARGET)
	mkdir -p $(BINP)
	cp -f ./sldm $(BIN)
	chmod 755 $(BIN)

clean: 
	rm -rf $(OBJS) $(TARGET)

uninstall: 
	rm -f $(BIN)

