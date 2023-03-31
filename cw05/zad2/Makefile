CC = gcc
CFLAGS = -Wall 
FILE = ./main

# ./main 0.0000000001 10000


compile:
	$(CC) $(CFLAGS) -c  $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)
	
tests:
	rm -rf raport.txt
	make compile
	./main 0.0000000001 1000
	./main 0.0000001 1000
	./main 0.0001 100

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE)
