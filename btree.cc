/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 * File: btree.cc
 *
 * Author: Tony Allen (cyril0allen@gmail.com)
 *
 * Purpose: Implementation of the Btree class.
 **=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#include "btree.h"

// -----------------------------------------------------------------------------
// Btree definitions
// -----------------------------------------------------------------------------

// Btree constructor:
// Creates an empty B-tree via creating an empty node and makes root point
// to this empty node.
template <class T>
Btree<T>::Btree(size_t _min_degree) {
  // Set min degree.
  degree = _min_degree;

  // There should be 0 keys in this newly created B-tree.
  num_keys = 0;

  // Create a node that is a leaf and root.
  root = new Node(degree, true, true);

  // TODO: write to disk?
}

// get_size:
// The number of keys contained in the tree.
template <class T>
size_t Btree<T>::get_size() {
  return num_keys;
}

// is_sane:
// Perform sanity check on the tree.
template <class T>
bool Btree<T>::is_sane() {
  // TODO
  assert(false);
}

// search:
// Search for a key with value k
template <class T>
std::pair <std::vector<T>, size_t> Btree<T>::search(T key) {
  // TODO
  assert(false);
}

// insert:
// Insert a key into the tree.
template <class T>
void Btree<T>::insert(T key) {
  // TODO
  assert(false);
}

// remove:
// Deletes a key from the tree.
template <class T>
void Btree<T>::remove(T key) {
  // TODO
  assert(false);
}

// -----------------------------------------------------------------------------
// Node definitions
// -----------------------------------------------------------------------------

// Node constructor
template <class T>
Btree<T>::Node::Node(size_t _min_degree, bool _is_leaf, bool _is_root) {
  // Set internal variables.
  leaf_status = _is_leaf;
  is_root = _is_root;
  degree = _min_degree;

  // Allocate room for keys
  keys.reserve(MAX_KEYS);

  // Allocate room for child pointers
  children.reserve(MAX_CHILDREN);
}

// get_num_keys:
// Return the number of keys in this node.
template <class T>
size_t Btree<T>::Node::get_num_keys() {
  assert(keys.size() <= MAX_KEYS);
  return keys.size();
}

// is_leaf:
// Return whether this node is a leaf.
template <class T>
bool Btree<T>::Node::is_leaf() {
  return leaf_status;
}

// is_full:
// Whether or not this node is at max key capacity
template <class T>
bool Btree<T>::Node::is_full() {
  assert(keys.size() <= MAX_KEYS);
  return keys.size() == (2*degree - 1);
}

// vacant_insert_key:
// Insert a key into this non-full node.
template <class T>
void Btree<T>::Node::vacant_insert_key(T key) {
  // Make sure number of keys is sane for this function.
  assert(MAX_KEYS > get_num_keys());

  // Place the new key in such a way that maintains the order invariant.
  for (auto it : keys) {
    if ((it == keys.end()) || (*it > key)) {
      keys.emplace(it, key);
    }
  }
  return;
}

