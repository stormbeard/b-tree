/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 * File: btree.h
 *
 * Author: Tony Allen (cyril0allen@gmail.com)
 *
 * Purpose: B-tree header/implementation.
 **=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*/
#ifndef BTREE_H_
#define BTREE_H_

#define DEBUG std::cerr << "DEBUG: (" << __func__ << "): "

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
      root.reset(new Node(this, true, true));

      // TODO: write to disk?
    }

    ~Btree() {
      //TODO: free up all nodes allocated.
      //assert(false);
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
    // The matching key found in the tree.
    // If nothing is found, an out of bound exception is thrown.
    T search(T key) {
      assert(root != NULL);

      // Find a node that may contain the key.
      std::pair<NodePtr, size_t> results = _find_home_node(key, root);
      T ret = (results.first->keys.at(results.second));
      if (ret == key){
        return ret;
      } else {
        throw std::out_of_range ("Unable to find requested key.");
      }
    }

    // Insert a key into the tree.
    //
    // Returns:
    // Void.
    void insert(T key) {
      // Check if root is null
      assert(root != NULL);
      // Check number of keys is sane
      assert(root->keys.size() <= MAX_KEYS_PER_NODE);
      // Check number of children makes sense with number of keys
      assert((root->is_leaf()) ||
             (root->children.size() - 1 == root->keys.size()));

      if (root->is_full()) {
        // Make a new node to potentially be the new root.
        NodePtr nnode(new Node(this, false, true));
        root->is_root = false;
        nnode->children.emplace_back(root);
        root = nnode;
        // The new root should have a single child
        assert(root->children.size() == 1);
        root->_vacant_split_child(0);
        _vacant_insert(key, nnode); // wat
      } else {
        _vacant_insert(key, root);
      }
      num_keys++;
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
      friend class Btree;

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
        Node(Btree<T>* _tree, bool _is_leaf, bool _is_root) :
          degree(_tree->degree),
          MAX_KEYS(2*degree - 1),
          MIN_KEYS(degree - 1),
          MAX_CHILDREN(MAX_KEYS + 1),
          tree(_tree)
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
          return keys.size() == MAX_KEYS;
        }

      private:
        // Pointer to the parent tree.
        Btree<T>* tree;

        // Whether this node is a leaf node
        bool leaf_status;

        // Whether this node is the root node.
        bool is_root;

        // The keys contained in this node.
        Keys keys;

        // The child pointers contained in this node.
        ChildNodes children;

        // Inserts key into this node if it's not full. The children ARE NOT
        // updated, so this MUST be done by the caller.
        //
        // Returns:
        // The index at which the key was inserted into the node.
        size_t _vacant_insert_key_in_node(T key) {
          // Make sure this node isn't full.
          assert(MAX_KEYS > get_num_keys());

          // The index at which the key will be placed.
          size_t index;

          // Place the new key/child such that the order invariant is upheld.
          for (auto it = keys.begin(); it <= keys.end(); ++it) {
            // Find where to insert key
            if ((it == keys.end()) || (*it > key)) {
              keys.emplace(it, key);
              index = std::distance(it, keys.end());
              return index;
            // Overwrite the key if found.
            } else if (*it == key) {
              *it = key;
              index = std::distance(it, keys.end());
              return index;
            }
          }

          // Should never make it to this point.
          assert(false);
          return -1;
        }

        // On a NON-FULL node, takes some index indicating a FULL child
        // and splits the FULL child and adjusts the node to have an additional
        // child.
        //
        // Returns:
        // A std::pair of pointers to the two newly created child nodes. The
        // first node in the pair is the node that was just split and the second
        // is the newly created node.
        std::pair <NodePtr, NodePtr> _vacant_split_child(size_t c_idx) {
          // Make sure this node isn't full.
          assert(MAX_KEYS > get_num_keys());
          // Check validity of index passed in.
          assert(children.size() > c_idx);
          // Make sure the child being split is actually full.
          assert(children.at(c_idx)->is_full());
          // Verify key/child number invariant
          assert(get_num_keys() == children.size() - 1);

          // Child being split.
          NodePtr c1 = children.at(c_idx);
          // Allocate new child
          NodePtr c2(new Node(tree, true, c1->leaf_status));

          // Copy relevant keys from c1 to c2.
          // TODO: Reverse this loop.
          for (auto it = c1->keys.begin() + degree;
              it != c1->keys.end(); ++it) {
            // Can treat this like a leaf since children are getting copied
            c2->_vacant_insert_key_in_node(*it);
          }
          assert(get_num_keys() == children.size() - 1);

          // Copy child pointers
          if (!c1->is_leaf()) {
            for (size_t i = degree; i < MAX_CHILDREN; i++) {
              c2->children.emplace_back(c1->children.at(i));
            }
          } else {
            c2->children.resize(degree);
          }

          // Insert midpoint into parent node's keys.
          _vacant_insert_key_in_node(c1->keys.at(degree - 1));
          children.emplace(children.begin() + c_idx + 1, c2);

          // Remove all copied keys and children from c1.
          c1->keys.resize(degree - 1);
          c1->children.resize(degree);

          // Check if sizes make sense for:
          // parent node
          assert(get_num_keys() == children.size() - 1);
          // child nodes
          assert(c1->get_num_keys() == degree - 1);
          assert(c1->children.size() == degree);
          assert(c2->get_num_keys() == degree - 1);
          assert(c2->children.size() == degree);
          // Make sure c1/c2 is in the correct spot on this node's child list.
          assert(c1 == children.at(c_idx));
          assert(c2 == children.at(c_idx + 1));
          return std::make_pair(c1, c2);
        }

    }; // End class Node

    // Pointer to the root of this tree.
    NodePtr root;

    // Number of keys in the tree.
    size_t num_keys;

    // Inserts a key into a node which is assumed to be vacant.
    //
    // Returns:
    // Void.
    void _vacant_insert(T key, NodePtr nd) {
      // Make sure node is not full
      assert(!nd->is_full());
      // Check the child/key counts make sense
      assert((nd->is_leaf()) || (nd->keys.size() + 1 == nd->children.size()));

      size_t idx = nd->keys.size(); // Index variable.
      NodePtr nxt;    // Possible child node to descend into.

      // If nd is leaf node, just insert the key
      if (nd->is_leaf()) {
        nd->_vacant_insert_key_in_node(key);
        return;
      }

      // Determine the correct child node to recursively drop into.
      if (key > nd->keys.back()) {
        idx = nd->keys.size();
      } else {
        while ((idx < nd->keys.size()) && (key < nd->keys.at(idx))) {
          idx++;
        }
      }
      nxt = nd->children.at(idx);

      // If the child is vacant, just drop in
      if (!nxt->is_full()) {
       return _vacant_insert(key, nxt);
      // If the child is full, split then decide the correct one to drop into.
      // The key that bubbled up after the split should be at they key at idx.
      } else {
        nd->_vacant_split_child(idx);
        if (key < nd->keys.at(idx)) {
          nxt = nd->children.at(idx);
        // If identical key bubbled, replace it and be done.
        } else if (key == nd->keys.at(idx)) {
          nd->keys[idx] = key;
          return;
        } else {
          nxt = nd->children.at(idx + 1);
        }
        return _vacant_insert(key, nxt);
      }
    }

    // Internal method to find a node that would be hosting the key passed in.
    //
    // Returns:
    // A pointer to a node that would make a good insertion point.
    std::pair<NodePtr, size_t> _find_home_node(T key, NodePtr local_root) {
      // Key iterator shortcuts
      auto it = local_root->keys.begin();
      auto first_it = it;
      auto last_it = local_root->keys.end();

      // Cycle through all the keys here.
      while (it != last_it) {
        // Found the key.
        if (*it == key) {
          return std::make_pair(local_root, std::distance(first_it, it));
        // Need to drop a level. Check if it's valid before doing so.
        } else if (*it > key) {
          size_t child_number = std::distance(first_it, it);
          if (local_root->children.at(child_number) == NULL) {
            // Nowhere to go, this is as good as it gets.
            return std::make_pair(local_root, 0);
          } else {
            // Dig deeper in the tree.
            return _find_home_node(key, local_root->children.at(child_number));
          }
        }
        it++;
      }

      // Drop into the far-right child if it's valid.
      if (local_root->children.back() == NULL) {
        // Nowhere to go, this is as good as it gets.
        return std::make_pair(local_root, 0);
      } else {
        return _find_home_node(key, local_root->children.back());
      }
    }


    // Walks through the tree printing all values in order.
    //
    // Returns:
    // Void.
    void _walk(NodePtr nn) {
      // Check the child/key counts make sense
      assert((nn->is_leaf()) || (nn->keys.size() + 1 == nn->children.size()));
      for (size_t i = 0; i < nn->keys.size(); i++) {
        if (!nn->is_leaf()) {
          _walk(nn->children.at(i));
        }
      }
      if (!nn->is_leaf()) {
        _walk(nn->children.at(nn->keys.size()));
      }
    }

}; // End class Btree

#endif // BTREE_H_

