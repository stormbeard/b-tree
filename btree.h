/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 * File: btree.h
 *
 * Author: Tony Allen (cyril0allen@gmail.com)
 *
 * Purpose: Header and interface for B-tree.
 **=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#ifndef BTREE_H_
#define BTREE_H_

#include <iostream>
#include <assert>
#include <vector>
#include <utility>

using FunctionPtr = void (*)();

template <class T>
class Btree {
  public:

    // Checks if the tree is obeying B-tree invariants:
    // -- All leaves are at same level.
    // -- Every node (except root) contains at least m-1 keys.
    // -- Every node contains at most 2m-1 keys.
    // -- All nodes have c+1 children, where c is number of keys contained.
    // -- All keys in the node are sorted in increasing order.
    //
    // Returns:
    // True if balanced. False otherwise.
    bool is_sane();

    // Number of keys contained in the tree.
    //
    // Returns:
    // Number of keys contained in the tree.
    unsigned long long get_size();

    // Search for a key with value k
    //
    // Returns:
    // An ordered pair containing a vector of node data and an element number
    // containing the piece of data.
    std::pair <std::vector<T>, unsigned int> search(T key);

    // Insert a key into the tree.
    //
    // Returns:
    // Void.
    void insert(T key);

    // Deletes a key from the tree.
    //
    // Returns:
    // Void.
    void remove(T key);

  private:

    // Pointer to the root of this tree.
    Node<T> *root;

    // Minimum degree of the tree.
    unsigned int degree;

    // Number of keys in the tree.
    unsigned long long num_keys;

    // Default constructor
    Btree(unsigned int _min_degree);

    class Node {
      public:
        // Get current number of keys in this node.
        //
        // Returns:
        // Number of keys in this node.
        unsigned int get_num_keys();

        // Whether the node is a leaf or not.
        //
        // Returns:
        // True if the node is a leaf.
        bool is_leaf();

        // Whether or not this node is at max key capacity
        //
        // Returns:
        // True if at max capacity. False otherwise.
        bool is_full();

      private:
        // Default constructor
        Node(unsigned int _min_degree, bool _is_leaf);

        // Current number of keys
        unsigned int num_keys;

        // Whether this node is a leaf node
        bool is_leaf;

        // The keys contained in this node.
        std::vector<T> keys;

        // The child pointers contained in this node.
        std::vector<Node*> children;
    }
}

#endif // BTREE_H_
