#include <iostream>
#include <memory>
#include "btree.h"

#include <stdlib.h>
#include <time.h>
#include <unordered_set>

using namespace std;

int main(){
  Btree<size_t> bt(2);

  const int num = 200000;
  std::unordered_set<size_t> vals;
  srand(time(NULL));

  // Create a set of unique values
  for (int i=0; i<num; i++) {
    vals.emplace(rand() % 1000000000);
  }
  for (auto i = vals.begin(); i != vals.end(); i++) {
    std::cout << "Inserting " << *i << std::endl;
    bt.insert(*i);
  }

  for (auto i = vals.begin(); i != vals.end(); i++) {
    std::cout << "Searching " << *i << std::endl;
    bt.search(*i);
  }

  for (auto i = vals.begin(); i != vals.end(); i++) {
    std::cout << "Removing " << *i << std::endl;
    bt.remove(*i);
  }

  cout << "This is a test file.\n" << "Size of newly created b-tree is "
       << bt.get_size() << "." << endl;
  return 0;
}
