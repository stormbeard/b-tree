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
#include <assert.h>
#include <vector>
#include <utility>

using FunctionPtr = void (*)();


template <class T>
class Btree {

  // Applies to all nodes.
  const size_t MAX_KEYS_PER_NODE = 2*degree - 1;
  // Applies to all nodes except root node.
  const size_t MIN_KEYS_PER_NODE = degree - 1;

  public:

    // Default constructor
    Btree(size_t _min_degree);

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
    size_t get_size();

    // Search for a key with value k
    //
    // Returns:
    // An ordered pair containing a vector of node data and an element number
    // containing the piece of data.
    std::pair <std::vector<T>, size_t> search(T key);

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

    // Internal node implementation
    class Node {
      const size_t MAX_KEYS = 2*degree - 1;
      const size_t MIN_KEYS = degree - 1;
      const size_t MAX_CHILDREN = MAX_KEYS + 1;

      public:
        // Default constructor
        Node(size_t _min_degree, bool _is_leaf, bool _is_root);

        // Get current number of keys in this node.
        //
        // Returns:
        // Number of keys in this node.
        size_t get_num_keys();

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

        // Inserts key into this node if it's not full.
        //
        // Returns:
        // Void.
        void vacant_insert_key(T key);

      private:
        // Whether this node is a leaf node
        bool leaf_status;

        // Minimum degree of this node.
        const size_t degree;

        // Whether this node is the root node.
        bool is_root;

        // The keys contained in this node.
        std::vector<T> keys;

        // The child pointers contained in this node.
        std::vector<Node*> children;
    };

    // Pointer to the root of this tree.
    Node *root;

    // Minimum degree of the tree.
    const size_t degree;

    // Number of keys in the tree.
    size_t num_keys;
};

#endif // BTREE_H_

