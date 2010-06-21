/*
 * DupDetector.cpp
 *
 *  Created on: 2009-1-13
 *      Author: jinglei
 */

#include <sf1v5/document-manager/duplicate-detection-submanager/DupDetector.h>
#include <sf1v5/document-manager/duplicate-detection-submanager/UtilTool.h>
#include <sf1v5/document-manager/duplicate-detection-submanager/BroderAlgorithm.h>
#include <sf1v5/document-manager/duplicate-detection-submanager/CharikarAlgorithm.h>
#include <sf1v5/document-manager/duplicate-detection-submanager/BloomFilter.h>

#include <util/CBitArray.h>
#include <wiselib/StopWatch.h>

#include <utility>

using namespace std;
using namespace izenelib::util;
using namespace boost;
using namespace sf1v5;

DUP_ALG DupDetector::_alg=CHARIKAR;
bool DupDetector::isFirstInstance=true;

DupDetector::DupDetector(unsigned int collectionId, float threshold) {
    /*
	if(isFirstInstance==true)
	{
		UtilTool::readDupProperty();
		isFirstInstance=false;
	}
    */
	_collectionId=collectionId;

	//ndAlgo=new CharikarAlgorithm(threshold);
    ndAlgo=new BroderAlgorithm;
    /*
	switch(_alg)
	{
	case BRODER:
		ndAlgo=new BroderAlgorithm;
		break;
	case CHARIKAR:
		ndAlgo=new CharikarAlgorithm;
		break;
	}
    */
	_needAnalyze=true;
	ds=0;
}

DupDetector::DupDetector(unsigned int collectionId) {

	_collectionId=collectionId;
	ndAlgo=new CharikarAlgorithm();

	_needAnalyze=true;
	ds=0;
}

DupDetector::~DupDetector() {
	delete ndAlgo;
	if(ds) delete ds;
}

/// Conduct a duplicate detection analysis for a given collection.
/// After this function succeeds, we should be able to get intended results in getUniqueDocIdList(), getDuplicatedDocIdList() and isDuplicated().
bool DupDetector::runDuplicateDetectionAnalysis(){

	//Pre-cluster docs into rough word clusters.
    MULTI_MAP_T keySigMap;
    set<ub4> keySet;
    preClustering(keySigMap,keySet);

    //now detect all the candidate dup doc pairs;
    cout<<"Detecting near dup pairs..."<<endl;
    StopWatch sw;
    typedef MULTI_MAP_T::iterator CIT;
    typedef pair<CIT,CIT> RANGE;

    int count=1;
    for(std::set<ub4>::iterator iterKey=keySet.begin();iterKey!=keySet.end();iterKey++,count++)
    {
    	if(count%512==0) cout<<".";
    	if(count%5120==0) cout<<endl;
    	RANGE range=keySigMap.equal_range(*iterKey);
     	for(CIT iter1=range.first;iter1!=range.second;iter1++)
    	{
    		CIT iter2=iter1;
    		++iter2;
    		for(;iter2!=range.second;iter2++)
    	    {
    			int neardupScore=ndAlgo->neardup_score(iter1->second, iter2->second);
    			bool needCompare=ndAlgo->is_neardup(neardupScore);
    		   if (needCompare&&!has_already_compared(iter1->second.get_doc_id(), iter2->second.get_doc_id()))
    		    {
    		    	unsigned int docId1=(iter1->second).get_doc_id();
    		    	unsigned int docId2=(iter2->second).get_doc_id();
    				_dupPairs.insert(make_pair(docId1,docId2));
    		    }
    	     }
    	 }
     }
    cout<<endl;
    sw.show();
    cout<<"pair detecting done"<<endl;

    // Now grouping the results
    grouping();

    _needAnalyze=false;
    return true;
}

/**
 * Get a duplicated document list to a given document.
 * @return if no dupDocs for this docId, return false, else return false;
 */
bool DupDetector::getDuplicatedDocIdList(unsigned int docId, std::vector<unsigned int>& docIdList)
{
	if(docGroupMap.find(docId)==docGroupMap.end()) return false;
	Components::size_type c=docGroupMap[docId];
	Components components(&parent[0], &parent[0] + parent.size());
	Components::value_type::iterator
		j = components[c].begin(),
		jend = components[c].end();
	for (;j!=jend;++j)
	{
		if(*j!=docId)
			docIdList.push_back(*j);
	}
	return true;
}

/// Get a unique document id list with duplicated ones excluded.
bool DupDetector::getUniqueDocIdList(std::vector<unsigned int>& docIdList)
{
	///Get all the dup docs of the collection.
	set<unsigned int> dupDocSet;
	Components components(&parent[0], &parent[0] + parent.size());
	for (Components::size_type c = 0; c < components.size(); ++c)
	{
		Components::value_type::iterator
		    j = components[c].begin(),
		    jend = components[c].end();
		if(++j!=jend){
	    	for (;j!=jend;++j)
	       {
	    	   dupDocSet.insert(*j);
	       }
	    }
	}
	DOC_BIT_MAP::iterator iter=_docBitMap.begin();
	for(;iter!=_docBitMap.end();iter++)
	{
		if (dupDocSet.find(iter->first)==dupDocSet.end())
			docIdList.push_back(iter->first);
	}
	return true;
}

/// Tell two documents belonging to the same collections are duplicated.
bool DupDetector::isDuplicated( unsigned int docId1, unsigned int docId2, bool& result)
{
	if(ds==NULL) return false;
	else{
		result=same_component(docId1,docId2,*ds);
		return true;
	}
}

/**
 * Insert new documents to a collection. If a document already exists, it is ignored.
 */
bool DupDetector::insertDocuments(const std::vector<unsigned int>& docIdList, const std::vector<std::vector<unsigned int> >& documentList)
{
	cout<<"inserting files"<<endl;
	assert(docIdList.size()==documentList.size());
	for(size_t i=0;i<docIdList.size();i++)
	{
		if(i%51==0) cout<<".";
		if(i%510==0) cout<<endl;
		if(_docBitMap.find(docIdList[i])==_docBitMap.end())
		{
		    CBitArray bitArray;
		    ndAlgo->generate_document_signature(documentList[i], bitArray);
		    _docBitMap.insert(make_pair(docIdList[i],bitArray));
		}
	}
	cout<<endl;
	_needAnalyze=true;
	return true;
}

/// Update contents of documents.
bool DupDetector::updateDocuments(const std::vector<unsigned int>& docIdList, const std::vector<std::vector<unsigned int> >&  documentList)
{
	assert(docIdList.size()==documentList.size());
	for(size_t i=0;i<docIdList.size();i++)
	{
		CBitArray bitArray;
		ndAlgo->generate_document_signature(documentList[i], bitArray);
		DOC_BIT_MAP::iterator it=_docBitMap.find(docIdList[i]);
		if(it!=_docBitMap.end()){
		    it->second=bitArray;
		}
		//when there is no previous doc to update, think it as insertion.
		else
			_docBitMap.insert(make_pair(docIdList[i],bitArray));
	}
	_needAnalyze=true;
	_pairTable.release();
	_dupPairs.clear();
   return true;

}

/// Remove documents from a collection. From now, those documents should be excluded in the result.
bool DupDetector::removeDocuments(const std::vector<unsigned int>& docIdList)
{

	for(size_t i=0;i<docIdList.size();i++)
	{
		_docBitMap.erase(docIdList[i]);
	}
	_needAnalyze=true;

	_pairTable.release();
	_dupPairs.clear();

	return true;
}

bool DupDetector::has_already_compared(int docID1, int docID2)
{
    ub8 key;
    // construct a unique key given two docIDs.
    if (docID1 > docID2) {
        key = docID1;
        key <<= 32;
        key += docID2;
    } else {
        key = docID2;
        key <<= 32;
        key += docID1;
    }

    if (_pairTable.find(key)) {
        return true;
    } else {
        _pairTable.insert(key);
        return false;
    }
}

/**
 * Pre-clustering the result into rough clusters with same "word" key.
 */
bool DupDetector::preClustering(MULTI_MAP_T& keySigMap, set<ub4>& keySet)
{

	cout<<"preclustering..."<<endl;
	StopWatch sw;
	DOC_BIT_MAP::iterator it=_docBitMap.begin();
	for(;it!=_docBitMap.end();it++)
	{
		std::set<NearDuplicateSignature> sigSet;
		// generate NearDuplicateSignature objects from this bit pattern
	   NearDuplicateSignature::
	       generate_near_duplicate_signatures(ndAlgo->num_dimensions(),
	            it->second,
	            it->first,
	            sigSet);
	   std::set<NearDuplicateSignature>::iterator it=sigSet.begin();
	   for (; it != sigSet.end(); it++) {
	      keySet.insert(it->get_sub_key().key);
	      keySigMap.insert(make_pair(it->get_sub_key().key,*it));
	    }
	}
	sw.show();
	cout<<"preclustering done"<<endl;

	return true;
}

/**
 * grouping the resulted dup doc pairs.
 */
bool DupDetector::grouping()
{
    StopWatch sw3;
    cout<<"start grouping..."<<endl;

    G.clear();
    for (DUP_PAIR_SET::iterator i = _dupPairs.begin(); i!=_dupPairs.end(); i++) {
    		add_edge(i->first, i->second, G);
     }
    rank.resize(num_vertices(G));
    parent.resize(num_vertices(G));

    if(ds)
    {
     	delete ds;
    }
   ds=new disjoint_sets<Rank, Parent>(&rank[0], &parent[0]);

    initialize_incremental_components(G, *ds);
    incremental_components(G, *ds);

    docGroupMap.clear();
	Components components(&parent[0], &parent[0] + parent.size());
	for (Components::size_type c = 0; c < components.size(); ++c)
	{

		Components::value_type::iterator
		    j = components[c].begin(),
		    k=j,
		    jend = components[c].end();
		if(++k!=jend){
			for (;j!=jend;++j)
	       {
	    		docGroupMap.insert(make_pair(*j,c));
	       }
	    }
	}

    sw3.show();
    cout<<"grouping done"<<endl;
    return true;
}

bool DupDetector::release()
{
	_docBitMap.clear();
	_pairTable.release();
	_dupPairs.clear();
	delete ds;
	ds=NULL;
	G.clear();
	rank.clear();
	parent.clear();
	docGroupMap.clear();
	_needAnalyze=true;
	return true;
}

bool DupDetector::clear()
{
	_pairTable.release();
	_dupPairs.clear();
	delete ds;
	ds=NULL;
	G.clear();
	rank.clear();
	parent.clear();
	docGroupMap.clear();
	_needAnalyze=true;
	return true;
}

/**
 * In following situations, for example, we need to analyze again.
 * If runDuplicateDetectionAnalysis() was not called or failed.
 * After runDuplicateDetectionAnalysis() finished, new data was inserted or updated. (or deleted)
 */
bool DupDetector::needToAnalyze()
{
	return _needAnalyze;
}

unsigned int DupDetector::getCollectionId() const{
	return _collectionId;
}
