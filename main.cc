#include <iostream>
#include <memory>
#include "btree.h"

using namespace std;

int main(){
  std::unique_ptr<Btree<unsigned long>> bt(new Btree<unsigned long>(2));
  cout << "This is a test file.\n" << "Size of newly created b-tree is "
       << bt->get_size() << "." << endl;
  return 0;
}
