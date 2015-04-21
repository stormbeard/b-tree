all:
	clang++ -Wall -std=gnu++11 main.cc btree.cc -o btree

clean:
	rm -rf *.o btree
