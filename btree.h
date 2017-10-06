/**
 * The btree is a linked structure which operates much like
 * a binary search tree, save the fact that multiple client
 * elements are stored in a single node.    Whereas a single element
 * would partition the tree into two ordered subtrees, a node
 * that stores m client elements partition the tree
 * into m + 1 sorted subtrees.
 */

#ifndef BTREE_H
#define BTREE_H

#include <iostream>
#include <cstddef>
#include <utility>
#include <vector>
#include <memory>
#include <iterator>
// we better include the iterator
#include "btree_iterator.h"

// we do this to avoid compiler errors about non-template friends
// what do we do, remember? :)

template <typename T>
class btree {
private:
    class bnode {
    public:
        // There are n-1 sub-trees in between the n values.
        // There are one sub-tree in each end
        // Thus in total we have n + 1 sub trees
        unsigned int _size;
        std::vector<T> _childVals;
        std::vector<std::shared_ptr<bnode>> _childTrees;
        std::weak_ptr<bnode> _parent;

        bnode(size_t maxNodeElems = 40, std::shared_ptr<bnode> parent = nullptr) : _size(maxNodeElems), _childTrees(_size + 1), _parent(parent) {
            // reserve space instead of populating them for easy sorted insertion.
            _childVals.reserve(_size);
        };

        ~bnode() = default;
        /**
         * Goes through the tree using level-order traversal a.k.a bfs
         * Returns a vector of the values
         */
        std::vector<T> bfs() const {
            // Copy all values
            std::vector<T> res(_childVals);
            for(auto child : _childTrees) {
                if(!child)
                    continue;
                auto c_res = (*child).bfs();
                std::move(c_res.begin(), c_res.end(), std::back_inserter(res));
            }
            return res;
        }
    };
    // Tree just has root
    std::shared_ptr<bnode> _root;
    using dt_tuple = std::pair<size_t, std::shared_ptr<bnode>>;

public:
    /** Hmm, need some iterator typedefs here... friends? **/
    using iterator = btree_iterator<T>;
    using const_iterator = btree_iterator<T, std::add_const>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    friend iterator;
    friend const_iterator;
    friend reverse_iterator;
    friend const_reverse_iterator;

    /**
     * Constructs an empty btree.    Note that
     * the elements stored in your btree must
     * have a well-defined zero-arg constructor,
     * copy constructor, operator=, and destructor.
     * The elements must also know how to order themselves
     * relative to each other by implementing operator<
     * and operator==. (These are already implemented on
     * behalf of all built-ins: ints, doubles, strings, etc.)
     *
     * @param maxNodeElems the maximum number of elements
     *                that can be stored in each B-Tree node
     */
    btree(size_t maxNodeElems = 40) : _root(std::make_shared<bnode>(maxNodeElems)) {};

    /**
     * The copy constructor and    assignment operator.
     * They allow us to pass around B-Trees by value.
     * Although these operations are likely to be expensive
     * they make for an interesting programming exercise.
     * Implement these operations using value semantics and
     * make sure they do not leak memory.
     */

    /**
     * Copy constructor
     * Creates a new B-Tree as a copy of original.
     *
     * @param original a const lvalue reference to a B-Tree object
     */
    btree(const btree<T>& original) : btree(original._root->_size) {
        // Get a list of all elements in original tree
        // Insert them one by one
        auto all_values = original._root->bfs();
        for (auto i : all_values) {
            this->insert(i);
        }
    };

    /**
     * Move constructor
     * Creates a new B-Tree by "stealing" from original.
     *
     * @param original an rvalue reference to a B-Tree object
     */
    btree(btree<T>&& original) : _root(std::move(original._root)){};

    /**
     * Copy assignment
     * Replaces the contents of this object with a copy of rhs.
     *
     * @param rhs a const lvalue reference to a B-Tree object
     */
    btree<T>& operator=(const btree<T>& rhs) {
        auto rhs_copy(rhs);
        *this = std::move(rhs_copy);
        return *this;
    };

    /**
     * Move assignment
     * Replaces the contents of this object with the "stolen"
     * contents of original.
     *
     * @param rhs a const reference to a B-Tree object
     */
    btree<T>& operator=(btree<T>&& rhs) {
        auto rhs_new(std::move(rhs));
        this->_root = std::move(rhs_new._root);
        return *this;
    };

    /**
     * Puts a breadth-first traversal of the B-Tree onto the output
     * stream os. Elements must, in turn, support the output operator.
     * Elements are separated by space. Should not output any newlines.
     *
     * @param os a reference to a C++ output stream
     * @param tree a const reference to a B-Tree object
     * @return a reference to os
     */
    friend std::ostream& operator<< (std::ostream& os, const btree<T>& tree) {
        auto vals = tree._root->bfs();
        std::cout << "There are " << vals.size() << " nodes \n";
        for (auto i = vals.cbegin(); i != vals.cend(); ++i) {
            if (i != vals.cbegin())
                os << ' ';
            os << *i;
        }
        return os;
    };

    /**
     * The following can go here
     * -- begin()
     * -- end()
     * -- rbegin()
     * -- rend()
     * -- cbegin()
     * -- cend()
     * -- crbegin()
     * -- crend()
     */

     reverse_iterator rbegin() {
         return reverse_iterator(end());
     }

     reverse_iterator rend() {
          return reverse_iterator(begin());
     }

     const_reverse_iterator rcbegin() const {
         return const_reverse_iterator(cend());
     }

     const_reverse_iterator rcend() const {
         return const_reverse_iterator(cbegin());
     }

     const_reverse_iterator rbegin() const {
         return rcbegin();
     }

     const_reverse_iterator rend() const {
         return rcend();
     }

     const_iterator cend() const {
         auto pair = findMax(_root);
         auto dist = pair.first;
         auto tree = pair.second;
         auto vec_it = tree->_childVals.begin() + dist;
         return const_iterator(vec_it, tree, true);
     }

     const_iterator cbegin() const {
         auto pair = findMin(_root);
         auto dist = pair.first;
         auto tree = pair.second;
         auto vec_it = tree->_childVals.begin() + dist;
         return const_iterator(vec_it, tree);
     }
     /**
      * Non Const
      */
     iterator end() {
         auto pair = findMax(_root);
         auto dist = pair.first;
         auto tree = pair.second;
         auto vec_it = tree->_childVals.begin() + dist;
         return iterator(vec_it, tree, true);
     };
     const_iterator end() const {
         return cend();
     }
     iterator begin() {
         auto pair = findMin(_root);
         auto dist = pair.first;
         auto tree = pair.second;
         auto vec_it = tree->_childVals.begin() + dist;
         return iterator(vec_it, tree);
     }

     const_iterator begin() const {
         return cbegin();
     }
    /**
        * Returns an iterator to the matching element, or whatever
        * the non-const end() returns if the element could
        * not be found.
        *
        * @param elem the client element we are trying to match.    The elem,
        *                if an instance of a true class, relies on the operator< and
        *                and operator== methods to compare elem to elements already
        *                in the btree.    You must ensure that your class implements
        *                these things, else code making use of btree<T>::find will
        *                not compile.
        * @return an iterator to the matching element, or whatever the
        *                 non-const end() returns if no such match was ever found.
        */
    iterator find(const T& elem) {
        auto pair = find(elem, _root);
        auto dist = pair.first;
        auto tree = pair.second;
        auto vec_it = tree->_childVals.begin() + dist;
        return iterator(vec_it, tree, (*vec_it != elem));
    };

    /**
        * Identical in functionality to the non-const version of find,
        * save the fact that what's pointed to by the returned iterator
        * is deemed as const and immutable.
        *
        * @param elem the client element we are trying to match.
        * @return an iterator to the matching element, or whatever the
        *                 const end() returns if no such match was ever found.
        */
    const_iterator find(const T& elem) const {
        auto pair = find(elem, _root);
        auto dist = pair.first;
        auto tree = pair.second;
        auto vec_it = tree->_childVals.begin() + dist;
        return const_iterator(vec_it, tree, (*vec_it != elem));
    };

    /**
        * Operation which inserts the specified element
        * into the btree if a matching element isn't already
        * present.    In the event where the element truly needs
        * to be inserted, the size of the btree is effectively
        * increases by one, and the pair that gets returned contains
        * an iterator to the inserted element and true in its first and
        * second fields.
        *
        * If a matching element already exists in the btree, nothing
        * is added at all, and the size of the btree stays the same.    The
        * returned pair still returns an iterator to the matching element, but
        * the second field of the returned pair will store false.    This
        * second value can be checked to after an insertion to decide whether
        * or not the btree got bigger.
        *
        * The insert method makes use of T's zero-arg constructor and
        * operator= method, and if these things aren't available,
        * then the call to btree<T>::insert will not compile.    The implementation
        * also makes use of the class's operator== and operator< as well.
        *
        * @param elem the element to be inserted.
        * @return a pair whose first field is an iterator positioned at
        *                 the matching element in the btree, and whose second field
        *                 stores true if and only if the element needed to be added
        *                 because no matching element was there prior to the insert call.
        */
    std::pair<iterator, bool> insert(const T& elem) {
        return insert(elem, _root);
    };
    /**
     * Inserts element to a subtree, used to be recursive, now it's iterative.
     * If found returns a tuple, the iterator and a bool.
     */
    std::pair<iterator, bool> insert(const T& elem, std::shared_ptr<bnode> node) {
        auto current = node;
        while (true) {
            auto &c_nodes = current->_childVals;
            auto &c_trees = current->_childTrees;
            auto lower_bound = std::lower_bound(c_nodes.begin(), c_nodes.end(), elem);
            auto subtree_idx = std::distance(c_nodes.begin(), lower_bound);
            auto &subtree     = c_trees[subtree_idx];
            if(lower_bound != c_nodes.end()) {
                if(*lower_bound == elem){
                    return std::make_pair<iterator, bool>({lower_bound, current}, false);
                }
            }
            if(c_nodes.size() < current->_size) {
                auto elem_it = c_nodes.insert(lower_bound, elem);
                return std::make_pair<iterator, bool>({elem_it, current}, true);
            }
            if(!subtree) {
                subtree = std::make_shared<bnode>(bnode(current->_size, current));
            }
            current = subtree;
        }
        return std::make_pair<iterator, bool>(end(), false);
    }
    /**
     * Finds an element in a given subtree
     */
    dt_tuple find(const T& elem, std::shared_ptr<bnode> node) const {
        auto current = node;
        while (true) {
            auto c_nodes = current->_childVals;
            auto c_trees = current->_childTrees;
            // There are n nodes
            // There are n + 1 subtrees
            // Lower bound finds the first iterator in iterator that is >= a given value
            // If not found, it returns the end iterator
            // distance(nodes.begin, nodes.end) = n (since the ending iterator goes past the actual end)
            // Thus if we have to look at a subtree, distance(nodes.begin, lower_bound) will always give the only possible subtree that the element is in
            auto lower_bound = std::lower_bound(c_nodes.begin(), c_nodes.end(), elem);
            auto subtree_idx = std::distance(c_nodes.begin(), lower_bound);
            if(lower_bound != c_nodes.end()) {
                if(*lower_bound == elem) {
                    return dt_tuple(subtree_idx , current);
                }
            }
            auto subtree = c_trees[subtree_idx];
            if (subtree)
                current = subtree;
            else
                break;
        }
        return findMax(_root);
    };
    /**
     * Helper functions, one finds local minimum and the other find local maximum in a subtree
     */
    friend dt_tuple findMin(std::shared_ptr<bnode> node) {
        auto dist = 0;
        auto first_subtree = node->_childTrees[dist];
        if(first_subtree)
           return(findMin(first_subtree));
        return dt_tuple(dist, node);
    }

    friend dt_tuple findMax(std::shared_ptr<bnode> node) {
        auto dist = std::distance(std::begin(node->_childVals), std::end(node->_childVals));
        auto final_subtree = node->_childTrees[dist];
        if(final_subtree)
           return findMax(final_subtree);
        return dt_tuple(dist - 1, node);
    }

    friend iterator convert_tuple(dt_tuple pair) {
        auto dist = pair.first;
        auto tree = pair.second;
        auto vec_it = tree->_childVals.begin() + dist;
        return iterator(vec_it, tree);
    }
    /**
        * Disposes of all internal resources, which includes
        * the disposal of any client objects previously
        * inserted using the insert operation.
        * Check that your implementation does not leak memory!
        */
    ~btree() {
        _root.reset();
    }
};

#endif
