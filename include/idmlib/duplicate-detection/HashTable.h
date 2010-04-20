/**
   @file HashTable.h
   @author Jinglei
   @date 2009.11.24
   @brief All restrictions are applied. See COPYRIGHT.
   This is an include file.  For a full description of the class and
   functions, see the file name with a "c" suffix.
*/
#ifndef HashTable_h
#define HashTable_h

//#include <wiselib/basics.h>
#include <wiselib/DArray.h>
#include <wiselib/YString.h>
#include <wiselib/hash/AccessMethods.h>
#include <iostream>
#include <iomanip>


namespace wiselib {

#define LOAD_FACTOR 2

  template <class KeyType, class DataType> class HashTable;

  template <class KeyType, class DataType>
      class PixHT {
      private:
          friend class HashTable<KeyType, DataType>;

          DynamicArray< DynamicArray<DataType> > *_tablePtr;
          /**
           * bin where the DynamicArray object is stored.
           */
          int _hashBin;
          int _curPix;
          /**
           * true if it can be used to retrieve items.
           */
          bool _isValid;

          void copy(const PixHT<KeyType, DataType>& aPixHT);
          void find_next();

      public:
          PixHT() : _tablePtr(NULL), _hashBin(-1), _isValid(false) {}
          PixHT(const HashTable<KeyType, DataType>& hashTable, int pos = 0);
          PixHT(const PixHT<KeyType, DataType>& aPixHT) { copy(aPixHT); }
          PixHT<KeyType, DataType>& operator=(const PixHT<KeyType, DataType>& aPixHT);

          /**
           * automatic conversion from PixHT<KeyType, DataType> to int.
           */
          operator bool() const { return _isValid; }
          void operator++();
          void operator++(int) { operator++(); }

          const KeyType& get_key() const { return ((*_tablePtr)[_hashBin])[_curPix].get_key(); }
          const DataType& get_data() const { return ((*_tablePtr)[_hashBin])[_curPix]; }
      };



  /**
   * @brief This is an ABC for all hash table related classes.
   *
   * hash() is a pure virtual function which must be defined in any
   * derived classes.

   * Dynamic Hash Tables (meaning table size growing depending on the size of input)
   * are directly derived from AccessMethods.
   * When the total number of keys are known, use PrimeHashTable
   * (UniversalHashTable is quite slow for most of the input cases compared to PrimeHash),
   * otherwise use any one of ExtendibleHash, ExtendibleHashLSB, or LinearHashTable.

   * The performance a particular hash table depends on the quality of hash key
   * and input pattern and usage.
   * Actual testing should be peformed before choosing any of the three dynamic
   * hash table classes.
   */

  template <class KeyType, class DataType>
      class HashTable  : public wiselib::hashing_methods::AccessMethods<KeyType, DataType> {
          friend class PixHT<KeyType, DataType>;

          DynamicArray< DynamicArray< DataType > > table;
          /**
           * stores actual number of data items in the hash table.
           */
          int numItems;
          /**
           * note: this is different from the size of the table.
           */

          HashTable(const HashTable&) : table(1) {
              std::cerr << "Can't use copy constructor." << std::endl;
              abort();
          }

          void enlarge_table();
      protected :
          virtual int hash(const KeyType& key) const = 0;

      public :
          HashTable(int estimatedNum);
          virtual ~HashTable() {}

          /**
           * returns the size of the hash table
           */
          virtual int size()  const { return table.size(); }
          /**
           * returns the actual number of data
           */
          virtual int num_items() const { return numItems; }

          virtual bool insert(const DataType&);
          /**
           * this update function needs deep copy to perform a delete.
           */
          virtual void update(DataType d) { del(d.get_key()); insert(d); }
          /**
           * returns NULL, if not found.
           */
          virtual DataType* find(const KeyType&);
          virtual const DataType* find(const KeyType&) const;
          virtual bool find(const DataType& data, int hashBin) const;
          virtual bool del(const KeyType&);

          /**
           * PixHT interface.
           */
          virtual const DataType* operator[](const PixHT<KeyType, DataType>& pix) const;
          virtual DataType* operator[](const PixHT<KeyType, DataType>& pix);
          virtual bool del(PixHT<KeyType, DataType>& pix);
          virtual void set_pix(PixHT<KeyType, DataType>& pix, int pos = 0) const;

          virtual void display(std::ostream& stream = std::cout) const;
          virtual void stats(const YString& description) const;
          virtual void release(); //
      };

  template <class KeyType, class DataType>
      std::ostream& operator<<(std::ostream& strm, const HashTable<KeyType, DataType>& src)
      {
          src.display(strm);
          return strm;
      }

  //-*-C++-*-
  /******************************************************************************
Description  : HashTable is an ABC which implements a general hash table data
structure for distinct keys, with key type and data
type templated. Collision is handled by chaining.
Assumptions  : Templated KeyType is assumed to provide operator==()
which must compare deep equality between keys.
Templated KeyType is assumed to provide operator<<()
which is used for diagnostic message.
Templated DataType is assumed to provide operator=()
which is needed for List, but since DataType
will usually be a pointer, it will exist by
default.
Templated DataType is assumed to provide
contains_key(const KeyType&) which is needed to
compare between key and data.
Templated DataType is assumed to provide const KeyType&
get_key() which returns KeyType.
Comments     : The hash function is undefined.
Complexity   : del() and find() takes O(LOAD_FACTOR + h), where
LOAD_FACTOR is a load factor of the hash table
and h is the time hash() function takes. For
details, refer to 'Introduction to Algorithms',
Cormen, Leiserson, and Rivest, p224-p225, MIT press.
Enhancements : For space and speed efficiency, dynamic feature is
desirable. With dynamic feature, hash table may
shrink or grow depending on data loads and key
distributions.
History      : Yeogirl Yun                                       2/14/96
Replaced DblList with List.
Added PixHT class and provided iterations.
Changed LOAD_FACTOR to be a constant instead of
static data member.
Yeogirl Yun
Added a copy constructor and display().         9/06/94
Yeogirl Yun
Added ok_members().                             9/02/94
Yeogirl Yun
Initial revision(.c .h)               	   4/26/94
   ******************************************************************************/

  const double FULL_LOAD_RATE = 0.75;
  /*****************************************************************************
Description : Find next item in the HashTable object given current hashBin
and pix. hashBin must be valid before this call.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void PixHT<KeyType, DataType>::find_next() {
          const DynamicArray< DynamicArray<DataType> >& table = *_tablePtr;

          while (true) {
              if (_curPix < 0) {
                  _hashBin++;
                  if (_hashBin >= table.size()) { // no more items.
                      _isValid = false;
                      break;
                  }
                  _curPix = table[_hashBin].high();
              } else {
                  _curPix--;
              }

              if (_curPix >= 0) {
                  break;
              }
          }
      }


  /*****************************************************************************
Description : Copy a PixHT object.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void PixHT<KeyType, DataType>::copy(const PixHT<KeyType, DataType>& aPixHT) {
          _tablePtr = aPixHT._tablePtr;
          _hashBin = aPixHT._hashBin;
          _curPix = aPixHT._curPix;
          _isValid = aPixHT._isValid;
      }


  /*****************************************************************************
Description : Initialize with a HashTable object. illegal pos argument
makes pix invalid.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      PixHT<KeyType, DataType>::PixHT(const HashTable<KeyType, DataType>& hashTable, int pos)
      : _tablePtr(NULL), _hashBin(-1), _isValid(false) {
          if (pos < 0 || pos >= hashTable.num_items()) {
              return; // illegal pos argument.
          }

          _tablePtr = (DynamicArray< DynamicArray<DataType> >*)&hashTable.table;
          _hashBin = 0;
          _curPix = (*_tablePtr)[_hashBin].high();
          _isValid = true;

          if (_curPix < 0) {
              find_next();
          }

          for (int i = 0; i < pos; i++) {
              (*this)++;
          }
      }




  /*****************************************************************************
Description  : Assigns a new value of an PixHT<KeyType, DataType> object
to itself.
Comments     :
   *****************************************************************************/
  template <class KeyType, class DataType>
      PixHT<KeyType, DataType>& PixHT<KeyType, DataType>::operator=(const PixHT<KeyType, DataType>& aPixHT)
      {
          if (this != &aPixHT) {
              copy(aPixHT);
          }

          return *this;
      }


  /*****************************************************************************
Description  : Advance the pointers of this object by one only if
pix is in legal state.
If the current pix becomes invalid,
_isValid is set to false.
Comments     :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void PixHT<KeyType, DataType>::operator++() {
          if (_isValid) {
              find_next();
          }
      }


  /******************************************************************************
Description  : Constructor with estimated table size argument.
Actual size loaded is estimatedNumElement/LOAD_FACTOR
Comments     :
   ******************************************************************************/

  template <class KeyType, class DataType>
      HashTable<KeyType, DataType>::HashTable(int estimatedNumElement) :
          table(estimatedNumElement)
    {
        numItems = 0;
    }



  /*****************************************************************************
Description: Enlarges the table and rehashes everything with a different
table size.
Comments   :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void HashTable<KeyType, DataType>::enlarge_table()
      {
          DynamicArray<DataType> tempStore(num_items());

          // first copy over all the data elements to tempStore.
          PixHT<KeyType, DataType> pix;
          int i = 0;
          for (set_pix(pix); pix; pix++, i++) {
              tempStore[i] = *(*this)[pix];
          }

          // now delelte all the hashed items.
          for (i = 0; i < tempStore.size(); i++)
              del(tempStore[i].get_key());

          // now increase the hash table twice.
          (void)(DynamicArray<DataType>&)table.index((int)(table.size() * 1.5));  // this will make it grow

          // now rehash everything.
          for (i = 0; i < tempStore.size(); i++)
              insert(tempStore[i]);
      }


  /******************************************************************************
Description  : Inserts a data item into hash table.
An attempt to insert duplicate data is regarded as a
non fatal error and return false.
Comments     :
   ******************************************************************************/
  template <class KeyType, class DataType>
      bool HashTable<KeyType, DataType>::insert(const DataType& dataValue)
      {
          if (num_items() > table.size())
              enlarge_table();

          KeyType keyValue = dataValue.get_key();
          int hashBin = hash(keyValue);

          // check if there is a duplicate data.
          if( find(dataValue, hashBin) )
              return false;

          // just add this to the end.
          table[hashBin][table[hashBin].size()] = dataValue;

          numItems++;
          return true;

          // statistics.
      }



  /******************************************************************************
Description   : Removes a data item from hash table. Returns True
iff found.
Comments      :
   ******************************************************************************/
  template <class KeyType, class DataType>
      bool HashTable<KeyType, DataType>::del(const KeyType& keyValue)
      {
          DynamicArray<DataType> &DynamicArrayItems = table[hash(keyValue)];
          for (int i = 0; i < DynamicArrayItems.size(); i++) {
              if (DynamicArrayItems[i].get_key() == keyValue) {
                  DynamicArrayItems.remove(i); // delete the data item.
                  numItems--;
                  return true;
              }
          }

          return false;
      }




  /******************************************************************************
Description  : Find the data item corresponding to keyValue.
Returns a valid pointer to the data item, if found.
Otherwise returns NULL.
Comments     : Note that there are two versions of find(). This is
for non-const object.
   ******************************************************************************/
  template <class KeyType, class DataType>
      DataType* HashTable<KeyType, DataType>::find(const KeyType& keyValue)
      {
          DynamicArray<DataType> &DynamicArrayItems = table[hash(keyValue)];
          for (int i = 0; i < DynamicArrayItems.size(); i++) {
              if (DynamicArrayItems[i].get_key() == keyValue) {
                  // statistics.
                  // numFinds++;
                  return &(DynamicArrayItems[i]);
              }
          }
          return NULL;
      }


  /******************************************************************************
Description  : Find the data item corresponding to keyValue.
Returns a valid pointer(const) to the data item, if found.
Otherwise returns NULL.
Comments     : Note that there are two versions of find(). This is
for const object.
   ******************************************************************************/
  template <class KeyType, class DataType>
      const DataType* HashTable<KeyType, DataType>::find(const KeyType&
              keyValue) const

      {
          const DynamicArray<DataType> &DynamicArrayItems = table[hash(keyValue)];
          for (int i = 0; i < DynamicArrayItems.size(); i++) {
              if (DynamicArrayItems[i].get_key() == keyValue ) {
                  // for statistics cast const into non-const
                  // ((HashTable<KeyType, DataType>*)this)->numFinds++;
                  return &(DynamicArrayItems[i]);
              }
          }
          return NULL;
      }


  /******************************************************************************
Description  : Find the data item in the given hash bin.
Returns true iff the given data item is found in the
hash bin.
Comments     :
   ******************************************************************************/
  template <class KeyType, class DataType>
      bool HashTable<KeyType, DataType>::find(const DataType& data, int hashBin)
      const
      {
          const DynamicArray<DataType> &DynamicArrayItems = table[hashBin];
          const KeyType& keyValue = data.get_key();
          for (int i = 0; i < DynamicArrayItems.size(); i++) {
              if (DynamicArrayItems[i].get_key() == keyValue) {
                  // ((HashTable<KeyType, DataType>*)this)->numFinds++;
                  return true;
              }
          }
          return false;
      }



  /*****************************************************************************
Description : Displays the contents of hash table. The sequence of data
elements to be displayed is the order of buckets. Lower
numbered bucket will precede the higher one. The format
will be the sequence number followed by the bucket number
and the contents of the bucket. If the bucket has no data
elements is will be skipped.
Comments    : At each bucket, it calls the DynamicArray::display(), since
bucket is a DynamicArray.
   *****************************************************************************/
  template <class KeyType, class DataType>
      void HashTable<KeyType, DataType>::display(std::ostream& stream) const
      {
          int count = 0;
          for (int i = 0; i < size(); i++)
              if (table[i].size() > 0) {
                  stream << "\nThe "<< count++ <<"th (bucket number : "
                      << i << ")" << std::endl;
                  table[i].display(stream);
              }
      }



  /******************************************************************************
Description  : Gives various statistics for the hash table.
Comments     : Note that this is not regular member function in the
sense that it may not be compiled in FAST mode.
   ******************************************************************************/
  template <class KeyType, class DataType>
      void HashTable<KeyType, DataType>::stats(const YString& /*description*/) const
      {
      }



  /*****************************************************************************
Description : Returns the pointer to DataType given a pix. Aborts if the pix
is invalid.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      const DataType* HashTable<KeyType, DataType>::operator[](const PixHT<KeyType, DataType>& pix) const
      {
          if (!pix) {
              std::cerr << "HashTable<KeyType, DataType>::operator[] const : invalid pix" <<
                  std::endl;
              abort();
          }

          return &(((*pix._tablePtr)[pix._hashBin])[pix._curPix]);
      }

  template <class KeyType, class DataType>
      DataType* HashTable<KeyType, DataType>::operator[](const PixHT<KeyType, DataType>& pix)
      {
          if (!pix) {
              std::cerr << "HashTable<KeyType, DataType>::operator[] : invalid pix" <<
                  std::endl;
              abort();
          }

          return &(((*pix._tablePtr)[pix._hashBin])[pix._curPix]);
      }



  /*****************************************************************************
Description : Deletes a DataType object and move PixHT to the next one.
Returns always true since PixHT should be valid.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      bool HashTable<KeyType, DataType>::del(PixHT<KeyType, DataType>& pix)
      {
          if (!pix) {
              std::cerr << "HashTable<KeyType, DataType>::del() : invalid pix" <<
                                                                  std::endl;
              abort();
          }

          (*pix._tablePtr)[pix._hashBin].remove(pix._curPix);

          if ((*pix._tablePtr)[pix._hashBin].size() <= pix._curPix)
              pix.find_next();

          numItems--;
          return true;
      }



  /*****************************************************************************
Description : Set the pix in pos'th item in the hash table.
Comments    :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void HashTable<KeyType, DataType>::set_pix(PixHT<KeyType, DataType>& pix,
              int pos) const
      {
          pix = PixHT<KeyType, DataType>(*this, pos);
      }



  /*****************************************************************************
Description:
Comments   :
   *****************************************************************************/
  template <class KeyType, class DataType>
      void HashTable<KeyType, DataType>::release()
      {
          for (int i = 0; i < table.size(); i++)
              table[i].truncate(0);
          numItems = 0;
      }

};
#endif



