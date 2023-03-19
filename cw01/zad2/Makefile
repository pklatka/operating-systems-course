CC = gcc
CFLAGS = -Wall -std=c11 -ggdb
LIB_DIR = ../zad1

static:
	$(CC) $(CFLAGS) -c $(LIB_DIR)/wc.c
	ar rc wc.a wc.o
	$(CC) $(CFLAGS) -c repl.c
	$(CC) repl.o wc.a -o repl

shared:
	$(CC) $(CFLAGS) -fPIC -c $(LIB_DIR)/wc.c
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) repl.c -L. -lwc -o repl
	# Run with LD_LIBRARY_PATH=. ./repl

dynamic:
	$(CC) $(CFLAGS) -fPIC -c $(LIB_DIR)/wc.c
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) repl.c -ldl -D DYNAMIC -o repl
	# Run with LD_LIBRARY_PATH=. ./repl

run:
	./repl

clean:
	rm -rf *.so *.gch *.o *.a /tmp/wc.txt repl.dSYM repl
