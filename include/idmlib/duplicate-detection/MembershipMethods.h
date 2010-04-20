/**
   @file MembershipMethods.h
   @author Yeogirl Yun
   @date 2009.11.24
 */
#ifndef MembershipMethods_h
#define MembershipMethods_h

#include <wiselib/basics.h>
#include <util/hashFunction.h>
namespace wiselib {
  const int PRIME = 1048583; // often used in computing hash value. Big prime number.

  template <class KeyType>
  class MembershipMethods {
  public:
    MembershipMethods() {}
    virtual ~MembershipMethods() {}

    virtual void release() = 0; // release the data in this access method.
    virtual int num_items() const = 0; // returns the number of items in this access method.
    virtual bool find(const KeyType& key) const = 0;
    virtual bool insert(const KeyType& data) = 0; // if duplicate return false else return true
    virtual bool del(const KeyType& key) = 0; // if no data return false, elese successful delete.
  };
};
#endif
