#*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
#* File: Makefile
#*
#* Author: Tony Allen (cyril0allen@gmail.com)
#**=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
CC = clang++ -Wall -std=gnu++11

debug:
	$(CC) -O0 -v -g main.cc -o btree

release:
	$(CC) main.cc -O3 -DNDEBUG -o btree

clean:
	rm -rf *.o btree
