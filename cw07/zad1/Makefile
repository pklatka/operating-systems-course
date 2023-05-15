CC = gcc
CFLAGS = -Wall 
FILE = ./main

compile:
	$(CC) $(CFLAGS) -c  $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)
	
all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE)
