#include <idmlib/similarity/all-pairs-similarity-search/all_pairs_search.h>
#include <idmlib/util/time_util.h>

using namespace idmlib::sim;
using namespace idmlib::util;

void AllPairsSearch::findAllSimilarPairs(boost::shared_ptr<DataSetIterator>& dataSetIterator, size_t maxDoc)
{
    DLOG(INFO) <<"Start all pairs similarity searching."<<std::endl;
    size_t count = 0;

    while (dataSetIterator->next())
    {
        SparseVectorType& sv = dataSetIterator->get();
#ifdef DOC_SIM_TEST
        sv.print();
#endif

        double t1 = TimeUtil::GetCurrentTimeMS();
        invertedIndexJoin_(sv);
        double t2 = TimeUtil::GetCurrentTimeMS();
        sTotalTime += (t2-t1);
        //cout <<"index a vector in "<<(t2-t1)<<"s."<<endl;

        count ++;
        if (count % 100 == 0)
            DLOG(INFO) << count << endl;

        if (maxDoc != 0 && count >= maxDoc)
            break;
    }

    cout <<"average time: index a vector in "<<(sTotalTime/count)<<"s."<<endl;

    allPairsOutput_->finish();

    DLOG(INFO) <<"End all pairs similarity searching.  processed "<< count<<std::endl;
}

/// private ////

void AllPairsSearch::invertedIndexJoin_(SparseVectorType& sv)
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
