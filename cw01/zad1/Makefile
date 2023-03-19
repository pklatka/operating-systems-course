CC = gcc
CFLAGS = -Wall -std=c11 -ggdb

static:
	$(CC) $(CFLAGS) -c wc.c
	ar rc wc.a wc.o

shared:
	$(CC) $(CFLAGS) -fPIC -c wc.c
	$(CC) -shared -fPIC -o libwc.so wc.o

clean:
	rm -rf *.so *.gch *.o *.a /tmp/wc.txt repl.dSYM repl
