/**
   @file PrimeHashTable.h: interface for the PrimeHashTable class.
   @author Jinglei
   @date   2009.11.24
**/

#if !defined(AFX_PRIMEHASHTABLE_H__4D67A11C_D741_4210_AF69_EB430245F1AB__INCLUDED_)
#define AFX_PRIMEHASHTABLE_H__4D67A11C_D741_4210_AF69_EB430245F1AB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "UHashTable.h"


namespace wiselib {
  //@@ there seems to be a bug in this class.
  //   using this class on Multiset results in an extremely slow performance
  //   compared UniversalHashTable

  template <class KeyType, class DataType>
    class PrimeHashTable : public HashTable<KeyType, DataType>
    {
    private:
      // could be implemented later.
      PrimeHashTable(const PrimeHashTable& source) {}   // no copy constructor.
      PrimeHashTable& operator=(const PrimeHashTable& source) { return *this; }

    protected:
      virtual int hash(const KeyType& keyValue) const;

    public:
      PrimeHashTable(int estimatedNum);
      virtual ~PrimeHashTable() {}
    };


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
    PrimeHashTable<KeyType, DataType> :: PrimeHashTable
    (int estimatedNum) : HashTable<KeyType, DataType>(std::max(UCHAR_MAX + 1, estimatedNum))
    {
    }

  /******************************************************************************
	Description: Computing hash value using big PRIME number..known to be universal.
	Comments:
  ******************************************************************************/
  template <class KeyType, class DataType>
    inline int PrimeHashTable<KeyType, DataType>::hash(const KeyType& key) const
    {
      return (HashFunction<KeyType>::convert_key(key) % HashTable<KeyType, DataType>::size());
    }

};
#endif // !defined(AFX_PRIMEHASHTABLE_H__4D67A11C_D741_4210_AF69_EB430245F1AB__INCLUDED_)
