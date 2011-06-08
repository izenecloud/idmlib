/**
 * @file DocumentRepresentor.h
 * @author Zhongxia Li
 * @date Jun 3, 2011
 * @brief 
 */
#ifndef DOCUMENT_REPRESENTOR_H_
#define DOCUMENT_REPRESENTOR_H_


#include <idmlib/idm_types.h>
#include <idmlib/util/CollectionUtil.h>

#include "SparseVectorSetFile.h"

#include <3rdparty/am/rde_hashmap/hash_map.h>


NS_IDMLIB_SSP_BEGIN

class DocumentRepresentor : public CollectionProcessor
{
public:
    DocumentRepresentor(
        const std::string& colBasePath,
        const std::string& laResPath,
        size_t maxDoc = 0,
        const std::string& docRepFileName = "./doc_rep.tmp",
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
    : CollectionProcessor(colBasePath, laResPath, maxDoc, encoding)
    , docCount_(0)
    , docRepVecFileName_(docRepFileName)
    , docRepVecOFile_(docRepVecFileName_)
    {
    	docRepVecOFile_.open();
    }

    ~DocumentRepresentor()
    {
    	docRepVecOFile_.close();
    }

public:
    void represent()
    {
        processSCD();
    }

private:
    /*virtual*/
    void processDocumentAnalyzedContent()
    {
        docCount_++;
        gatherTFDF_();

    }

    /*virtual*/
    void postProcess()
    {
    	finishInsert();

    	cout << "term count: "<<term_df_map_.size() <<endl;
    	cout << "document count: " << docCount_ << endl;

    	calcWeight();
    }

private:
    void gatherTFDF_()
    {
        term_tf_map_.clear();

        termid_t termid;
        for (TermIdList::iterator iter = pTermIdList_->begin(); iter != pTermIdList_->end(); iter++)
        {
            termid = iter->termid_;

            hashmap_iterator_t tf_ret = term_tf_map_.find(termid);
            if (tf_ret == term_tf_map_.end())
            {
                term_tf_map_.insert(hashmap_t::value_type(termid, 1));

                // df
                hashmap_iterator_t df_ret = term_df_map_.find(termid);
                if (df_ret == term_df_map_.end())
                {
                    term_df_map_.insert(hashmap_t::value_type(termid, 1));
                }
                else
                {
                    df_ret->second ++;
                }
            }
            else
            {
                tf_ret->second ++;
            }
        }

        // document representation vector
        SparseVector<> sv(curDocId_, term_tf_map_.size());

        size_t docLen = pTermIdList_->size();
        for(hashmap_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            iter->second /= docLen;  // normalize tf
            sv.insertItem(iter->first, iter->second);
        }

//        sv.print();
        docRepVecOFile_.put(sv);
    }

    void finishInsert()
    {
    	docRepVecOFile_.flush();
    	cout << "---- saved:  " << docRepVecOFile_.size()<<endl;
    	docRepVecOFile_.close();
    }

    void calcWeight()
    {
        DLOG(INFO) << "start calcWeight" <<endl;

        SparseVectorSetIFile<> inf(docRepVecFileName_);
        inf.open();

        SparseVectorSetOFile<> docrepFile("docrep.tmp");
        docrepFile.open();

        size_t total = 0;
        while(inf.next())
        {
        	SparseVector<> sv = inf.get();

        	float idf;
        	int df = 0;
        	SparseVector<>::list_iter_t iter;
        	for (iter = sv.list.begin(); iter != sv.list.end(); iter++)
        	{
        		df = term_df_map_[iter->itemid];
        		if (df == 0)
        			df = 1;

        		idf = std::log((float)docCount_ / term_df_map_[iter->itemid]);
        		iter->value *= idf;
        	}

        	docrepFile.put(sv);

        	++total;
        }

        inf.close();
        //inf.remove(); //remove it

        docrepFile.close();

        DLOG(INFO) << "end calcWeight" <<endl;

        cout << "total vector: " <<total << endl;
    }

public:
    /// for test
    void print()
    {
        cout << "--- Doc Count: " << docCount_ <<endl;
        cout << "--- TF --- " << endl;
        for(hashmap_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            cout << iter->first << " : " << iter->second << endl;
        }
        cout << endl;
        cout << "--- DF --- " << endl;
        for(hashmap_iterator_t iter = term_df_map_.begin(); iter != term_df_map_.end(); iter++)
        {
            cout << iter->first << " : " << iter->second << endl;
        }
        cout << endl;
    }

private:
    size_t docCount_;
    std::string docRepVecFileName_;
    SparseVectorSetOFile<> docRepVecOFile_;

    typedef rde::hash_map<uint32_t, float> hashmap_t;
    typedef rde::hash_map<uint32_t, float>::iterator hashmap_iterator_t;

    hashmap_t term_tf_map_;
    hashmap_t term_df_map_;
};

NS_IDMLIB_SSP_END

#endif /* DOCUMENT_REPRESENTOR_H_ */
