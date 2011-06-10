/**
 * @file all_pairs_search.h
 * @author Zhongxia Li
 * @date Jun 7, 2011
 * @brief 
 */
#ifndef ALL_PAIRS_SEARCH_H_
#define ALL_PAIRS_SEARCH_H_

#include <idmlib/idm_types.h>

#include "all_pairs_output.h"
#include "data_set_iterator.h"

#include <idmlib/util/time_util.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/am/google/sparse_hash_map>

#include <glog/logging.h>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

static float sTotalTime = 0;


/**
 * Finds all pairs of vectors in the dataset (vector set) with cosine
 * similarity meeting the specified threshold.
 *
 * [1] Bayardo, R.J., Ma, Y., Srikant, R.: Scaling up all pairs similarity search.
 * In: WWW(2007) 436 D. Lee et al.
 * [2] Dongjoo Lee, Jaehui Park, Junho Shim, Sang-goo Lee. An Efficient Similarity
 * Join Algorithm with Cosine Similarity Predicate. In DEXA (2)(2010)
 */
class AllPairsSearch
{
    typedef SparseVectorItemType InvertItem;
    typedef SparseVectorType InvertedList;


public:
    AllPairsSearch(
            boost::shared_ptr<AllPairsOutput> allPairsOutput,
            float thresholdSim = 0.04)
    : vector_index_num_max_(10)
    , vector_index_num_cur_(0)
    , thresholdSim_(thresholdSim)
    , allPairsOutput_(allPairsOutput)
    {
    }

public:

    /**
     * Memory Resident Data (inverted index for finding pairs is memory resident data)
     * @param dataSetIterator
     * @param maxDoc
     */
    void findAllSimilarPairs(boost::shared_ptr<DataSetIterator>& dataSetIterator, size_t maxDoc = 0);

    /**
     * Extensions for Disk Resident Data
     * @brief assume that each data set is memory containable when buiding inverted index.
     * @param dataSetIteratorList  with multi data set inputs.
     * @param maxDoc
     */

    void findAllSimilarPairs(std::vector<boost::shared_ptr<DataSetIterator> >& dataSetIteratorList, size_t maxDoc = 0);

    /**
     * Extensions for Disk Resident Data
     * @brief TODO, process large input file
     */

private:
    /// basic join
    void findMatchPairs0(SparseVectorType& sv);
    void indexVector0(SparseVectorType& sv);

private:
    InvertedList& getInvertedList(uint32_t ilid)
    {
        invertedlists_iterator_t ret = invertedLists_.find(ilid);
        if (ret != invertedLists_.end())
        {
            return *(ret->second);
        }

        return NULLInvertedList_;
    }

    /**
     * incrementally build inverted index
     */
    void updateInvertedList(uint32_t ilid, uint32_t itemid, float v)
    {
        invertedlists_iterator_t ret = invertedLists_.find(ilid);
        if (ret != invertedLists_.end())
        {
            ret->second->insertItem(itemid, v);
        }
        else
        {
            boost::shared_ptr<InvertedList> il(new InvertedList(ilid));
            il->insertItem(itemid, v);
            invertedLists_.insert(invertedlists_t::value_type(ilid, il));
        }
    }

private:
    typedef rde::hash_map<uint32_t, float > hashmap_t;
    typedef rde::hash_map<uint32_t, float >::iterator hashmap_iterator_t;

    typedef google::sparse_hash_map<uint32_t, boost::shared_ptr<InvertedList> > invertedlists_t;
    typedef google::sparse_hash_map<uint32_t, boost::shared_ptr<InvertedList> >::iterator invertedlists_iterator_t;

    invertedlists_t invertedLists_; // inverted index for score accumulation
    InvertedList NULLInvertedList_; // an empty list

    size_t vector_index_num_max_;  // the max number of vetors that can be indexed (keep inverted index in memory)
    size_t vector_index_num_cur_;  // current number of vetors that has indexed

    hashmap_t candidateVecs_;
    float thresholdSim_;

    boost::shared_ptr<AllPairsOutput> allPairsOutput_;
};

NS_IDMLIB_SIM_END

#endif /* ALL_PAIRS_SEARCH_H_ */
