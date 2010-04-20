#ifndef TRIE_h
#define TRIE_h 1

#include <wiselib/hash/AccessMethods.h>
#include <wiselib/Array.h>
#include <wiselib/DArray.h>
//using namespace wiselib::hashing_methods;
namespace wiselib {
  typedef void (*ManipFuncType)(void* node);

  template <class DataType>
    class TrieNode {
    public:
    TrieNode** branch;
    DataType** data;

    TrieNode(int bs) {
      branch = new TrieNode<DataType>*[bs];
      data = new DataType*[bs];
      int i;
      for (i = 0; i < bs; i++) {
	branch[i] = NULL;
	data[i] = NULL;
      }
    }
    ~TrieNode() { delete[] branch; delete[] data; }
  };


  template <class DataType>
    class Trie : public wiselib::hashing_methods::AccessMethods<string, DataType> {
    private:
    int branchSize;
    TrieNode<DataType>* root;
    int num;

    void release(TrieNode<DataType>* t);
    void insert(TrieNode<DataType>* t, DataType* d, const DataType& data, int depth);
    void in_order(TrieNode<DataType>* node, ManipFuncType manip);
    static const int defaultTrieBS = 128;

    public:
    Trie(int bs = defaultTrieBS)
      : branchSize(bs), root(NULL), num(0) {}

    ~Trie() { release(root); }
    void release() { release(root); num = 0; root = NULL; }

    int num_items() const { return num; }

    DataType* find(const string& key);
    const DataType* find(const string& key) const { return (const DataType*)((Trie<DataType>*)(this))->find(key); }
    bool insert(const DataType& data);
    bool del(const string& key);
    void traverse(ManipFuncType manip);
  };


  /******************************************************************************
	Description: Deletes all of the elements in the Trie.
	Comments   :
  ******************************************************************************/
  template <class DataType>
    void Trie<DataType>::release(TrieNode<DataType>* t)
    {
      DynamicArray<TrieNode<DataType>*> nodeStack;
      DynamicArray<int> indexStack;
      int sp = 0;
      int i;

    release_label_1:
      if (t) {
	for (i = 0; i < branchSize; i++) {
	  if (t->branch[i]) {
	    nodeStack[sp] = t;
	    indexStack[sp] = i;
	    sp++;
	    t = t->branch[i];
	    goto release_label_1;
	  release_label_2:
	    delete t->branch[i];
	  }
	  delete t->data[i];
	}
      }

      if (sp > 0) {
	sp--;
	t = nodeStack[sp];
	i = indexStack[sp];
	goto release_label_2;
      }
    }

  /*****************************************************************************
  Description : Search for target in trie.
  Pre         : root points to the root of a trie
  Post        : If the search is successful, the return value is the trie
                  DataType which points to the information structure holding
		  target; otherwise, return value is NULL
  Comments    :
  *****************************************************************************/
  template <class DataType>
    DataType* Trie<DataType>::find(const string& target)
    {
      TrieNode<DataType>* t= root;

      char *s = (char *)target.c_str();
      while (t) {
	char c = *s;
	s++;
	if (*s == 0) {
	  // input is exhausted.
	  DataType* d = t->data[c];
	  if (!d)
	    return NULL;
	  else if (d->get_key() == target)
	    return d;
	  else
	    return NULL;
	}
	else {
	  TrieNode<DataType>* n = t->branch[c];
	  if (n)
	    t = n;
	  else {
	    DataType* d = t->data[c];
	    if (!d)
	      return NULL; // no data
	    else { // input is not exhausted yet.
	      if (d->get_key() == target)
		return d;
	      else
		return NULL;
	    }
	  }
	}
      }


      return NULL;
    }



  /******************************************************************************
	Description: Inserts two data elements into the newly created trie node
	Comments   :
  ******************************************************************************/
  template <class DataType>
    void Trie<DataType>::insert(TrieNode<DataType>* t, DataType* d, const DataType& data, int depth)
    {
      const string& k1 = d->get_key();
      const string& k2 = data.get_key();
      TrieNode<DataType>* pre = NULL;

    insert_label_1:
      char k1c = k1[depth];
      char k2c = k2[depth];
      if (k1c == k2c) {
	// another collision.
	TrieNode<DataType>* node = new TrieNode<DataType>(branchSize);
	t->branch[k1c] = node;
	pre = t;
	t = node;
	depth++;
	goto insert_label_1;
      }
      else { // now just add two distinct prefixes into the new trie node
	if (k1.size() == (size_t)depth)
	  pre->data[k1[depth-1]] = d;
	else
	  t->data[k1c] = d;
	if (k2.size() == (size_t)depth)
	  pre->data[k2[depth-1]] = new DataType(data);
	else
	  t->data[k2c] = new DataType(data);
      }
    }


  /*****************************************************************************
	Description : Insert data into the trie.
	Comments    :
  *****************************************************************************/
  template <class DataType>
    bool Trie<DataType>::insert(const DataType& data)
    {
      string& key = (string&)(data.get_key());
      char *s = (char *)key.c_str();
      int keySize = key.size();

      if (!root) {
	// first element in the Trie.
	num++;
	root = new TrieNode<DataType>(branchSize);
	root->data[*s] = new DataType(data);
	return true;
      }

      int depth = 0;
      TrieNode<DataType>* t = root;
      while (true) {
	char c = s[depth];
	depth++;
	if (keySize == depth) { // input is exhausted.
	  if (t->data[c] == NULL) { // data slot available.
	    t->data[c] = new DataType(data);
	    num++;
	    return true;
	  }
	  else {
	    DataType* d = t->data[c];
	    if (d->get_key() == key)
	      return false; // already exists.
	    else {
	      // since key is exhausted, replace this data with the given key data
	      t->data[c] = new DataType(data);
	      // now add d into the new trie node
	      if (t->branch[c]) {
		t = t->branch[c];
		((DataType&)data) = *d;
		delete d;
		key = data.get_key();
		s = (char *)key.c_str();
		keySize = key.size();
		continue;
	      }
	      else {
		// now add d into the new trie node
		TrieNode<DataType>* node = new TrieNode<DataType>(branchSize);
		t->branch[c] = node;
		node->data[d->get_key()[depth]] = d;
		num++;
		return true;
	      }
	    }
	  }
	}
	else {
	  TrieNode<DataType>* n = t->branch[c];
	  if (n)
	    t = n;
	  else { // no more strings beyond this depth yet.
	    DataType* d = t->data[c];
	    if (!d) { // first element of the strings that starts with the current prefix
	      t->data[c] = new DataType(data);
	      num++;
	      return true;
	    }
	    else { // input is not exhausted yet.
	      if (d->get_key() == key)
		return false; // duplicate data.
	      else {
		// now create another trie node
		TrieNode<DataType> *node = new TrieNode<DataType>(branchSize);
		if (d->get_key().size() == (size_t)depth) {
		  // leave this data alone
		  t->branch[c] = node;
		  t = node;
		}
		else {
		  // also note that key length is greater than depth.
		  t->branch[c] = node;
		  t->data[c] = NULL;
		  insert(node, d, data, depth); // insert the two ditinct data into the trie.
		  num++;
		  return true;
		}
	      }
	    }
	  }
	}
      }
    }



  /*****************************************************************************
  Description : Delete the TrieNode identified by the target key.
  Comments    :
  *****************************************************************************/
  template <class DataType>
    bool Trie<DataType>::del(const string& target)
    {
      TrieNode<DataType>* t= root;

      char *s = (char *)target.c_str();
      while (t) {
	char c = *s;
	s++;
	if (*s == 0) {  // input is exhausted.
	  if (!t->data[c])
	    return false; // no such data
	  else {
	    if (t->data[c]->get_key() != target)
	      return false;
	    else {
	      delete t->data[c];
	      t->data[c] = NULL;
	      num--;
	      return true;
	    }
	  }
	}
	else {
	  TrieNode<DataType>* n = t->branch[c];
	  if (n)
	    t = n;
	  else {
	    DataType* d = t->data[c];
	    if (!d)
	      return false; // no data
	    else { // input is not exhausted yet.
	      if (d->get_key() != target)
		return false; // no data
	      else {
		delete t->data[c];
		t->data[c] = NULL;
		num--;
		return true;
	      }
	    }
	  }
	}
      }

      return false;
    }


  /*****************************************************************************
  Description : Traversal function.
  Comments    : Helper function.
  *****************************************************************************/
  template <class DataType>
    void Trie<DataType>::in_order(TrieNode<DataType>* node, ManipFuncType manip)
    {
      int i;
      int sp = 0;
      DynamicArray<TrieNode<DataType>*> nodeStack;
      DynamicArray<int> indexStack;


    in_order_label_1:
      i = 0;
      while (i < branchSize) {
	if (node->data[i])
	  manip(node->data[i]);
	if (node->branch[i]) {
	  nodeStack[sp] = node;
	  indexStack[sp] = i;
	  sp++;
	  node = node->branch[i];
	  goto in_order_label_1;
	}
      in_order_label_2:
	i++;
      }

      if (sp > 0) {
	--sp;
	node = nodeStack[sp];
	i = indexStack[sp];
	goto in_order_label_2;
      }
    }



  /*****************************************************************************
  Description : Traverse the data in the trie in increasing order.
  Comments    :
  *****************************************************************************/
  template <class DataType>
    void Trie<DataType>::traverse(ManipFuncType manip)
    {
      if (root)
	in_order(root, manip);
    }

};
#endif
