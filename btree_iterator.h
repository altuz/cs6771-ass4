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
template <typename T>
struct Identity {
    using type = T;
};

template <typename Base, template <typename U> class Constness> class btree_iterator;

template <typename Base, template <typename U> class Constness = Identity> class btree_iterator {
    using   T                 = typename Constness<Base>::type;
public:
    using   difference_type   = std::ptrdiff_t;
    using   iterator_category = std::bidirectional_iterator_tag;
    using   value_type        = T;
    using   pointer           = T*;
    using   reference         = T&;

    using   tree_node         = typename btree<Base>::bnode;
    using   location          = typename std::vector<Base>::iterator;
    // Iterator constructor
    btree_iterator(location currNode, std::shared_ptr<tree_node> currTree, bool end = false) :
    _currNode(currNode), _currTree(currTree), _end(end) {}
    // Comparison operators
    bool operator==(const btree_iterator& other) const {
        auto currTree = _currTree.lock();
        auto otherTree = other._currTree.lock();
        return (_currNode == other._currNode &&
                currTree == otherTree &&
                _end == other._end);
    };
    bool operator!=(const btree_iterator& other) const {
        return !operator==(other);
    }
    // Access operators
    reference operator*() const {
        return (*_currNode);
    };
    pointer operator->() const {
        return &(operator*());
    }
    // Movement
    btree_iterator operator++(int) {
        auto old_val = *this;
        operator++();
        return old_val;
    }

    btree_iterator& operator++() {
        auto currTree = _currTree.lock();
        auto curr_val = *_currNode;
        auto dist = std::distance(currTree->_childVals.begin(), _currNode);
        // check next sub_tree;
        // next_tree is a shared ptr
        auto &next_tree = currTree->_childTrees[dist + 1];
        if(next_tree) {
            // find min here
            auto pair = findMin(next_tree);
            auto dist = pair.first;
            auto tree = pair.second;
            auto vec_it = tree->_childVals.begin() + dist;
            *this = btree_iterator(vec_it, tree);
        }
        // go to next item in current subtree
        // if not at the end, return that one
        else if((++_currNode) != currTree->_childVals.end()){
            // do nothing
        }
        else {
            // at the end of local iterator,
            // go up till you find one that is not at yet at the end
            while(std::distance(_currNode, currTree->_childVals.end()) == 0) {
                auto parent_tree = currTree->_parent.lock();
                if (!parent_tree) {
                    auto pair = findMax(currTree);
                    auto dist = pair.first;
                    auto tree = pair.second;
                    auto vec_it = tree->_childVals.begin() + dist;
                    *this = btree_iterator(vec_it, tree, true);
                    break;
                }
                auto &p_vals = parent_tree->_childVals;
                auto lower_bound = std::lower_bound(p_vals.begin(), p_vals.end(), curr_val);
                _currNode = lower_bound;
                _currTree = parent_tree;
                currTree = _currTree.lock();
            }
        }
        return *this;
    };

    btree_iterator operator--(int) {
        auto old_val = *this;
        operator--();
        return old_val;
    }
    btree_iterator& operator--() {
        if (_end) {
            _end = false;
            return *this;
        }
        // check previous sub_tree;
        // prev_tree is a shared ptr
        auto currTree = _currTree.lock();
        auto curr_val = *_currNode;
        auto dist = std::distance(currTree->_childVals.begin(), _currNode);
        auto &prev_tree = currTree->_childTrees[dist];
        if (prev_tree) {
            // get last iterator from that tree
            auto pair = findMax(prev_tree);
            auto dist = pair.first;
            auto tree = pair.second;
            auto vec_it = tree->_childVals.begin() + dist;
            *this = btree_iterator(vec_it, tree, true);
            _end  = false;
        }
        else if(_currNode != currTree->_childVals.begin()){
            --_currNode;
        }
        else {
            // at the start of local iterator,
            // go up till you find one that is not at yet at the start
            while(std::distance(currTree->_childVals.begin(), _currNode) == 0) {
                auto parent_tree = currTree->_parent.lock() ;
                if (!parent_tree) {
                    auto pair = findMin(currTree);
                    auto dist = pair.first;
                    auto tree = pair.second;
                    auto vec_it = tree->_childVals.begin() + dist;
                    *this = btree_iterator(vec_it, tree, true);
                    break;
                }
                auto &p_vals = parent_tree->_childVals;
                auto lower_bound = std::lower_bound(p_vals.begin(), p_vals.end(), curr_val);
                _currNode = lower_bound;
                _currTree = parent_tree;
                currTree = _currTree.lock();
            }
            --_currNode;
        }
        return *this;
    };
private:
    // Iterator stores current subtree
    // and current location in subtree
    location _currNode;
    std::weak_ptr<tree_node> _currTree;
    bool _end;
};

#endif
