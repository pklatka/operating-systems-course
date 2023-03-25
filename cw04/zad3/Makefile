CC = gcc
CFLAGS = -Wall
FILE1 = ./sender
FILE2 = ./catcher

compile_sender:
	$(CC) $(CFLAGS) -c  $(FILE1).c 
	$(CC) $(CFLAGS) -o $(FILE1) $(FILE1).o

compile_catcher:
	$(CC) $(CFLAGS) -c  $(FILE2).c 
	$(CC) $(CFLAGS) -o $(FILE2) $(FILE2).o

run_sender:
	make compile_sender
	$(FILE1)

run_catcher:
	make compile_catcher
	$(FILE2)

all:
	make run_sender
	make run_catcher
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE1) $(FILE2)
