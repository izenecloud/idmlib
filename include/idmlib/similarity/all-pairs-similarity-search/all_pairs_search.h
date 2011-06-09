/**
 * @file AllPairsSearch.h
 * @author Zhongxia Li
 * @date Jun 7, 2011
 * @brief 
 */
#ifndef ALL_PAIRS_SEARCH_H_
#define ALL_PAIRS_SEARCH_H_

#include <idmlib/idm_types.h>

#include "data_set_iterator.h"
#include "all_pairs_output.h"
#include <idmlib/semantic_space/esa/SparseVector.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>
#include <3rdparty/am/google/sparse_hash_map>

#include <glog/logging.h>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN


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
     * Finds all pairs of vectors in the dataset (vector set) with cosine
     * similarity meeting the specified threshold.
     *
     * [1] Bayardo, R.J., Ma, Y., Srikant, R.: Scaling up all pairs similarity search.
     * In: WWW(2007) 436 D. Lee et al.
     * [2] Dongjoo Lee, Jaehui Park, Junho Shim, Sang-goo Lee. An Efficient Similarity
     * Join Algorithm with Cosine Similarity Predicate. In DEXA (2)(2010)
     *
     * @param dataSetIterator
     */
    void findAllSimilarPairs(boost::shared_ptr<DataSetIterator>& dataSetIterator, size_t maxDoc = 0)
    {
        DLOG(INFO) <<"Start all pairs similarity searching."<<std::endl;
        size_t count = 0;

        while (dataSetIterator->next())
        {
            SparseVectorType& sv = dataSetIterator->get();
#ifdef DOC_SIM_TEST
            sv.print();
#endif
            invertedIndexJoin_(sv);

            count ++;
            if (count % 1000 == 0)
                DLOG(INFO) << count << endl;

            if (maxDoc != 0 && count >= maxDoc)
                break;
        }

        allPairsOutput_->finish();

        DLOG(INFO) <<"End all pairs similarity searching.  processed "<< count<<std::endl;
    }

private:
    void invertedIndexJoin_(SparseVectorType& sv)
    {
        // empty map from doc id to weight, forward
        candidateVecs_.clear();

        uint32_t vecid = sv.rowid;
        uint32_t vecitemid;
        float vecitemv;

        SparseVectorType::list_iter_t vecIter;
        for (vecIter = sv.list.begin(); vecIter != sv.list.end(); vecIter++)
        {
            vecitemid = vecIter->itemid;
            vecitemv = vecIter->value;

            InvertedList& il = getInvertedList(vecitemid);
            // score accumulation based on inverted index
            if (il.len > 0)
            {
                uint32_t candidateVecid;
                float ilitemv;
                InvertedList::list_iter_t ilIter;
                for (ilIter = il.list.begin(); ilIter != il.list.end(); ilIter++)
                {
                    candidateVecid = ilIter->itemid;
                    ilitemv = ilIter->value;
                    // accumulate, fill weight table
                    if (candidateVecs_.find(candidateVecid) == candidateVecs_.end())
                    {
                        candidateVecs_.insert(hashmap_t::value_type(candidateVecid, vecitemv * ilitemv));
                    }
                    else
                    {
                        candidateVecs_[candidateVecid] += vecitemv * ilitemv;
                    }
                }
            }

            // update inverted index
            updateInvertedList(vecitemid, vecid, vecitemv);
        }

        // Output similar pairs
        for (hashmap_iterator_t citer = candidateVecs_.begin(); citer != candidateVecs_.end(); citer++)
        {
            if (citer->second > this->thresholdSim_) {
                // output
                //cout << "(" << vecid <<"," << citer->first <<", " << citer->second<<")" << endl;
                allPairsOutput_->putPair(vecid, citer->first, citer->second);
                allPairsOutput_->putPair(citer->first, vecid, citer->second);
            }
        }
    }

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
