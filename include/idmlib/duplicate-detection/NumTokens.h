/**
   @file NumTokens.h
   @author Jinglei
   @date 2009.11.24
 */
#ifndef __NUMTOKENS_H_
#define __NUMTOKENS_H_

#include "TernaryTree.h"
#include <types.h>
namespace sf1v5{
/**
 * @brief convert tokens into id instead of using hash function
 *
 * using a ternary search tree to store each string token's id,
 * all memebers are static, so need to run clear() function when
 * no need this class.
 */
class NumTokens {
private:
    /**
     * @brief ternery search tree
     */
    static TernarySearchTree<unsigned char, uint64_t, 0> tst_tree;
    /**
     * @brief id for a new member.
     */
    static uint64_t max_id;
public:
    /**
     * @brief get a string id
     *
     * @param key string to get id
     * @param key_size string's size
     *
     * @return token id
     */
    static uint64_t get_id(const unsigned char * key, uint32_t key_size);
    /**
     * @brief clear the memory, and set the max_id to 0
     */
    static void clear() {
        tst_tree.clear();
        max_id = 0;
    }
};
}
#endif

