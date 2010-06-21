#ifndef __TERNARY_SEARCH_TREE_H_
#define __TERNARY_SEARCH_TREE_H_

/**
 * @file TernaryTree.h
 * @author Yeogirl Yun
 * @date   2009.11.24
 * @brief define a ternery search tree
 */

#include <stddef.h>
#include <stdint.h>
#include <assert.h>

template <typename K, typename V, V NO_TERMINAL>
class TernarySearchTree;

/** 
 * @brief TernarySearchTree node
 * @param K key type
 * @param V value type
 * @param NO_TERMINAL if a node is not terminal, set the value to this
 */
template <typename K, typename V,V NO_TERMINAL>
class TSTnode {/*{{{*/
protected:
    /** 
     * @brief constructor of TSTnode
     * 
     * @param key initial key
     * @param value initial value
     */
    TSTnode(const K& key=K(), const V& value = NO_TERMINAL):
        key_(key), value_(value), left_(NULL), child_(NULL), right_(NULL) {}
    /** 
     * @brief desconstructor, delete siblings and child if exist
     */
    ~TSTnode() {
        if(left_ != NULL) { delete left_; left_ = NULL; }
        if(child_ != NULL) { delete child_; child_ = NULL; }
        if(right_ != NULL) { delete right_; right_ = NULL; }
    }
protected:
    /** 
     * @brief key
     */
    const K key_;

    /** 
     * @brief value
     */
    V value_;

    /** 
     * @brief left sibling
     */
    TSTnode * left_;

    /** 
     * @brief child node
     */
    TSTnode * child_;

    /** 
     * @brief right sibling
     */
    TSTnode * right_;
public:
    /** 
     * @brief get key
     * 
     * @return const reference of key_
     */
    const K& getKey()const {
        return key_;
    }

    /** 
     * @brief get value
     * 
     * @return const reference of value_
     */
    const V& getValue() const {
        return value_;
    }

    /** 
     * @brief get value
     * 
     * @return reference of value_
     */
    V& getValue() {
        return value_;
    }

    /** 
     * @brief set value
     * 
     * @param v value to set, default is NO_TERMINAL
     * 
     * @return reference of value_ after set
     */
    V& setValue(const V& v = NO_TERMINAL) {
       return value_ = v; 
    }

    /** 
     * @brief search sibling from a key
     * 
     * @param key key to find
     * 
     * @return if exist return it, else return NULL;
     */
    TSTnode * search_sibling(const K& key);

    /** 
     * @brief search a child from key
     * 
     * @param key  key of the child to search
     * 
     * @return if exist, return it , else return NULL
     */
    TSTnode * search_child(const K& key);

    /** 
     * @brief add a sibling, if the child exist, do nothing
     * 
     * @param key key of the sibling
     * @param v value of the sibling
     * 
     * @return added sibling
     */
    TSTnode * add_sibling(const K& key, const V& v = NO_TERMINAL);

    /** 
     * @brief add a child, if the child exist , do nothing
     * 
     * @param key key of the child
     * @param v value of the child
     * 
     * @return the added child
     */
    TSTnode * add_child(const K& key, const V& v = NO_TERMINAL);

    /** 
     * @brief free left_, child_, and right_ space, and set them to NULL
     */
    void clear() {
        if(left_ != NULL) { delete left_; left_ = NULL; }
        if(child_ != NULL) { delete child_; child_ = NULL; }
        if(right_ != NULL) { delete right_; right_ = NULL; }
    }

    friend class TernarySearchTree<K,V,NO_TERMINAL>;
};/*}}}*/

/** 
 * @brief ternary search tree
 * @param K key type
 * @param V value type
 * @param NO_TERMINAL if a node is not terminal, set the value to this
 */
template <typename K, typename V,V NO_TERMINAL = V()>
class TernarySearchTree {/*{{{*/
public:
    /** 
     * @brief constructor of TernarySearchTree
     */
    TernarySearchTree():level_count_(0){};
    /** 
     * @brief desconstructor
     */
    ~TernarySearchTree() { root_.clear();}
public:
    /** 
     * @brief Node type of the tree
     */
    typedef TSTnode<K,V,NO_TERMINAL> Node;
protected:
    /** 
     * @brief root node
     */
    Node root_; 
    /** 
     * @brief level count;
     */
    uint32_t level_count_;
public:
    /** 
     * @brief insert into tree, if exist, change value with new value
     * 
     * @param keys keys array
     * @param key_size keys size
     * @param v value
     * 
     * @return if insert success, return the terminal node, else return NULL
     */
    Node * insert(const K * keys, uint32_t key_size, const V& v);
    /** 
     * @brief find a value from key
     * 
     * @param keys which key to search
     * @param key_size size of key
     * 
     * @return value of key
     */
    Node * search(const K * keys, uint32_t key_size);
    /**
     * @brief free all nodes' space, except root_ node;
     */
    void clear() {
        root_.clear();
    }
};/*}}}*/

/*{{{*/ //functions of TSTnode
template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V,NO_TERMINAL> * TSTnode<K,V,NO_TERMINAL>::add_child(const K& k, const V& v) {/*{{{*/
    if(child_ == NULL) {
        return child_ = new TSTnode(k, v);
    } else {
        return child_->add_sibling(k, v);
    }
}/*}}}*/

template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V, NO_TERMINAL> * TSTnode<K,V, NO_TERMINAL>::add_sibling(const K& k, const V& v) {/*{{{*/
    TSTnode * current = this;
    while(current != NULL) {
        if(current->key_ == k) {
            break;
        } else if(current->key_ > k) {
            if(current->left_ == NULL) {
                current->left_ = new TSTnode(k, v);
                current = current->left_;
                break;
            } else {
                current = current->left_;
            }
        } else {
            if(current->right_ == NULL) {
                current->right_= new TSTnode(k, v);
                current = current->right_;
                break;
            } else {
                current = current->right_;
            }
        }
    }
    assert(current != NULL);
    return current;
}/*}}}*/

template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V,NO_TERMINAL> * TSTnode<K,V,NO_TERMINAL>::search_sibling(const K& key) {/*{{{*/
    TSTnode * current = this;
    while(current != NULL) {
        if(current->key_ == key) {
            break;
        } else if(current->key_ > key) {
            current = current->left_;
        } else {
            current = current->right_;
        }
    }
    return current;
}/*}}}*/

template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V,NO_TERMINAL> * TSTnode<K,V,NO_TERMINAL>::search_child(const K& key) {/*{{{*/
    if(child_ != NULL) {
        return child_->search_sibling(key);
    }
    return NULL;
}/*}}}*/
/*}}}*/

/*{{{*/ //functions of TernarySearchTree
template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V,NO_TERMINAL> * TernarySearchTree<K,V,NO_TERMINAL>::insert(const K * keys, uint32_t key_size, const V& v) {/*{{{*/
    Node * node = &root_;

    for(uint32_t i = 0; i < key_size; ++i) {
        node = node->add_child(keys[i]);
    }

    if(node->getValue() == NO_TERMINAL) {
        node->setValue(v);
    }

    if(level_count_ < key_size) {
        level_count_ = key_size;
    }

    return node;
}/*}}}*/

template <typename K, typename V, V NO_TERMINAL>
TSTnode<K,V,NO_TERMINAL> * TernarySearchTree<K,V,NO_TERMINAL>::search(const K * keys, uint32_t key_size) {/*{{{*/
    Node * node = &root_;

    for(uint32_t i = 0; i < key_size; ++i) {
        node = node->search_child(keys[i]);
        if(node == NULL) {
            break;
        }
    }

    return node;
}/*}}}*/
/*}}}*/
#endif

