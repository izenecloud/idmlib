/**
   @file NearDuplicatePair.h
   @author Jinglei
   @date 2009.11.24
 */
#ifndef NearDuplicatePair_h
#define NearDuplicatePair_h 1

#include "NearDuplicateSignature.h"
#include "RandomProjection.h"
#include "TypeKey.h"


namespace sf1v5{
/**
 * @brief store the dup doc pair and the score.
 */
class NearDuplicatePair {
public:
  wiselib::TypeKey<wiselib::ub2> neardupScore; // from 0 to 2^16-1. the higher the better.
  NearDuplicateSignature* sigFirst;
  NearDuplicateSignature* sigSecond;

  inline NearDuplicatePair() : neardupScore(0), sigFirst(NULL), sigSecond(NULL) {}
  inline ~NearDuplicatePair() {}

  inline const wiselib::TypeKey<wiselib::ub2>& get_key() const { return neardupScore; }
  inline void display(ostream& stream) const {
    stream << "{ score: " << neardupScore << " <"
	   << sigFirst->get_doc_string(sigFirst->get_doc_id())
	   << ", "
	   << sigSecond->get_doc_string(sigSecond->get_doc_id()) << "> }";
  }
  // for DynamicArray interface
  bool operator<(const NearDuplicatePair& src) const {
    return neardupScore < src.neardupScore;
  }
};

DEF_DISPLAY(NearDuplicatePair);
}
#endif
