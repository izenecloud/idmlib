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
        const std::string& docRepPath = "./doc_rep_tmp",
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
    : CollectionProcessor(colBasePath, laResPath, maxDoc, encoding)
    , docCount_(0)
    , docRepFile_(docRepPath)
    {
    }

public:
    void represent()
    {
        processSCD();
    }

    /*virtual*/
    void processDocumentAnalyzedContent()
    {
        docCount_++;
        gatherTFDF_();
        //print();
    }

    /*virtual*/
    void postProcess()
    {
        spvecFile_.flush();

        cout << "---- flush, reload ---" <<endl;
        spvecFile_.load();
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

        sv.print();
        spvecFile_.add(sv);
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
    std::string docRepFile_;

    typedef rde::hash_map<uint32_t, float> hashmap_t;
    typedef rde::hash_map<uint32_t, float>::iterator hashmap_iterator_t;

    hashmap_t term_tf_map_;
    hashmap_t term_df_map_;

    SparseVectorSetFile<> spvecFile_;
};

NS_IDMLIB_SSP_END

#endif /* DOCUMENT_REPRESENTOR_H_ */
