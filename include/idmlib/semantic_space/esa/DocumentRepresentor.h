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
        const std::string& docSetDir,
        size_t maxDoc = 0,
        bool removeStopwords = false,
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
    : CollectionProcessor(colBasePath, laResPath, maxDoc, removeStopwords, encoding)
    , docCount_(0)
    , docSetDir_(docSetDir)
    , docRepVecOFile_(docSetDir+"/doc_rep_tf.tmp")
    {
        if (!boost::filesystem::exists(docSetDir)) {
            boost::filesystem::create_directories(docSetDir);
            std::cout <<"** create: "<<docSetDir<<endl;
        }

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
        // todo, proess mining properties
        docCount_++;
        gatherTFDF();

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
    void gatherTFDF()
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
        SparseVectorType sv(curDocId_, term_tf_map_.size());

        size_t docLen = pTermIdList_->size();
        for(hashmap_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            iter->second /= docLen;  // normalize tf
            sv.insertItem(iter->first, iter->second);
        }

#ifdef DOC_SIM_TEST
        sv.print();//
#endif
        docRepVecOFile_.put(sv);
    }

    void finishInsert()
    {
    	docRepVecOFile_.flush();

    	docRepVecOFile_.close();
    }

    void calcWeight()
    {
        DLOG(INFO) << "post processing (start calcWeight)" <<endl;

        SparseVectorSetIFileType inf(docSetDir_+"/doc_rep_tf.tmp");
        inf.open();

        SparseVectorSetOFileType docRepFile(docSetDir_+"/doc_rep.vec");
        docRepFile.open();

        size_t total = 0;
        while(inf.next())
        {
        	SparseVectorType sv = inf.get();

        	float idf;
        	int df = 0;
        	SparseVectorType::list_iter_t iter;
        	for (iter = sv.list.begin(); iter != sv.list.end(); )
        	{
        		df = term_df_map_[iter->itemid];
        		if (df == 0)
        			df = 1;

        		idf = std::log((float)docCount_ / term_df_map_[iter->itemid]);
        		iter->value *= idf;

        		//filt
        		if(iter->value > thresholdWegt_)
        		{
        		    iter++;
        		}
        		else
        		{
        		    iter = sv.list.erase(iter);
        		    sv.len --;
        		}
        	}

        	if (sv.len > 0)
        	    docRepFile.put(sv);

        	++total;
        	if ((total % 5000) == 0)
        	    DLOG(INFO) << total*100/docCount_ <<"%"<< endl;
        }

        inf.close();
        inf.remove(); //remove it

        docRepFile.close();

        DLOG(INFO) << "post processing (end calcWeight)" <<endl;

        cout << "inerpretation vector count: " <<total << endl;
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
    static const float thresholdWegt_ = 0.01f;

    size_t docCount_;
    std::string docSetDir_;
    SparseVectorSetOFileType docRepVecOFile_;

    typedef rde::hash_map<uint32_t, float> hashmap_t;
    typedef rde::hash_map<uint32_t, float>::iterator hashmap_iterator_t;

    hashmap_t term_tf_map_;
    hashmap_t term_df_map_;
};

NS_IDMLIB_SSP_END

#endif /* DOCUMENT_REPRESENTOR_H_ */
