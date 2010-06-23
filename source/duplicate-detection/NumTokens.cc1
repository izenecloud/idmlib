#include <idmlib/duplicate-detection/NumTokens.h>
namespace sf1v5
{

  TernarySearchTree<unsigned char, uint64_t, 0> NumTokens::tst_tree;

  uint64_t NumTokens::max_id = 1;
  uint64_t NumTokens::get_id(const unsigned char * key, uint32_t key_size) {
    TSTnode<unsigned char, uint64_t, 0> * node = tst_tree.insert(key, key_size, max_id);
    if(node->getValue() == max_id) {
      ++max_id;
    }
    return node->getValue();
  }

}

