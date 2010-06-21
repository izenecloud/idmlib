/**
   @file MEfficientLHT.h
   @author Yeogirl Yun
   @date 2009.11.24
 */
#ifndef MEfficientLHT_h
#define MEfficientLHT_h 1

#include <iostream>
#include <wiselib/hash/DHashTableConstants.h>
#include "MembershipMethods.h"

namespace wiselib {
  /******************************************************************************
	@class LHTElem is a wrapper class for KeyType and provide next elem
	               for List like behaviour within the MEfficientLHT.
  ******************************************************************************/
  template <class KeyType>
  class MELHTElem {
  public:
    KeyType key;
    MELHTElem<KeyType>* next;
    MELHTElem(const MELHTElem& source) : key(source.key), next(source.next) {}
    MELHTElem(const KeyType& d) : key(d), next(NULL) {}
    MELHTElem& operator=(const MELHTElem& source) {
      if (this != &source) {
	key= source.key;
	next = source.next;
      }
      return *this;
    }
    ~MELHTElem() {}
  };


  /******************************************************************************
	@class Segment is used to support dynamic growth/shrink property of
	             MEfficientLHT class.
  ******************************************************************************/
  template <class KeyType>
  class MELHTSegment
  {
  public:
    MELHTElem<KeyType> *seg[SEGMENT_SIZE];

    MELHTSegment() {
      for (int i = 0; i < SEGMENT_SIZE; i++)
	seg[i] = NULL;
    }

    ~MELHTSegment() {
      for (int i = 0; i < SEGMENT_SIZE; i++)
	delete seg[i];
    }

    void release() {
      for (int i = 0; i < SEGMENT_SIZE; i++) {
	delete seg[i];
	seg[i] = NULL;
      }
    }
  };


  /******************************************************************************
    @class: MEfficientLHT is based on Per-Ake Larson's work, Dynamic
                  Hash Tables and it is different from EfficientLHT in that
                  it supports dyamic membership method with slightely
                  improved efficiency

    Comments:   Use this class when DataType is the same as KeyType.
    Enhancements:
    History:
                  Yeogirl Yun                                   1/18/07
	 	    Initial Revision
  *******************************************************************************/
  template <class KeyType>
  class MEfficientLHT : public MembershipMethods<KeyType> {
  protected:
    int p; // next bucket to be split
    int maxp; // upper bound on p during this expansion
    int keycount; // number of records in the table
    int currentsize; // current number of buckets
    double minloadfctr, maxloadfctr; // lower and upper bound on the load factor
    MELHTSegment<KeyType>* directory[DIRECTORY_SIZE];

    virtual int hash(const KeyType& key) const;
    void expand_table();
    void contract_table(); //  { /* @@ yet to be implemented */ }
    void init();

  public:
    MEfficientLHT();
    MEfficientLHT(double minloadf, double maxloadf);

    virtual ~MEfficientLHT();
    void release();

    int num_items() const { return keycount; }
    int num_buckets() const { return currentsize; }

    bool find(const KeyType& key) const;
    bool insert(const KeyType& elem); // standard insert.
    bool del(const KeyType& key); // standard del

    void display(std::ostream& stream) const;
  };
  DEF_TEMPLATE_DISPLAY1(MEfficientLHT);


  /*************************************************************
	Description: Default constructor, Initialize member variables.
	Comments:
  *************************************************************/
  template <class KeyType>
  MEfficientLHT<KeyType>::MEfficientLHT()
  {
    init();
  }


  template <class KeyType>
  void MEfficientLHT<KeyType>::init()
  {
    p = 0;
    maxp = SEGMENT_SIZE;
    keycount = 0;
    currentsize = SEGMENT_SIZE;
    minloadfctr = MIN_LOAD_FACTOR;
    maxloadfctr = MAX_LOAD_FACTOR;
    directory[0] = new MELHTSegment<KeyType>();

    for (int i = 1; i < DIRECTORY_SIZE; i++)
      directory[i] = NULL;
  }


  /******************************************************************************
	Description: releases the MEfficientLHT object.
	Comments:
  ******************************************************************************/
  template <class KeyType>
  void MEfficientLHT<KeyType>::release()
  {
    for (int i = 0; i < DIRECTORY_SIZE; i++) {
      delete directory[i];
      directory[i] = NULL;
    }
    init();
  }


  /******************************************************************
	Description:
	Comments:
  *******************************************************************/
  template <class KeyType>
  MEfficientLHT<KeyType>::MEfficientLHT(double minloadf, double maxloadf)
  {
    init();

    minloadfctr = minloadf;
    maxloadfctr = maxloadf;
  }


  /******************************************************************
	Description:
	Comments:
  *******************************************************************/
  template <class KeyType>
  MEfficientLHT<KeyType>::~MEfficientLHT()
  {
    for (int i = 0; i < DIRECTORY_SIZE; i++)
      delete directory[i];
  }


  /*************************************************************
	Description: returns a hash value of a key
	Comments:
  *************************************************************/
  template <class KeyType>
  int MEfficientLHT<KeyType>::hash(const KeyType& key) const
  {
    ub4 h;
    int address;

    h = HashFunction<KeyType>::hashFN64(key);
    address = h & (maxp - 1);
    if (address < p)
      address = h & (2*maxp - 1);

    return address;
  }


  /*************************************************************
	Description: Expands the table
	Comments:
  **************************************************************/
  template <class KeyType>
  void MEfficientLHT<KeyType>::expand_table()
  {
    int newaddress, oldsegmentindex, newsegmentindex;
    MELHTSegment<KeyType>* oldsegment, *newsegment;
    MELHTElem<KeyType>* current, *previous; // for scanning down the old chain
    MELHTElem<KeyType>* lastofnew; // points to the last KeyType of the new chain

    // reached maximum size of address space? if so, just continue the chaining.
    if (maxp + p < DIRECTORY_SIZE * SEGMENT_SIZE) {
      // locate the bucket to be split
      oldsegment = directory[p/SEGMENT_SIZE];
      oldsegmentindex = p % SEGMENT_SIZE;

      // Expand address space, if necessary create a new MELHTSegment<KeyType>
      newaddress = maxp + p;
      newsegmentindex = newaddress % SEGMENT_SIZE;
      if (newsegmentindex == 0)
	directory[newaddress / SEGMENT_SIZE] = new MELHTSegment<KeyType>();
      newsegment = directory[newaddress / SEGMENT_SIZE];

      // adjust the state variables
      p = p + 1;
      if (p == maxp) {
	maxp = 2 * maxp;
	p = 0;
      }

      currentsize = currentsize + 1;

      // relocate records to the new bucket
      current = oldsegment->seg[oldsegmentindex];
      previous = NULL;
      lastofnew = NULL;
      newsegment->seg[newsegmentindex] = NULL;

      while (current != NULL) {
	if (hash(current->key) == newaddress) {
	  // attach it to the end of the new chain
	  if (lastofnew == NULL)
	    newsegment->seg[newsegmentindex] = current;
	  else
	    lastofnew->next = current;
	  if (previous == NULL)
	    oldsegment->seg[oldsegmentindex] = current->next;
	  else
	    previous->next = current->next;
	  lastofnew = current;
	  current = current->next;
	  lastofnew->next = NULL;
	}
	else {
	  // leave it on the old chain
	  previous = current;
	  current = current->next;
	}
      }
    }
  }

  /*************************************************************
	Description: Contracts the table, exactly the opposite of expand_table().
	Comments:
  **************************************************************/
  template <class KeyType>
  void MEfficientLHT<KeyType>::contract_table()
  {
    int oldsegmentindex, newsegmentindex;
    MELHTSegment<KeyType>* oldsegment, *newsegment;
    MELHTElem<KeyType>* current, *previous; // for scanning down the new/current chain

    // Is the table contractable or has more than one segment
    if (currentsize > SEGMENT_SIZE) { // there is a bucket to shrink.
      // locate the bucket to free
      currentsize--;

      oldsegment = directory[currentsize/SEGMENT_SIZE];
      oldsegmentindex = currentsize % SEGMENT_SIZE;

      // adjust the state variables
      p = p - 1;
      if (p < 0) {
	maxp = maxp/2;
	p = maxp - 1;
      }

      newsegment = directory[p/SEGMENT_SIZE];
      newsegmentindex = p % SEGMENT_SIZE;

      // relocate records to the new bucket
      current = newsegment->seg[newsegmentindex];

      // sacn down the end of the current where the additonal records
      // will be attached to.
      previous = current;
      while (current != NULL) {
	previous = current;
	current = current->next;
      }
      // attach the chain of records to the end of the new bucket
      if (previous) {
	previous->next = oldsegment->seg[oldsegmentindex];
      }
      else
	newsegment->seg[newsegmentindex] = oldsegment->seg[oldsegmentindex];

      oldsegment->seg[oldsegmentindex] = NULL;

      // if necessary delete the old MELHTSegment<KeyType>
      if (oldsegmentindex == 0) {
	delete directory[currentsize/SEGMENT_SIZE];
	directory[currentsize/SEGMENT_SIZE] = NULL;
      }
    }
  }


  /*************************************************************
	Description: returns true if Key is found.
	Comments:
  *************************************************************/
  template <class KeyType>
  bool MEfficientLHT<KeyType>::find(const KeyType& key) const
  {
    int address = hash(key);
    MELHTElem<KeyType>* elem = directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE]; // first on chain
    while (elem != NULL) {
      if (key == elem->key) // found!
	return true;
      else
	elem = elem->next;
    }

    // not found
    return false;
  }


  /*************************************************************
    Description:
    Comments:
  *************************************************************/
  template <class KeyType>
  bool MEfficientLHT<KeyType>::insert(const KeyType& elem)
  {
    if (maxloadfctr * currentsize <= keycount)
      expand_table(); // due for expanding table

    const KeyType& key = elem;
    int address = hash(key);
    MELHTSegment<KeyType>* currentMELHTSegment = directory[address/SEGMENT_SIZE];
    int segIndex = address % SEGMENT_SIZE;
    MELHTElem<KeyType>* e = currentMELHTSegment->seg[segIndex]; // first on chain
    if (e == NULL)
      currentMELHTSegment->seg[segIndex] = new MELHTElem<KeyType>(elem);
    else {
      while (e->next != NULL) {
	if (e->key == key) // duplicate data
	  return false;
	e = e->next; // go to the end of the chain
      }
      if (e->key == key)
	return false;
      e->next = new MELHTElem<KeyType>(elem);
    }

    keycount++; // increment key count by one.
    return true;
  }


  /*************************************************************
	Description: Deletes an KeyType.
	Comments: This method takes the ownership and deletes the data itself
	            when successful.
  *************************************************************/
  template <class KeyType>
  bool MEfficientLHT<KeyType>::del(const KeyType& key)
  {

    if (minloadfctr * currentsize > keycount)
      contract_table();

    int address = hash(key);
    MELHTElem<KeyType>* elem = directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE]; // first on chain
    MELHTElem<KeyType>* prev = elem;

    while (elem != NULL) {
      if (key == elem->key) { // found!
	if (prev == elem)  //
	  directory[address/SEGMENT_SIZE]->seg[address % SEGMENT_SIZE] = prev->next;
	else
	  prev->next = elem->next;

	delete elem;
	keycount--;

	return true;
      }
      else {
	prev = elem;
	elem = elem->next;
      }
    }

    // not found
    return false;
  }


  /******************************************************************
	Description:
	Comments:
  *******************************************************************/
  template <class KeyType>
  void MEfficientLHT<KeyType>::display(std::ostream& stream) const
  {
    stream << "Member variables: " << std::endl;
    stream << "the number of buckets: " << currentsize << std::endl;
    stream << "the number of records: " << keycount << std::endl;
    stream << "max load factor: " << maxloadfctr << std::endl;
    stream << "min load factor: " << minloadfctr << std::endl;
    stream << "max p value: " << maxp << std::endl;
    stream << "current p value: " << p << std::endl;
  }
};
#endif
