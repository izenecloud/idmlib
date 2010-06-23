// All restrictions are applied. See COPYRIGHT.
// This is an include file.  For a full description of the class and
// functions, see the file name with a "c" suffix.

#ifndef _UniversalHashTable_h
#define _UniversalHashTable_h 1

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "HashTable.h"
#include "RCharArray.h"
namespace wiselib {
  // This class implements hash table based on Universal hash function.
  template <class KeyType, class DataType>
  class UniversalHashTable :  public HashTable<KeyType, DataType> {
  protected :
    RandCharArray randomData;
    virtual int hash(const KeyType& keyValue) const;

  public :
    UniversalHashTable(int estimatedNumElement);
    UniversalHashTable(int estimatedNumElement, unsigned int seed);
    UniversalHashTable(const UniversalHashTable& source);
    virtual ~UniversalHashTable() {}

    // provides persistence through the following two interfaces.
    void serialize(const YString& file) const;
    void serialize(int fd) const;
    void deserialize(const YString& file);
    void deserialize(int fd);
  };
  // MLC++ - Machine Learning in -*- C++ -*-
  // See Descrip.txt for terms and conditions relating to use and distribution.

  /*****************************************************************************
  Description  : This is a class for an Universal hash table. It derives from
                    HashTable<KeyType, DataType> base class and
		    defines Universal hash function. Universal hashing
		    function is chosen at run time from a collection
		    of such functions and the expected number of
		    collisions involving a particular key is less than
		    1. For details, refer to 'Introduction to
		    Algorithms', Cormen et al, p230-p231, MIT press.
  Assumptions  : hash() function assumes KeyType can be cast to char*
                    (without loss of semantics, i.e. that key does not
		    contain pointers within it). In other cases, new
		    hash() function may be defined  properly in any
		    derived classes from this class.
		 The KeyType should be a class which has a size()
		    member function returning the length of key in bytes.
		 The KeyType should be a class which has a c_str()
		    member function returning the object image to be hashed.
  Comments     : The size of hash table, if it is less than 2^8+1, is
                    extended to 2^8+1. This is a necessary condition for
		    universal hashing functions to work properly.
  Complexity   : hash() takes O(m), where m is the length of keyValue.
  Enhancements :
  History      : YeoGirl Yun                                      4/17/94
                   initial revision
  *****************************************************************************/


  /******************************************************************************
  Description  : Copy constructor.
                 It initializes randomData and pass the source argument over
      		   to base class' copy constructor.
		 It should not be called by any derived class when it defined
		   its own hash function because within this copy constructor
		   UniversalHashTable<>::hash() would be used.
  Comments     :
  ******************************************************************************/
  template <class KeyType, class DataType>
  UniversalHashTable<KeyType, DataType>::UniversalHashTable
  (const UniversalHashTable<KeyType, DataType>& source)
    : HashTable<KeyType, DataType>(std::max(source.num_items(),
					    (int)(source.size()*LOAD_FACTOR))),
      randomData(0)
  {
    PixHT<KeyType, DataType> pix;
    for (source.set_pix(pix); pix; pix++)
      insert(*source[pix]);
  }


  /******************************************************************************
  Description  : Constructor with estimated number arguments.
  Comments     : The size samller than (UCHAR_MAX + 1) is extened to
                   the size of UCHAR_MAX + 1 without any error
		   message. This is to make hash table size greater
		   than 2^8. This will work even in the case
		   LOAD_FACTOR is 1.
  ******************************************************************************/
  template <class KeyType, class DataType>
  UniversalHashTable<KeyType, DataType> :: UniversalHashTable(int estimatedNum)
    : HashTable<KeyType, DataType>(std::max(UCHAR_MAX + 1, estimatedNum)),
      randomData(0)
  {
  }


  /******************************************************************************
  Description  : Constructor with estimated number arguments and seed for
                   RandCharArray.
  Comments     : The size samller than (UCHAR_MAX + 1) is extened to
                   the size of UCHAR_MAX + 1 without any error
		   message. This is to make hash table size greater
		   than 2^8. This will work even in the case
		   LOAD_FACTOR is 1.
  ******************************************************************************/
  template <class KeyType, class DataType>
  UniversalHashTable<KeyType, DataType>::UniversalHashTable(int estimatedNum,
							    unsigned int seed)
    : HashTable<KeyType, DataType>(std::max(UCHAR_MAX + 1, estimatedNum)),
      randomData(0, seed)
  {
  }


  /******************************************************************************
  Description  : Universal hash function. It assumes the length of
                   keyValue is greater than 0( >= 1 ).
  Comments     : It requires the size of hash table should be greater
                   than 2^8, or 256.
  ******************************************************************************/
  template <class KeyType, class DataType>
  int UniversalHashTable<KeyType, DataType>:: hash(const KeyType& keyValue) const
  {
    const unsigned char* ptr = (const unsigned char*) keyValue.c_str();
    long hashValue = 0;

    // note : To iterate from the end helps speed execution time since
    //        it obviates unnecesary iterative extensions of array.
    //        For details, see the DynamicArray.c file.
    for(int i = keyValue.size()-1; i >= 0 ; i-- )
      hashValue += ptr[i] * // constness cast away : randomData, being a
	// dynamic array, does not allow const function.
	((UniversalHashTable<KeyType, DataType> *)this)->
	randomData.index(i);

    return hashValue % HashTable<KeyType, DataType>::size();
  }



  /*
    Description : PersistentUniversalHashTable supplement UniversalHashTable
    with the facility of persistent interface. In other words,
    at every update, delete, insert of hash items, the
    result is reflected to a persistent storage indicated by
    an ostream object given at construction time.

    It also provides the ability to initialize the self object
    by reading the presitent storage and maintains the same
    semantics when the object was stored. The same semantics
    means the same hash items are stored in the hash table
    and are accessible by the same key to each item.

    Assumptions : DataType must have the following additional members on
    top of requirements of UniversalHashTable.
    1. read_image(istream& stream);
    2. write_image(ostream& stream);
    Comments    : As of June 27, insertion, update, and deletion is not
    reflected immediately. User must call write_iamge() whenever
    he/she wants to save the data.
    Enhancemnets: Implement immediate update, insertion, and deletion.
    History     : Yeogirl Yun                                    June 27, 1996
    Initial Revision (.h.c)
  */


  /*****************************************************************************
  Description : Write all the elements in the hash table to the given stream.
                The stream will be read into the hash table with the method
		  deserialize(istream&).
  Comments    : Simple file format. At the begining of the file, the number
                  of data elements are written in integer (4 bytes).
		  Subsequent writes of data elements are defiend by
		  data element's serialize(ostream&) method.
  *****************************************************************************/
  template <class KeyType, class DataType>
  void UniversalHashTable<KeyType, DataType>::serialize(const YString& file) const
  {
    int fd = open(file.as_const_char(), O_WRONLY|O_CREAT|O_TRUNC, 0x755);
    serialize(fd);
    close(fd);
  }

  template <class KeyType, class DataType>
  void UniversalHashTable<KeyType, DataType>::serialize(int fd) const
  {
    int numOfItems = HashTable<KeyType, DataType>::num_items();
    write(fd, (char *)&numOfItems, sizeof(int));

    PixHT<KeyType, DataType> pix;
    int k = 0;
    for (set_pix(pix); pix; pix++,k++) {
      (*this)[pix]->serialize(fd);
    }
  }


  /*****************************************************************************
  Description : Read data elements from the given stream. The number of
                  data elements to be read is determined by the first
		  4 bytes being interpreted as an integer value.
  Comments    :
  *****************************************************************************/
  template <class KeyType, class DataType>
  void UniversalHashTable<KeyType, DataType>::deserialize(const YString& file)
  {
    int fd = open(file.as_const_char(), O_RDONLY);
    deserialize(fd);
    close(fd);
  }


  template <class KeyType, class DataType>
  void UniversalHashTable<KeyType, DataType>::deserialize(int fd)
  {
    int numToRead;
    read(fd, (char *)&numToRead, sizeof(int));

    for (int i = 0; i < numToRead; i++) {
      DataType temp;
      temp.deserialize(fd);
      insert(temp);
    }
  }

};
#endif




