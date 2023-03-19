CC = gcc
CFLAGS = -Wall -std=c11 -ggdb

# O1

static_O1:
	$(CC) $(CFLAGS) -c ../zad1/wc.c -O1
	ar rc wc.a wc.o
	$(CC) $(CFLAGS) -c ../zad2/repl.c -O1
	$(CC) repl.o wc.a -o repl

shared_O1:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O1
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -L. -lwc -o repl -O1
	# Run with LD_LIBRARY_PATH=. ./repl

dynamic_O1:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O1
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -ldl -D DYNAMIC -o repl -O1
	# Run with LD_LIBRARY_PATH=. ./repl

test_static_O1:
	make static_O1
	./repl < test.txt > results_static_O1.txt
	make clean

test_shared_O1:
	make shared_O1
	./repl < test.txt > results_shared_O1.txt
	make clean

test_dynamic_O1:
	make dynamic_O1
	./repl < test.txt > results_dynamic_O1.txt
	make clean

# O2

static_O2:
	$(CC) $(CFLAGS) -c ../zad1/wc.c -O2
	ar rc wc.a wc.o
	$(CC) $(CFLAGS) -c ../zad2/repl.c -O2
	$(CC) repl.o wc.a -o repl

shared_O2:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O2
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -L. -lwc -o repl -O2
	# Run with LD_LIBRARY_PATH=. ./repl

dynamic_O2:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O2
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -ldl -D DYNAMIC -o repl -O2
	# Run with LD_LIBRARY_PATH=. ./repl

test_static_O2:
	make static_O2
	./repl < test.txt > results_static_O2.txt
	make clean

test_shared_O2:
	make shared_O2
	./repl < test.txt > results_shared_O2.txt
	make clean

test_dynamic_O2:
	make dynamic_O2
	./repl < test.txt > results_dynamic_O2.txt
	make clean

# O3

static_O3:
	$(CC) $(CFLAGS) -c ../zad1/wc.c -O3
	ar rc wc.a wc.o
	$(CC) $(CFLAGS) -c ../zad2/repl.c -O3
	$(CC) repl.o wc.a -o repl

shared_O3:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O3
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -L. -lwc -o repl -O3
	# Run with LD_LIBRARY_PATH=. ./repl

dynamic_O3:
	$(CC) $(CFLAGS) -fPIC -c ../zad1/wc.c -O3
	$(CC) -shared -o libwc.so wc.o
	$(CC) $(CFLAGS) ../zad2/repl.c -ldl -D DYNAMIC -o repl -O3
	# Run with LD_LIBRARY_PATH=. ./repl

test_static_O3:
	make static_O3
	./repl < test.txt > results_static_O3.txt
	make clean

test_shared_O3:
	make shared_O3
	./repl < test.txt > results_shared_O3.txt
	make clean

test_dynamic_O3:
	make dynamic_O3
	./repl < test.txt > results_dynamic_O3.txt
	make clean

run_all_tests:
	make test_static_O1
	make test_shared_O1
	make test_dynamic_O1
	make test_static_O2
	make test_shared_O2
	make test_dynamic_O2
	make test_static_O3
	make test_shared_O3
	make test_dynamic_O3

clean:
	rm -rf *.so *.gch *.o *.a /tmp/wc.txt repl.dSYM repl
