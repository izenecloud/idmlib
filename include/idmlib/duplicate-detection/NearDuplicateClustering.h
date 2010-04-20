#ifndef NearDuplicateClustering_h
#define NearDuplicateClustering_h 1
/**
 * @file NearDuplicateClustering.h
 * @author Jinglei
 * @date 2009.11.24
 * @brief clustering
 */

#include "MultiHashSet.h"
#include "NearDuplicatePair.h"
#include "TypeKey.h"
#include "Trie.h"
#include "MEfficientLHT.h"
#include "DupTypes.h"
#include "NearDuplicateAlgorithm.h"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/connected_components.hpp"
#include "boost/graph/incremental_components.hpp"

#include <map>
#include <utility>
#include <set>
#include <ext/hash_map>

namespace sf1v5{
/**
 * @brief near dulicate clustering class
 */

class NearDuplicateClustering {
protected:
  /**
   * @brief multi hash set used to find similarity pairs
   */
  MULTI_MAP_T mMultiMap;
  std::set<wiselib::ub4> mKeySet;

  /**
   * @brief store already compared document id pair, the pair is a ub8 number
   * high 32bits is bigger than lower 32bits;
   */
  wiselib::MEfficientLHT<wiselib::TypeKey<wiselib::ub8> > pairTable;
  /**
   * @brief near duplicate algorithm
   */
  NearDuplicateAlgorithm& ndAlgo;
  /**
   * @brief percent can be treat as duplicate
   */
//  float threshold_;

  /**
   * @brief test if two documents have been already compared, if not compared,
   * store them as compared
   *
   * @param docID1 first document id
   * @param docID2 second document id
   *
   * @return true, compared; false else.
   */
  bool has_already_compared(int docID1, int docID2);

  /**
   * private static constants.
   */
  static const int ONEWORD_DIVISION_FACTOR; // conservatively half the size of word.
  static const int MIN_NUM_ONEWORD_CLUSTERS;
public:
  inline NearDuplicateClustering(int numEstimatedObjects,
				 NearDuplicateAlgorithm& algo)
    :  ndAlgo(algo)
  {
	  ds=NULL;
  }
  inline ~NearDuplicateClustering() {if(ds!=NULL) delete ds; }

  /**
   * @brief add near duplicate signature by bitArray
   *
   * @param docName document name
   * @param bitArry docuemnt signature
   */
  void add_near_duplicate_signatures(const string& docName, wiselib::CBitArray& bitArry);
  /**
   * @brief add near duplicate signatures from the document token set.
   *
   * @param docName document name
   * @param tokens tokens in the document with the name docName
   */
  void add_near_duplicate_signatures(const string& docName, const wiselib::DynamicArray<wiselib::ub8>& tokens);
  // finds near-duplicate pairs in order of proximity
  void find_near_duplicate_pairs(wiselib::DynamicArray<NearDuplicatePair>& results);
  // test near duplicate algorithms
  void test_near_duplicate_algorithm(const wiselib::DynamicArray<wiselib::YString>& docNames,
				     const wiselib::DynamicArray<wiselib::YString>& documents);
protected:
	typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS> Graph;
	typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
	typedef boost::graph_traits<Graph>::vertices_size_type size_type;
	typedef size_type* Rank;
	typedef Vertex* Parent;
	typedef boost::component_index<unsigned int> Components;
	Graph G;//used for grouping.
	std::vector<size_type> rank;
	std::vector<Vertex> parent;
    boost::disjoint_sets<Rank, Parent> * ds;//used for grouping.
};
}
#endif
