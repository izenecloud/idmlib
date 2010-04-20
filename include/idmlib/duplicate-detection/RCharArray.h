// All restrictions are applied. See COPYRIGHT.
// This is an include file.  For a full description of the class and
// functions, see the file name with a "c" suffix.

#ifndef RandCharArray_h
#define RandCharArray_h

#include <wiselib/basics.h>
#include <wiselib/DArray.h>
#include <wiselib/MRandom.h>


namespace wiselib {
  // this is a dynamic array which is solely used for universal hashing.

  class RandCharArray : public DynamicArray<unsigned char> {
    RandCharArray(const RandCharArray&) : DynamicArray<unsigned char>(1) {
//      cerr << "Can't use copy constructor" << endl;
      abort();
    }
    MRandom randomVal;
  protected:
    virtual void extend_array(int i);
    public :
      RandCharArray(int size);
    RandCharArray(int size, unsigned int seed);
    virtual ~RandCharArray() {}
  };
};
#endif

