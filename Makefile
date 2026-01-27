CC = gcc
CFLAGS = -Wall -Wextra -Werror

mt:mt.o
	$(CC) $(CFLAGS) mt.o -o mt
mt.o:mt.c
	$(CC) $(CFLAGS) -c mt.c -o mt.o

.PHONY: clean
clean:
	rm -f mt.o mt
