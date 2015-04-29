/*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*=*
 * File: btree.h
 *
 * Author: Tony Allen (cyril0allen@gmail.com)
 *
 * Purpose: B-tree header/implementation.
 *
 * TODO: Use std::lower_bound to find items in sorted vectors.
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
      MIN_KEYS_PER_NODE(degree - 1),
      MIN_KEYS_FOR_RM(degree)
    {
      assert(degree >= 2);

      // There should be 0 keys in this newly created B-tree.
      num_keys = 0;

      // Create a node that is a leaf and assign to root.
      root.reset(new Node(this, true));
    }

    ~Btree() { }

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
      assert(root->get_num_keys() <= MAX_KEYS_PER_NODE);
      // Check number of children makes sense with number of keys
      assert((root->is_leaf()) ||
             (root->children.size() - 1 == root->get_num_keys()));

      if (root->is_full()) {
        // Make a new node to potentially be the new root.
        NodePtr nnode(new Node(this, false));
        nnode->children.emplace_back(root);
        root = nnode;
        // The new root should have a single child
        assert(root->children.size() == 1);
        root->_vacant_split_child(0);
        _vacant_insert(key, nnode);
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
      // Call recursive removal function starting at the root.
      _remove(key, root);

      num_keys--;

      // Handle the case where the tree depth has decreased.
      _fix_empty_root();
    }

  private:

    // Minimum size of node for key removal.
    const size_t MIN_KEYS_FOR_RM;

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
        Node(Btree<T>* _tree, bool _is_leaf) :
          degree(_tree->degree),
          MAX_KEYS(2*degree - 1),
          MIN_KEYS(degree - 1),
          MAX_CHILDREN(MAX_KEYS + 1),
          tree(_tree)
      {
          // Set internal variables.
          leaf_status = _is_leaf;

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
          assert(get_num_keys() <= MAX_KEYS);
          return get_num_keys() == MAX_KEYS;
        }

        // Whether this node is the root node.
        bool is_root() {
          return this == (tree->root).get();
        }

      private:
        // Pointer to the parent tree.
        Btree<T>* tree;

        // Whether this node is a leaf node
        bool leaf_status;

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
          assert((get_num_keys() == children.size() - 1) || is_leaf());

          // Child being split.
          NodePtr c1 = children.at(c_idx);
          // Allocate new child
          NodePtr c2(new Node(tree, c1->leaf_status));

          // Copy relevant keys from c1 to c2.
          for (auto it = c1->keys.begin() + degree;
              it != c1->keys.end(); ++it) {
            // Can treat this like a leaf since children are getting copied
            c2->_vacant_insert_key_in_node(*it);
          }
          assert((get_num_keys() == children.size() - 1) || is_leaf());

          // Copy child pointers
          if (!c1->is_leaf()) {
            for (size_t i = degree; i < MAX_CHILDREN; i++) {
              c2->children.emplace_back(c1->children.at(i));
            }
          } else {
            c2->children.resize(0);
          }

          // Insert midpoint into parent node's keys.
          _vacant_insert_key_in_node(c1->keys.at(degree - 1));
          children.emplace(children.begin() + c_idx + 1, c2);

          // Remove all copied keys and children from c1.
          c1->keys.resize(degree - 1);
          if (!c1->is_leaf()) {
            c1->children.resize(degree);
          }

          // Check if sizes make sense for:
          // parent node
          assert(get_num_keys() == children.size() - 1);
          // child nodes
          assert(c1->get_num_keys() == degree - 1);
          assert((c1->children.size() == degree) || c1->is_leaf());
          assert(c2->get_num_keys() == degree - 1);
          assert((c2->children.size() == degree) || c2->is_leaf());
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

    // Check if a key exists in a given node. If so, overwrite it and
    // return true. Otherwise, do nothing and return false.
    //
    // Returns:
    // True if the key exists in the node and is overwritten. False if
    // the key does not exist in the node.
    bool _is_duplicate(T key, NodePtr nd) {
      for (auto it = nd->keys.begin(); it != nd->keys.end(); it++) {
        if (key == *it) {
          *it = key;
          return true;
        }
      }
      return false;
    }

    // Inserts a key into a node which is assumed to be vacant.
    //
    // Returns:
    // Void.
    void _vacant_insert(T key, NodePtr nd) {
      // Make sure node is not full
      assert(!nd->is_full());
      // Check the child/key counts make sense
      assert((nd->is_leaf()) ||
             (nd->get_num_keys() + 1 == nd->children.size()));

      // Index variable.
      size_t idx = 0;
      // Possible child node to descend into.
      NodePtr nxt;

      // If nd is leaf node, just insert the key
      if (nd->is_leaf()) {
        nd->_vacant_insert_key_in_node(key);
        return;
      }

      // If key is in nd, just replace it and call it a day.
      if ( _is_duplicate(key, nd)) {
        return;
      }

      // Determine the correct child node to recursively drop into.
      if (key > nd->keys.back()) {
        idx = nd->keys.size();
      } else if (key >= nd->keys.front()) {
        while ((idx < nd->keys.size()) && (key > nd->keys.at(idx))) {
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
          if (local_root->is_leaf()) {
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
      if (local_root->is_leaf()) {
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
      assert((nn->is_leaf()) ||
             (nn->get_num_keys() + 1 == nn->children.size()));
      for (size_t i = 0; i < nn->get_num_keys() - 1; i++) {
        if (!nn->is_leaf()) {
          _walk(nn->children.at(i));
        }
        DEBUG << nn->keys.at(i) << " to " << nn->keys.at(i+1) << "\n";
        assert(nn->keys.at(i) < nn->keys.at(i+1));
      }
      DEBUG << nn->keys.back() << "\n";
      if (!nn->is_leaf()) {
        _walk(nn->children.at(nn->get_num_keys()));
      }
    }

    // Removes key k from a node nd with number of keys greater than or
    // equal to the degree of the btree unless nd is root.
    //
    // Returns:
    // Void.
    void _rm_key_from_node(T key, NodePtr nd) {
      // Make sure we meed the minimum size requirement.
      assert((nd->get_num_keys() >= MIN_KEYS_FOR_RM) || nd->is_root());
      // Make sure key/node numbers are sane
      assert((nd->get_num_keys() == nd->children.size() - 1) ||
             nd->is_leaf());

      for (auto it = nd->keys.begin(); it < nd->keys.end(); it++) {
        if (*it == key) {
          nd->keys.erase(it);
          return;
        } else if (*it > key) {
          break;
        }
      }

      // Reaching this point means the key was not found. Throw error.
      throw std::out_of_range ("Unable to find key for removal.");
    }

    // Merges node nd_z  and key into nd_y and deletes nd_z.
    // nd_z and nd_y must have degree-1 keys in them exactly.
    //
    // Warning: nd_z is a GONER once this is called, so handle
    // any pointers to it appropriately.
    //
    // Returns:
    // Pointer to the newly merged node.
    NodePtr _merge_nodes(NodePtr nd_y, T key, NodePtr nd_z) {
      // Check the size requirements
      assert(nd_y->get_num_keys() == (degree - 1));
      assert(nd_z->get_num_keys() == (degree - 1));
      // Make sure key/node numbers are sane
      assert((nd_z->get_num_keys() == nd_z->children.size() - 1) ||
             nd_z->is_leaf());
      // Make sure key/node numbers are sane
      assert((nd_y->get_num_keys() == nd_y->children.size() - 1) ||
             nd_y->is_leaf());
      // Check ordering will output a sorted block
      assert(nd_y->keys.back() < key);
      assert(key < nd_z->keys.front());
      // Check ordering of keys
      _check_ordering(nd_y);
      _check_ordering(nd_z);

      nd_y->keys.emplace_back(key);

      // Loop through all keys in nd_z and append them to nd_y
      for (auto it = nd_z->keys.begin(); it < nd_z->keys.end(); it++) {
        nd_y->keys.emplace_back(*it);
      }

      // Loop through all children in nd_z and append them to nd_y
      for (auto it = nd_z->children.begin(); it < nd_z->children.end(); it++) {
        nd_y->children.emplace_back(*it);
      }

      // Destroy nd_z
      nd_z.reset();

      // Check ordering of keys
      _check_ordering(nd_y);

      return nd_y;
    }

    // Finds the predecessor key k_pr to the key k in the subtree rooted at nd.
    // k must belong to the parent of nd whose keys are all greater than the
    // keys in nd.
    //
    // Returns:
    // The key k_pr
    T _get_predecessor(T k, NodePtr nd) {
      if (nd->is_leaf()) {
        return nd->keys.back();
      } else {
        return _get_predecessor(k, nd->children.back());
      }
    }

    // Finds the successor key k_s to the key k in the subtree rooted at nd.
    // k must belong to the parent of nd whose keys are all less than the keys
    // in nd.
    //
    // Returns:
    // The key k_s
    T _get_successor(T k, NodePtr nd) {
      if (nd->is_leaf()) {
        return nd->keys.front();
      } else {
        return _get_successor(k, nd->children.front());
      }
    }

    // Removes key k at index idx from internal node nd.
    //
    // Example tree:
    //
    //      key
    //    /     \
    //   Y       Z
    //
    // Returns:
    // Void.
    void _rm_key_from_internal_node(T key, size_t idx, NodePtr nd) {
      // Make sure we meed the minimum size requirement.
      assert((nd->get_num_keys() >= MIN_KEYS_FOR_RM) || nd->is_root());
      // Make sure this is not a leaf
      assert(!nd->is_leaf());
      // Make sure the element exists
      assert(idx <= nd->get_num_keys() - 1);
      // Make sure key/node numbers are sane
      assert(nd->get_num_keys() == nd->children.size() - 1);
      // Check ordering of keys
      _check_ordering(nd);

      NodePtr nd_y = nd->children.at(idx);
      NodePtr nd_z = nd->children.at(idx+1);

      // If the child (y) that precedes k in nd has keys >= degree of the btree,
      // find the predecessor key and replace k with that key.
      if (nd_y->get_num_keys() >= degree) {
        assert(nd->keys.at(idx) > nd_y->keys.back());
        nd->keys.at(idx) = _get_predecessor(nd->keys.at(idx), nd_y);
        _check_ordering(nd_y);
        return _remove(nd->keys.at(idx), nd_y);
      // If y has fewer keys than the degree, examine z. If z has number of
      // keys >= degree, then find the successor key in z and replace k with
      // that key.
      } else if ((nd_y->get_num_keys() < degree) &&
                 (nd_z->get_num_keys() >= degree)) {
        assert(nd->keys.at(idx) < nd_z->keys.front());
        nd->keys.at(idx) = _get_successor(nd->keys.at(idx), nd_z);
        _check_ordering(nd_z);
        return _remove(nd->keys.at(idx), nd_z);
      // Otherwise, if both y and z have exactly MIN_KEYS keys, merge k and all
      // keys in z into y. Delete node z and delete k from y.
      } else {
        assert(nd_y->keys.back() < nd->keys.at(idx));
        assert(nd->keys.at(idx) < nd_z->keys.front());
        T tmp = nd->keys.at(idx);
        // Don't want to lose the new root, so handle erasing a 1 key root
        if ((nd->is_root()) && (nd->get_num_keys() == 1)) {
          root = nd_y;
        }
        nd->keys.erase(nd->keys.begin() + idx);
        nd->children.erase(nd->children.begin() + idx + 1);
        nd_y = _merge_nodes(nd_y, tmp, nd_z);
        // Make sure the root didn't just become empty.
        _fix_empty_root();
        _check_ordering(nd_y);
        return _remove(key, nd_y);
      }
    }

    // Remove the key from a an internal root node that DOES NOT contain the
    // key. idx is the index of the child of the node that was passed in that
    // we will continue the search on.
    //
    // Returns:
    // Void.
    void _rm_from_internal_node_without_key(T key, size_t idx, NodePtr nd) {
      // Make sure this isn't a leaf
      assert(!nd->is_leaf());
      // Make sure that the index is within the range of children.
      assert(idx < nd->children.size());
      // Make sure we meet the node removal requirement.
      assert((nd->get_num_keys() >= MIN_KEYS_FOR_RM) || nd->is_root());
      // Make sure key/node numbers are sane
      assert(nd->get_num_keys() == nd->children.size() - 1);
      // Check ordering of keys
      _check_ordering(nd);

      // Pointer to the child node that roots the key we want to remove.
      NodePtr cn = nd->children.at(idx);
      // Pointer to the left sibling of cn.
      NodePtr lsibling;
      // Pointer to the right sibling of cn.
      NodePtr rsibling;
      // Pointer to the sibling which will be borrowed from.
      NodePtr borrow_target;

      // Determine the values for the siblings. Make them NULL if non-existant.
      lsibling = (idx == 0) ? NULL : nd->children.at(idx - 1);
      rsibling =
        (idx == nd->children.size() - 1) ? NULL : nd->children.at(idx + 1);

      // This would be a nice place to check whether siblings match leaf status
      if ((lsibling != NULL) && (rsibling != NULL)) {
        assert(lsibling->is_leaf() == rsibling->is_leaf());
      }

      // If there are more than degree-1 keys, we can just descend into it.
      if (cn->get_num_keys() > (degree - 1)) {
        _check_ordering(cn);
        return _remove(key, cn);
      }

      // If cn has an immediate sibling with at least degree keys, give cn
      // an extra key by moving a key from nd down into cn and moving a key from
      // cn's sibling up into nd. The child pointer to the right of the key from
      // nd will be moved into cn by appending it to the child list.

      // Determine who to borrow from. If we can't borrow from anyone, assign
      // NULL and handle that case.
      T tmp_key;
      NodePtr tmp_child;
      if ((lsibling != NULL) && (lsibling->get_num_keys() >= degree)) {
        tmp_key = nd->keys.at(idx - 1);
        if (!lsibling->is_leaf()) {
          tmp_child = lsibling->children.back();
        }
        // Replace key in nd with one from lsibling
        nd->keys.at(idx - 1) = lsibling->keys.back();
        // Add the child ptr from lsibling and key from nd to cn.
        cn->keys.emplace(cn->keys.begin(),tmp_key);
        if (!lsibling->is_leaf()) {
          cn->children.emplace(cn->children.begin(), tmp_child);
        }
        // Remove last child and key from lsibling.
        lsibling->keys.erase(lsibling->keys.end() - 1);
        if (!lsibling->is_leaf()) {
          lsibling->children.erase(lsibling->children.end() - 1);
        }
        _check_ordering(cn);
        return _remove(key, cn);
      } else if ((rsibling != NULL) && (rsibling->get_num_keys() >= degree)) {
        tmp_key = nd->keys.at(idx);
        if (!rsibling->is_leaf()) {
        tmp_child = rsibling->children.front();
        }
        // Replace key in nd with one from rsibling
        nd->keys.at(idx) = rsibling->keys.front();
        // Add the child ptr from rsibling and key fron nd to cn.
        cn->keys.emplace_back(tmp_key);
        if (!rsibling->is_leaf()) {
          cn->children.emplace_back(tmp_child);
        }
        // Remove first child and first key from rsibling.
        rsibling->keys.erase(rsibling->keys.begin());
        if (!rsibling->is_leaf()) {
          rsibling->children.erase(rsibling->children.begin());
        }
        _check_ordering(cn);
        return _remove(key, cn);
      }

      // If we've made it this far, no siblings had at least degree keys.
      // In the case where a key is not present in an internal node and
      // the child node that roots the subtree that may contain the key
      // has only degree-1 keys, we must guarantee that we descent into
      // a node containing at least degree keys.
      //
      // This section merges the child node cn with a sibling and moves
      // a key down from nd to become the median key for the newly merged node.
      // We then recursively drop down into the new node to remove the key.
      T tmp;
      if (lsibling != NULL) {
        tmp = nd->keys.at(idx - 1);
        nd->children.erase(nd->children.begin() + idx);
        nd->keys.erase(nd->keys.begin() + idx - 1);
        _merge_nodes(lsibling, tmp, cn);
        // Make sure the root didn't just become empty.
        _fix_empty_root();
        _check_ordering(lsibling);
        return _remove(key, lsibling);
      } else {
        tmp = nd->keys.at(idx);
        nd->children.erase(nd->children.begin() + idx + 1);
        nd->keys.erase(nd->keys.begin() + idx);
        _merge_nodes(cn, tmp, rsibling);
        // Make sure the root didn't just become empty.
        _fix_empty_root();
        _check_ordering(cn);
        return _remove(key, cn);
      }
    }

    // Fixes empty root if it exists
    //
    // Returns:
    // Void
    void _fix_empty_root() {
        assert((root->get_num_keys() == root->children.size() - 1) ||
               (root->is_leaf()));
        if ((root->get_num_keys() == 0) && (!root->is_leaf())) {
          root = root->children.at(0);
        }
    }

    // Internal key removal function. Removes k from the subtree rooted
    // under node nd. If the key is not found in the tree, an exception
    // is thrown identical to search().
    //
    // Returns:
    // Void.
    void _remove(T key, NodePtr nd) {
      // Make sure we meed the minimum size requirement.
      assert((nd->get_num_keys() >= MIN_KEYS_PER_NODE) || nd->is_root());
      // Make sure key/node numbers are sane
      assert((nd->get_num_keys() == nd->children.size() - 1) ||
             nd->is_leaf());

      // Index placeholder.
      size_t idx = 0;
      // The correct root node for this key.
      NodePtr home_nd;

      // Determine the node to recursively drop into.
      if (key > nd->keys.back()) {
        idx = nd->get_num_keys();
        // Unable to find the key!
        if (nd->is_leaf()) {
          throw std::out_of_range ("Unable to find key for removal.");
        } else {
          home_nd = nd->children.at(idx);
        }
      } else if (key >= nd->keys.front()) {
        while ((idx < nd->get_num_keys()) && (key > nd->keys.at(idx))) {
          idx++;
        }
        if (idx < nd->keys.size()) {
          home_nd = (nd->keys.at(idx) == key) ? nd : nd->children.at(idx);
        } else {
          home_nd = nd->children.at(idx);
        }
      }

      // Verify ordering assertions
      _check_ordering(nd);

      // Is leaf that contains the key. If the leaf doesn't contain the key,
      // throw an error.
      if (nd->is_leaf()) {
        if (home_nd != nd) {
          throw std::out_of_range ("Unable to find key for removal.");
        }
        return _rm_key_from_node(key, nd);
        assert(nd->is_root() || (nd->get_num_keys() >= MIN_KEYS_PER_NODE));
      // Is an internal node that contains the key.
      } else if ((nd == home_nd) && (!nd->is_leaf())) {
      _check_ordering(home_nd);
        return _rm_key_from_internal_node(key, idx, home_nd);
      // Is an internal node that does not contain the key
      } else if (home_nd != nd) {
      _check_ordering(nd->children.at(idx));
        return _rm_from_internal_node_without_key(key, idx, nd);
      }

      // If we've made it this far there was a logic error.
      assert(false);
    }

    // Checks the ordering of keys for node nd via assertions.
    //
    // Returns:
    // Void.
    void _check_ordering(NodePtr nd) {
      assert(nd->is_leaf() ||
        (nd->keys.front() > nd->children.front()->keys.back()));
      assert(nd->is_leaf() ||
        (nd->keys.back() < nd->children.back()->keys.front()));
    }


}; // End class Btree

#endif // BTREE_H_

