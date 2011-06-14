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
        findMatchPairs0(sv);
        double t2 = TimeUtil::GetCurrentTimeMS();
        sTotalTime += (t2-t1);
        //cout <<"find matched paris in "<<(t2-t1)<<"s."<<endl;

        indexVector0(sv);

        count ++;
        if (count % 1000 == 0)
            DLOG(INFO) << count << endl;

        if (maxDoc != 0 && count >= maxDoc)
            break;
    }

    cout <<"average time: find matches for a vector in "<<(sTotalTime/count)<<"s."<<endl;

    allPairsOutput_->finish();

    DLOG(INFO) <<"End all pairs similarity searching.  processed "<< count<<std::endl;
}

/// Extensions for Disk Resident Data
void AllPairsSearch::findAllSimilarPairs(std::vector<boost::shared_ptr<DataSetIterator> >& dataSetIteratorList, size_t maxDoc)
{
    DLOG(INFO) <<"Start all pairs similarity searching."<<std::endl;
    size_t count = 0;

    size_t fileSize = 0;
    size_t dataSetNum = dataSetIteratorList.size();
    for (size_t i = 0; i < dataSetNum; i++)
    {
        DLOG(INFO) <<"===> Resume(start rebuilding inverted index) at vector "<<count <<endl;
        // for memory resident part of vectors, build inverted index and find all pairs.
        invertedLists_.clear();
        boost::shared_ptr<DataSetIterator>& dataSetIterator = dataSetIteratorList[i];
        dataSetIterator->init();
        while (dataSetIterator->next())
        {
            SparseVectorType& sv = dataSetIterator->get();
            findMatchPairs0(sv);
            indexVector0(sv);

            count ++;
            if (count % 1000 == 0)
                DLOG(INFO) << count << endl;
        }

        if (i == 0)
            fileSize = count;

        DLOG(INFO) <<"===> Halting(stop building inverted index) at vector "<<count <<endl;

        // for left vectors in data set, find all pairs with memory resident vectors,
        // but not update inverted index.
        size_t left = 0;
        for (size_t j = i+1; j < dataSetNum; j ++)
        {
            boost::shared_ptr<DataSetIterator>& dataSetIterator = dataSetIteratorList[j];
            dataSetIterator->init();
            while (dataSetIterator->next())
            {
                SparseVectorType& sv = dataSetIterator->get();
                findMatchPairs0(sv);

                left ++;
                if (left % 3000 == 0)
                    DLOG(INFO) << left*100/(fileSize*(dataSetNum-i+1))<<"%" << endl;
            }
        }
    }

    allPairsOutput_->finish();

    DLOG(INFO) <<"End all pairs similarity searching.  processed "<< count<<std::endl;
}


/// private ////

void AllPairsSearch::findMatchPairs0(SparseVectorType& sv)
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

void AllPairsSearch::indexVector0(SparseVectorType& sv)
{
    uint32_t vecid = sv.rowid;
    uint32_t vecitemid;
    float vecitemv;

    SparseVectorType::list_iter_t vecIter;
    for (vecIter = sv.list.begin(); vecIter != sv.list.end(); vecIter++)
    {
        vecitemid = vecIter->itemid;
        vecitemv = vecIter->value;

        updateInvertedList(vecitemid, vecid, vecitemv);
    }
}




