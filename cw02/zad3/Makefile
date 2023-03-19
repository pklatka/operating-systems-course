CC = gcc
CFLAGS = -Wall -std=c11
FILE = ./wcbytes

compile:
	$(CC) $(CFLAGS) -c  $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)

compare:
	$(FILE)
	wc --bytes *

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./result2.txt
