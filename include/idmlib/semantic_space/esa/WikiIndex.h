/**
 * @file WikiIndex.h
 * @author Zhongxia Li
 * @date Jun 1, 2011
 * @brief Definition of inverted index structure for Wikipedia, or other knowledge corpus.
 */
#ifndef WIKIINDEX_H_
#define WIKIINDEX_H_

#include <idmlib/idm_types.h>

#include <ir/index_manager/utility/system.h>
#include <ir/index_manager/index/LAInput.h>

#include <3rdparty/am/rde_hashmap/hash_map.h>


using namespace izenelib::util;
using namespace izenelib::ir::indexmanager;


NS_IDMLIB_SSP_BEGIN


/** Wikipedia concept */
struct WikiDoc
{
    docid_t docid;
    boost::shared_ptr<TermIdList> pTermIdList;
};

/**
 * Inverted index
 * @brief It can be used for Wikipedia or other knowledge corpus.
 */
class WikiIndex
{
public:
    WikiIndex(const string& dataFile = "./wiki.idx")
    : dataFile_(dataFile)
    {
        init();
    }

public:
    /**
     * Construct index
     * @{
     */
    virtual void insertDocument(WikiDoc& wikiDoc) = 0;

    virtual void postProcess() = 0;
    /** @} */

    virtual void load() = 0;

public:
    /// for test
    void printTF()
    {
        cout << "--- Doc Count: " << docCount_ <<endl;
        cout << "--- TF --- " << endl;
        for(termtf_map_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            cout << iter->first << " : " << iter->second << endl;
        }
        cout << endl;
    }

protected:
    void init()
    {
        docCount_ = 0;
        curDoc_ = 0;
    }

    void gatherTF_(WikiDoc& wikiDoc)
    {
        term_tf_map_.clear();
        termid_t termid;
        for (TermIdList::iterator iter = wikiDoc.pTermIdList->begin(); iter != wikiDoc.pTermIdList->end(); iter++)
        {
            termid = iter->termid_;

            termtf_map_iterator_t ret = term_tf_map_.find(termid);
            if (ret == term_tf_map_.end()) {
                term_tf_map_.insert(termtf_map_t::value_type(termid, 1));
            }
            else {
                ret->second ++;
            }
        }

        // normalize tf
        size_t docLen = wikiDoc.pTermIdList->size();
        for(termtf_map_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            iter->second /= docLen;
        }
    }

protected:
    string dataFile_;
    size_t docCount_;
    docid_t curDoc_;

    typedef rde::hash_map<uint32_t, float> termtf_map_t;
    typedef rde::hash_map<uint32_t, float>::iterator termtf_map_iterator_t;

    termtf_map_t term_tf_map_; // for a document
};

NS_IDMLIB_SSP_END

#endif /* WIKIINDEX_H_ */
