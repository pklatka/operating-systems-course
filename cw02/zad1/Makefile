CC = gcc
CFLAGS = -Wall -std=c11
FILE = ./charswapper
TEST_FILE = ./test.txt

# Use functions from standard library
compile:
	$(CC) $(CFLAGS) -c  -D USE_FILE_SYSTEM_FUNCTIONS=1 $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

# Use system functions
compile2:
	$(CC) $(CFLAGS) -D USE_FILE_SYSTEM_FUNCTIONS=0 -c $(FILE).c 
	$(CC) $(CFLAGS) -o $(FILE) $(FILE).o

run:
	make compile
	$(FILE)

test_times:
	make compile
	$(FILE) a b $(TEST_FILE) ./result.txt > pomiar_zad_1.txt
	make clean
	make compile2
	$(FILE) a b $(TEST_FILE) ./result.txt >> pomiar_zad_1.txt
	make clean

compare:
	$(FILE) a b $(TEST_FILE) ./result.txt
	tr a b < $(TEST_FILE)  >  ./tr-result.txt
	diff -s ./result.txt ./tr-result.txt

all:
	make run
	make clean

clean:
	rm -rf _.* *.so *.gch *.o *.a $(FILE) ./result.txt ./tr-result.txt
