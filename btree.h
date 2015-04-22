/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 * File: btree.h
 *
 * Author: Tony Allen (cyril0allen@gmail.com)
 *
 * Purpose: B-tree header/implementation.
 **=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#ifndef BTREE_H_
#define BTREE_H_

#include <iostream>
#include <assert.h>
#include <vector>
#include <memory>
#include <utility>

template <class T>
class Btree {
  // Forward declarations
  class Node;

  // Typedefs
  typedef std::shared_ptr<Node> NodePtr;

  // Minimum degree of the tree.
  const size_t degree;
  // Applies to all nodes.
  const size_t MAX_KEYS_PER_NODE;
  // Applies to all nodes except root node.
  const size_t MIN_KEYS_PER_NODE;

  public:

    // Default constructor
    Btree(const size_t _min_degree) :
      degree(_min_degree),
      MAX_KEYS_PER_NODE(2*degree - 1),
      MIN_KEYS_PER_NODE(degree - 1)
  {
      assert(degree >= 2);

      // There should be 0 keys in this newly created B-tree.
      num_keys = 0;

      // Create a node that is a leaf and root.
      root.reset(new Node(degree, true, true));

      // TODO: write to disk?
    }

    ~Btree() {
      //TODO: free up all nodes allocated.
      assert(false);
    }

    // Checks if the tree is obeying B-tree invariants:
    // -- All leaves are at same level.
    // -- Every node (except root) contains at least m-1 keys.
    // -- Every node contains at most 2m-1 keys.
    // -- All nodes have c+1 children, where c is number of keys contained.
    // -- All keys in the node are sorted in increasing order.
    //
    // Returns:
    // True if balanced. False otherwise.
    bool is_sane() {
      // TODO
      assert(false);
    }

    // Number of keys contained in the tree.
    //
    // Returns:
    // Number of keys contained in the tree.
    size_t get_size() {
      return num_keys;
    }

    // Search for a node/index containing value k.
    //
    // Returns:
    // An ordered pair holding a pointer to a vector of keys and an
    // element number containing the piece of data.
    // If nothing is found, a null pointer is returned with index 0.
    std::pair <NodePtr, size_t> search(T key) {
      assert(root != NULL);
      _search(key, root);
    }

    // Insert a key into the tree.
    //
    // Returns:
    // Void.
    void insert(T key){
      // TODO
      assert(false);
    }

    // Deletes a key from the tree.
    //
    // Returns:
    // Void.
    void remove(T key){
      // TODO
      assert(false);
    }

  private:

    // Internal node implementation
    class Node {
      // Typedefs
      typedef std::vector<T> Keys;
      typedef std::vector<NodePtr> ChildNodes;

      // Minimum degree of this node.
      const size_t degree;
      // Max/min values for the node.
      const size_t MAX_KEYS;
      const size_t MIN_KEYS;
      const size_t MAX_CHILDREN;

      public:
        // Default constructor
        Node(size_t _min_degree, bool _is_leaf, bool _is_root) :
          degree(_min_degree),
          MAX_KEYS(2*degree - 1),
          MIN_KEYS(degree - 1),
          MAX_CHILDREN(MAX_KEYS + 1)
      {
          // Set internal variables.
          leaf_status = _is_leaf;
          is_root = _is_root;

          // Allocate room for keys
          keys.reserve(MAX_KEYS);

          // Allocate room for child pointers
          children.reserve(MAX_CHILDREN);
        }

        // Get current number of keys in this node.
        //
        // Returns:
        // Number of keys in this node.
        size_t get_num_keys(){
          assert(keys.size() <= MAX_KEYS);
          return keys.size();
        }

        // Whether the node is a leaf or not.
        //
        // Returns:
        // True if the node is a leaf.
        bool is_leaf(){
          return leaf_status;
        }

        // Whether or not this node is at max key capacity
        //
        // Returns:
        // True if at max capacity. False otherwise.
        bool is_full(){
          assert(keys.size() <= MAX_KEYS);
          return keys.size() == (2*degree - 1);
        }

      private:
        // Whether this node is a leaf node
        bool leaf_status;

        // Whether this node is the root node.
        bool is_root;

        // The keys contained in this node.
        Keys keys;

        // The child pointers contained in this node.
        ChildNodes children;

        // Inserts key into this node if it's not full.
        //
        // Returns:
        // Void.
        void _vacant_insert_key(T key){
          // Make sure this node isn't full.
          assert(MAX_KEYS > get_num_keys());

          // Place the new key in such a way that maintains the order invariant.
          for (auto it : keys) {
            if ((it == keys.end()) || (*it > key)) {
              keys.emplace(it, key);
            }
          }
          return;
        }

        // On a NON-FULL node, takes some index indicating a FULL child
        // and splits the FULL child and adjusts the node to have an additional
        // child.
        //
        // Returns:
        // A std::pair of pointers to the two newly created child nodes.
        std::pair <NodePtr, NodePtr> _vacant_split_child(size_t c_idx){
          // Make sure this node isn't full.
          assert(MAX_KEYS > get_num_keys());
          // TODO
          assert(false);
        }
    };

    // Pointer to the root of this tree.
    NodePtr root;

    // Number of keys in the tree.
    size_t num_keys;

    // Internal method to search for a node/index containing value k.
    //
    // Returns:
    // An ordered pair holding a pointer to a collection of keys and an
    // element number containing the piece of data.
    // If nothing is found, a null pointer is returned with index 0.
    std::pair <NodePtr, size_t> _search(T key, NodePtr local_root) {
      // If we made it to a null child, the key wasn't found.
      if (local_root == NULL) {
        return std::make_pair(NULL, 0);
      }

      auto it = local_root->begin();
      auto first_it = it;
      auto last_it = local_root->end();
      while (it != last_it) {
        // Found it.
        if (*it == key) {
          return std::make_pair(local_root, std::distance(first_it, it));
        // Need to drop a level
        } else if (*it > key) {
          size_t child_number = std::distance(first_it, it);
          return search(key, local_root->children.at(child_number));
        }
        it++;
      }
      // Drop into the far-right child
      return search(key, local_root->children.back());
    }
};

#endif // BTREE_H_

