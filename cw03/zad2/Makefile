CC = gcc
CFLAGS = -Wall -std=c11
FILE = ./main

compile:
	$(CC) $(CFLAGS) -c  $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)

test1:
	make compile
	./main ./no_files

test2:
	make compile
	./main ./some_files

test3:
	make compile
	./main ./lots_of_files

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./result2.txt
