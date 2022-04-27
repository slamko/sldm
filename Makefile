CC=gcc
WFLAGS=-Wall -Werror -Wextra
LCFLAGS=$(WFLAGS) -O2
CFLAGS=$(LCFLAGS) -c -o
LIBS=-lncurses
TARGET=sldm
BDIR=build
SRCD=src

SRC:=$(wildcard $(SRCD)/*.c)
OBJS:=$(SRC:$(SRCD)%.c=$(BDIR)%.o)
HEADERS:=$(wildcard $(SRCD)/*.h)

BINP=/usr/local/bin
BIN=$(BINP)/sldm

vpath %.h $(SRCD)
vpath %.c $(SRCD)

.PHONY: all debug install clean uninstall

all: $(TARGET)

$(TARGET): $(BDIR) $(OBJS) config.h
	$(CC) $(LCFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

debug: LCFLAGS=$(WFLAGS) -O0 -g
debug: clean
debug: $(TARGET)

$(BDIR):
	mkdir -p $(BDIR)

$(OBJS): build/%.o : $(SRCD)/%.c $(HEADERS)
	$(CC) $(CFLAGS) $@ $<

install: $(TARGET)
	mkdir -p $(BINP)
	cp -f ./$(TARGET) $(BIN)
	chmod 755 $(BIN)

clean: 
	$(RM) -r $(OBJS) $(TARGET)

uninstall: 
	$(RM) $(BIN)

