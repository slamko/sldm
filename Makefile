CC=gcc
LCFLAGS=-Wall -Werror -g
CFLAGS=$(LCFLAGS) -c
NAME=sldm
OBJS=sldm.o main.o log-utils.o config-names.o entry-prompt.o

MAIN_H=src/config-names.h src/log-utils.h
SLDM_H=src/main.h $(MAIN_H)

all: $(OBJS)
	$(CC) $(LCFLAGS) $(OBJS) -o $(NAME)

sldm.o: src/sldm.c $(SLDM_H)
	$(CC) $(CFLAGS) src/sldm.c

main.o: src/main.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) src/main.c

entry-prompt.o: src/entry-prompt.c src/command-names.h  $(MAIN_H)
	$(CC) $(CFLAGS) src/entry-prompt.c

log-utils.o: src/log-utils.c src/config-names.h
	$(CC) $(CFLAGS) src/log-utils.c

config-names.o: src/config-names.c
	$(CC) $(CFLAGS) src/config-names.c

clean: 
	rm -rf $(OBJS) $(NAME)