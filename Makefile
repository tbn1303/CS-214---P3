CC = gcc
CFLAGS = -g -Wall -Wvla -std=c99 -fsanitize=address,undefined

all: mysh

mysh: mysh.c parser.c executor.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $<

mysh.c parser.c executor.c: mysh.h

clean:
	rm -f *.o mysh
