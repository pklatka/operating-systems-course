CC = gcc
CFLAGS = -Wall -std=c11 -ggdb

static:
	$(CC) $(CFLAGS) -c ../zad1/wc.c
	ar rc wc.a wc.o
	$(CC) $(CFLAGS) -c ../zad2/repl.c
	$(CC) repl.o wc.a -o repl

shared:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -L. -lwc -o repl
	# Run with LD_LIBRARY_PATH=. ./repl

dynamic:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -ldl -D DYNAMIC -o repl
	# Run with LD_LIBRARY_PATH=. ./repl

test_static:
	make static
	./repl < test.txt > results_static.txt
	make clean

test_shared:
	make shared
	./repl < test.txt > results_shared.txt
	make clean

test_dynamic:
	make dynamic
	./repl < test.txt > results_dynamic.txt
	make clean

run_all_tests:
	make test_static
	make test_shared
	make test_dynamic

clean:
	rm -rf *.so *.gch *.o *.a /tmp/wc.txt repl.dSYM repl

