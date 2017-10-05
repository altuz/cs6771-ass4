#ifndef BTREE_ITERATOR_H
#define BTREE_ITERATOR_H

#include <iterator>
#include <memory>
#include <vector>
/**
 * You MUST implement the btree iterators as (an) external class(es) in this file.
 * Failure to do so will result in a total mark of 0 for this deliverable.
 **/
template <typename T> class btree;
// iterator related interface stuff here; would be nice if you called your
// iterator class btree_iterator (and possibly const_btree_iterator)
template <typename T> class btree_iterator {
public:
    typedef std::ptrdiff_t                     difference_type;
    typedef std::bidirectional_iterator_tag    iterator_category;
    typedef T                                  value_type;
    typedef T*                                 pointer;
    typedef T&                                 reference;

    typedef typename btree<T>::bnode tree_node;
    typedef typename std::vector<T>::iterator location;
    reference operator*() const {
        return (*_currNode);
    };
    pointer operator->() const { return &(operator*()); }
    btree_iterator& operator++();
    bool operator==(const btree_iterator& other) const {
        return (_currNode == other._currNode && _currTree == other._currTree);
    };
    bool operator!=(const btree_iterator& other) const { return !operator==(other); }
    btree_iterator(location currNode, std::shared_ptr<tree_node> currTree) :
    _currNode(currNode), _currTree(currTree) {}

private:
    // positioning in subtree
    location _currNode;
    // subtree
    std::shared_ptr<tree_node> _currTree;
};

template <typename T> class const_btree_iterator {
};
#endif
