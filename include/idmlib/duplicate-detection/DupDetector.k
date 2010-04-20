/*
 * DupDetector.h
 *
 *  Created on: 2009-1-13
 *      Author: jinglei
 */

#ifndef DUPDETECTOR_H_
#define DUPDETECTOR_H_
#include "NearDuplicateClustering.h"
#include "MEfficientLHT.h"
#include "DupTypes.h"
//#include "TestDocument.h"

#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/connected_components.hpp"
#include "boost/graph/incremental_components.hpp"

#include <wiselib/IntTypes.h>

#include <set>
#include <list>
#include <vector>
#include <ext/hash_map>


namespace sf1v5{

/**
 * @brief The api interface to SF1.
  */
class DupDetector {
public:
	DupDetector(unsigned int collectionId, float threshold);
	DupDetector(unsigned int collectionId);
	virtual ~DupDetector();
	/// Perform an initialization process and load necessary data.
	//bool init();

	/// Release allocated resource, including the  _docBitMap for the whole collection.
	bool release();

	/// Release all other resources except the  _docBitMap for the whole collection.
	bool clear();

	/// Get a unique document id list with duplicated ones excluded.
	bool getUniqueDocIdList(std::vector<unsigned int>& docIdList);

	/// Get a duplicated document list to a given document.
	bool getDuplicatedDocIdList(unsigned int docId, std::vector<unsigned int>& docIdList);

	/// Tell two documents belonging to the same collections are duplicated.
	bool isDuplicated( unsigned int docId1, unsigned int docId2, bool& result);

	/// Conduct a duplicate detection analysis for a given collection. After this function succeeds, we should be able to get intended results in getUniqueDocIdList(), getDuplicatedDocIdList() and isDuplicated().
	bool runDuplicateDetectionAnalysis();

	/// Whether need to analyze the collection.
	bool needToAnalyze();

	/// Insert new documents to a collection. If a document already exists, it is ignored.
	bool insertDocuments(const std::vector<unsigned int>& docIdList, const std::vector<std::vector<unsigned int> >& documentList);

	/// Update contents of documents.
	bool updateDocuments(const std::vector<unsigned int>& docIdList, const std::vector<std::vector<unsigned int> >& documentList);

	/// Remove documents from a collection. From now, those documents should be excluded in the result.
	bool removeDocuments(const std::vector<unsigned int>& docIdList);

	/// Get the collection id for which this detector is running;
	unsigned int getCollectionId() const;

public:
	/**
	 * @brief The parameter, the algorithm to use, bloom filter could not be used on termId representation of a doc.
	 * BRODER=0, CHARIKAR=1.
	 */
	static DUP_ALG _alg;

private:
	static bool isFirstInstance;

protected:

	/// whether the collection need to be analyzed.
	bool _needAnalyze;

	///the collectionID to run dup detection.
   unsigned int _collectionId;


   /**
    * @brief map a docID to its bitstring.
    * dup only store the bitstring skeleton of the document content.
    * this is used to represent the whole collection in a very space saving way.
    */
   DOC_BIT_MAP _docBitMap;

   /**
    * @brief store already compared document id pair, the pair is a ub8 number
    * high 32bits is bigger than lower 32bits;
    * After inserting or updating doc set, only uncompared pairs are inserted into the _dupPairs result.
    */
   wiselib::MEfficientLHT<wiselib::TypeKey<wiselib::ub8> > _pairTable;

   // The near duplicate pairs detected in the collection.
   //DUP_PAIR _dupPairs;
   DUP_PAIR_SET _dupPairs;

   /**
    * @brief near duplicate algorithm
    */
   NearDuplicateAlgorithm* ndAlgo;

protected:
	//define types used for grouping using boost disjoint set.
	typedef boost::adjacency_list <boost::vecS, boost::vecS, boost::undirectedS> Graph;
	typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
	typedef boost::graph_traits<Graph>::vertices_size_type size_type;
	typedef size_type* Rank;
	typedef Vertex* Parent;
	typedef boost::component_index<unsigned int> Components;
	Graph G;//used for grouping.
	std::vector<size_type> rank;
	std::vector<Vertex> parent;
	///used for grouping.
    boost::disjoint_sets<Rank, Parent> * ds;
	/// map each dup docid to its group, used to quickly find dupIds in the same group.
	map<unsigned int, Components::size_type> docGroupMap;

protected:
	bool preClustering(MULTI_MAP_T& keySigMap, set<wiselib::ub4>& keySet);
	bool grouping();
	bool has_already_compared(int docID1, int docID2);
};
}
#endif /* DUPDETECTOR_H_ */
