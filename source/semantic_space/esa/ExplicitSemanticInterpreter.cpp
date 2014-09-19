#include <idmlib/semantic_space/esa/ExplicitSemanticInterpreter.h>
#include <idmlib/util/time_util.h>

#include <glog/logging.h>

#include <sstream>
using namespace std;
using namespace idmlib::ssp;
using namespace idmlib::util;

const float MemWikiIndex::thresholdWegt_ = 0.02f; // 0.015 ~ 0.02 ?


static double sTotalTime = 0;
//#define DOC_SIM_TEST

bool ExplicitSemanticInterpreter::interpret(size_t maxOutFileSize, size_t maxCount)
{
    DLOG(INFO) << "start interpreting" <<endl;

    if (!bWikiIndexState_)
    {
        std::cerr << "WikiIndex load failed!"<< std::endl;
        return false;
    }

    SparseVectorSetIFileType inf(docSetPath_+"/doc_rep.vec");
    if (!inf.open())
    {
        return false;
    }

    // output file
    size_t fileCount = 1;
    stringstream outFileName;
    outFileName << docSetPath_<< "/doc_int.vec" << (fileCount++);
    cout << outFileName.str() <<endl;

    boost::shared_ptr<SparseVectorSetOFileType> pof(new SparseVectorSetOFileType(outFileName.str()));
    if (!pof->open())
        return false;

    size_t total = 0;
    while (inf.next())
    {
        SparseVectorType& docvec = inf.get();
#ifdef DOC_SIM_TEST
        docvec.print();
#endif

        SparseVectorType ivec(docvec.rowid);

        //double t1 = TimeUtil::GetCurrentTimeMS(); // time
#ifndef IZENE_INDEXER_
        interpret_(docvec, ivec);
#else
        interpret_ir_(docvec, ivec);
#endif
        //double t2 = TimeUtil::GetCurrentTimeMS(); // time
        //cout << "interpreted in "<< (t2-t1) <<"s."<< endl;
        //sTotalTime += (t2-t1); // time

#ifdef DOC_SIM_TEST
        cout << "**interpreted vector: " << endl;
        //ivec.sort();
        ivec.print();
        cout <<ivec.rowid<< endl;
#endif
        pof->put(ivec);

        ++total;

        if ((total % 1000) == 0)
            LOG(INFO) << "interpreted: " <<total << endl;

        if (maxCount != 0 && total >= maxCount)
            break;

        // check output file
        if ((total % maxOutFileSize) == 0)
        {
            pof->close();

            stringstream outFileName;
            outFileName << docSetPath_<< "/doc_int.vec" << (fileCount++);
            cout << outFileName.str() <<endl;
            pof.reset(new SparseVectorSetOFileType(outFileName.str()));
            if (!pof->open())
            {
                cout<<"failed to open: "<<outFileName.str()<<endl;
                return false;
            }
        }
    }

    cout << "average time: " << sTotalTime/total << "s." <<endl; //time

    inf.close();
    pof->close();

    DLOG(INFO) << "end interpreting. ("<<total <<")"<<endl;

    return true;
}


/// private

void ExplicitSemanticInterpreter::interpret_(SparseVectorType& docvec, SparseVectorType& ivec)
{
    // The weights of interpretation vector (of which each element is the weight of the doc to a concept),
    // are calculated by incrementally weight accumulation while traverse entries for text terms
    // over the inverted index of wiki concepts, conceptWeightMap is used to accumulate weights.

    std::map<docid_t, weight_t> conceptWeightMap;

    uint32_t termid, conceptId;
    weight_t wdoc, wcon;
    SparseVector<>::list_iter_t tIter, cIter;

    // for all terms in representation of the document
    for (tIter = docvec.list.begin(); tIter != docvec.list.end(); tIter ++)
    {
        termid = tIter->itemid;
        wdoc = tIter->value;

        // for entry of current word(term), calculate sub weights to concepts
        SparseVectorType& consVec = pWikiIndex_->getInvertedList(termid);
        for (cIter = consVec.list.begin(); cIter != consVec.list.end(); cIter ++)
        {
            conceptId = cIter->itemid;
            wcon = cIter->value;

            if (conceptWeightMap.find(conceptId) != conceptWeightMap.end())
            {
                conceptWeightMap[conceptId] += wdoc * wcon;
            }
            else
            {
                conceptWeightMap.insert(std::make_pair(conceptId, wdoc * wcon));
            }
        }
    }

    std::map<docid_t, weight_t>::iterator cwIter;
#if 1 // normalization (convert to unit-length vector)
    // vector length, or magnitude
    weight_t vecLength = 0;
    for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
    {
        vecLength +=  cwIter->second * cwIter->second;
    }
    vecLength = std::sqrt(vecLength);
#else // not normalize
    weight_t vecLength = 1;
#endif
    // set doc interpretation vector
    weight_t w;
    for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
    {
        w = cwIter->second / vecLength;
        if (w > thresholdWegt_)
        {
            w = (int)((w+0.0005)*1000)/1000.0; // 4 after decimal point
            ivec.insertItem(cwIter->first, w);
        }

    }
}


#ifdef IZENE_INDEXER_
void ExplicitSemanticInterpreter::interpret_ir_(SparseVectorType& docvec, SparseVectorType& ivec)
{
    std::map<docid_t, weight_t> conceptWeightMap;

    for (size_t i = 0; i < docvec.list.size(); i++)
    {
        weight_t wtext = docvec.list[i].value;

        // mining properties
        Term term("Content", docvec.list[i].itemid);

        izenelib::ir::indexmanager::IndexReader* indexReader = pIndexer_->getIndexReader();
        izenelib::ir::indexmanager::TermReader*
        pTermReader = indexReader->getTermReader(idmlib::ssp::IzeneIndexHelper::COLLECTION_ID);
        if (!pTermReader)
        {
            continue;
        }

        bool find = pTermReader->seek(&term);
        if (find)
        {
            // term indexed
            weight_t DF = pTermReader->docFreq(&term);
            weight_t IDF = std::log(wikiDocNum_ / DF);
            ///cout <<"DF: "<< DF << ", IDF: " << IDF <<" ";

            izenelib::ir::indexmanager::TermDocFreqs* pTermDocReader = NULL;
            pTermDocReader = pTermReader->termDocFreqs();

            weight_t TF = 0;
            if (pTermDocReader != NULL)
            {
                // iterate concepts indexed by term
                while (pTermDocReader->next())
                {
                    docid_t conceptId = pTermDocReader->doc();
                    TF = pTermDocReader->freq();

                    size_t docLen =
                        indexReader->docLength(conceptId, idmlib::ssp::IzeneIndexHelper::getPropertyIdByName("Content"));
                    ///cout << "( doc:"<<conceptId<<", tf:"<<TF<<", doclen:"<<docLen<<", tf:"<<(TF / docLen) <<" ) " ;

                    // TF*IDF
                    weight_t wcon = IDF * (TF / docLen);
                    if (conceptWeightMap.find(conceptId) != conceptWeightMap.end())
                    {
                        // xxx, wtext is TF of text, IDF should be statistic in corpus where text is from ?
                        conceptWeightMap[conceptId] += (wtext*IDF) * wcon;
                    }
                    else
                    {
                        conceptWeightMap.insert(std::make_pair(conceptId, (wtext*IDF) * wcon));
                    }
                }
                ///cout << endl;
            }

            if (pTermDocReader)
            {
                delete pTermDocReader;
                pTermDocReader = NULL;
            }
        }

        if (pTermReader)
        {
            delete pTermReader;
            pTermReader = NULL;
        }
    }

    // construct interpretation vector
    weight_t w;
    std::map<docid_t, weight_t>::iterator cwIter;
    weight_t vecLength = 0; // (or magnitude) for normalization
    for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
    {
        vecLength +=  cwIter->second * cwIter->second;
    }
    vecLength = std::sqrt(vecLength);

    ivec.list.resize(0); // init
    for (cwIter = conceptWeightMap.begin(); cwIter != conceptWeightMap.end(); cwIter++)
    {
        w = cwIter->second / vecLength;
        if (w > thresholdWegt_)
            ivec.insertItem(cwIter->first, w);
    }
}
#endif
