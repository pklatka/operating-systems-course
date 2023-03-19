CC = gcc
CFLAGS = -Wall -std=c11
FILE = ./fwt
TEST_DIR = .

compile:
	$(CC) $(CFLAGS) -c  $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)

compare:
	$(FILE) $(TEST_DIR)
	find $(TEST_DIR) | xargs wc --bytes

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./result2.txt
