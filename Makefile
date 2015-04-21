#*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#* File: Makefile
#*
#* Author: Tony Allen (cyril0allen@gmail.com)
#**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
CC = clang++ -Wall -std=gnu++11

all:
	$(CC) main.cc -o btree

debug:
	$(CC) -O0 -v -g main.cc -o btree

clean:
	rm -rf *.o btree
