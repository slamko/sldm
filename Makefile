CC=gcc
LCFLAGS=-Wall -Werror -g
CFLAGS=$(LCFLAGS) -c
NAME=sldm
OBJS=sldm.o main.o log-utils.o xconfig-names.o

all: $(OBJS)
	$(CC) $(LCFLAGS) $(OBJS) -o $(NAME)

sldm.o: src/sldm.c src/main.h src/xconfig-names.h src/log-utils.h
	$(CC) $(CFLAGS) src/sldm.c

main.o: src/main.c src/command-names.h src/xconfig-names.h src/log-utils.h
	$(CC) $(CFLAGS) src/main.c

log-utils.o: src/log-utils.c src/xconfig-names.h
	$(CC) $(CFLAGS) src/log-utils.c

xconfig-names.o: src/xconfig-names.c
	$(CC) $(CFLAGS) src/xconfig-names.c

clean: 
	rm -rf $(OBJS) $(NAME)