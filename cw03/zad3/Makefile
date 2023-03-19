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
	$(FILE) . \#include
	$(FILE) . asdf
	$(FILE) . zadanie

test2:
	$(FILE) .. \#include
	$(FILE) .. asdf
	$(FILE) .. zadanie

test3:
	$(FILE) ../.. \#include
	$(FILE) ../.. asdf
	$(FILE) ../.. zadanie

test4:
	$(FILE) ~/ \#include
	$(FILE) ~/ asdf
	$(FILE) ~/ zadanie

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./result2.txt
