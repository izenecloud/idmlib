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
    void gatherTFDF();

    void finishInsert()
    {
    	docRepVecOFile_.flush();

    	docRepVecOFile_.close();
    }

    void calcWeight();

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
