CFLAGS=-Ilibmembuf
SOURCES=$(wildcard *.c) libmembuf/lib.c

main: $(SOURCES) 
	$(CC) $(SOURCES) $(CFLAGS) -o main

debug: $(SOURCES)
	$(CC) $(SOURCES) $(CFLAGS) -g -o debug
