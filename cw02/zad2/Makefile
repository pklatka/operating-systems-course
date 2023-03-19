CC = gcc
CFLAGS = -Wall -std=c11
FILE = ./filereverser
TEST_FILE = ./test.txt

compile:
	$(CC) $(CFLAGS) -c  -D READ_CHAR_BY_CHAR=1 $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

compile2:
	$(CC) $(CFLAGS) -D READ_CHAR_BY_CHAR=0 -c $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)

test_times:
	make compile
	$(FILE) $(TEST_FILE) ./result.txt > pomiar_zad_2.txt
	make clean
	make compile2
	$(FILE) $(TEST_FILE) ./result.txt >> pomiar_zad_2.txt
	make clean

compare:
	$(FILE) $(TEST_FILE) ./result.txt
	$(FILE) ./result.txt ./result2.txt
	diff -s $(TEST_FILE) ./result2.txt

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./result2.txt
