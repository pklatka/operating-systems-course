CC = gcc
CFLAGS = -Wall
FILE1 = ./fork_inheritance
FILE2 = ./exec_inheritance
FILE3 = ./exec_inheritance_child

compile_fork:
	$(CC) $(CFLAGS) -c  $(FILE1).c 
	$(CC) $(CFLAGS) -o $(FILE1) $(FILE1).o

compile_exec:
	$(CC) $(CFLAGS) -c  $(FILE2).c 
	$(CC) $(CFLAGS) -o $(FILE2) $(FILE2).o
	$(CC) $(CFLAGS) -c  $(FILE3).c 
	$(CC) $(CFLAGS) -o $(FILE3) $(FILE3).o

compile_exec_child:
	$(CC) $(CFLAGS) -c  $(FILE3).c 
	$(CC) $(CFLAGS) -o $(FILE3) $(FILE3).o


run_fork:
	make compile_fork
	$(FILE1) "ignore"
	$(FILE1) "handler"
	$(FILE1) "mask"
	$(FILE1) "pending"


run_exec:
	make compile_exec_child
	make compile_exec
	$(FILE2) "ignore"
	$(FILE2) "mask"
	$(FILE2) "pending"

all:
	make run_fork
	make run_exec
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE1) $(FILE2) $(FILE3) 
