#include <iostream>
#include <memory>
#include "btree.h"

using namespace std;

int main(){
  Btree<size_t> bt(16*1024);

  int num = 100000;

  for (int i=0; i<num; i++) {
    std::cout << "inserting " << i << " into tree.\n";
    bt.insert(i);
  }
  for (int i=0; i<num; i++) {
    std::cout << "searching for " << i << " found " << bt.search(i) << std::endl;
  }

  cout << "This is a test file.\n" << "Size of newly created b-tree is "
       << bt.get_size() << "." << endl;
  return 0;
}
