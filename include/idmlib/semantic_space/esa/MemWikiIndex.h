/**
 * @file MemWikiIndex.h
 * @author Zhongxia Li
 * @date Jun 2, 2011
 * @brief 
 */
#ifndef MEM_WIKI_INDEX_H_
#define MEM_WIKI_INDEX_H_

#include <idmlib/idm_types.h>

#include "WikiIndex.h"

#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

NS_IDMLIB_SSP_BEGIN

/**
 * Memory resident inverted index
 * @brief To ensure the performance of ESA, the inverted index of Wikipeida should be
 * kept in memory, in case the heavy i/o overhead of a disk resident index.
 * The memory resident inverted index can be entirely saved to (or loaded from) disk.
 */
class MemWikiIndex : public WikiIndex
{
public:
    void insertDocument(WikiDoc& wikiDoc)
    {
        docCount_ ++;
        curDoc_ = wikiDoc.docid;

        gatherTF_(wikiDoc);

        updateInvertIndex_();
    }

    void postProcess()
    {
        calcWeight_();

        flush();
    }

    void load()
    {
        clear();
        loadIndex_();
    }

private:
    void updateInvertIndex_()
    {
        termid_t termid;
        float w;
        for(termtf_map_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
        {
            termid = iter->first;
            w = iter->second; // tf

            invertedlists_iterator_t ret = invertedLists_.find(termid);
            if (ret == invertedLists_.end())
            {
                // term has not been indexed yet
                boost::shared_ptr<InvertedList> invertList(new InvertedList(termid));
                invertList->insertDoc(curDoc_, w);
                invertedLists_.insert(invertedlists_t::value_type(termid, invertList));
            }
            else
            {
                // term has been indexed
                ret->second->insertDoc(curDoc_, w);
            }
        }
    }

    /**
     * Calculate term-concept weights over all wiki index using TFIDF scheme.
     */
    void calcWeight_()
    {
        /// todo filt
        float idf;
        for (invertedlists_iterator_t it = invertedLists_.begin(); it != invertedLists_.end(); it++)
        {
            idf = std::log((float)docCount_ / it->second->len);

            std::vector<InvertItem>::iterator iit;
            for (iit = it->second->list.begin(); iit != it->second->list.end(); iit++)
            {
                iit->weight *= idf; // tf * idf
            }
        }
    }

    void flush()
    {
        // file is opened in append mode, so remove it firstly.
        boost::filesystem::remove(dataFile_);

        // save whole inverted index
        std::ofstream fout(dataFile_.c_str(), ios::app);
        boost::archive::text_oarchive oa(fout);

        size_t count = invertedLists_.size();
        oa << count;

        for (invertedlists_iterator_t it = invertedLists_.begin(); it != invertedLists_.end(); it++)
        {
            oa << *it->second;
        }

        fout.close();
    }

    void clear()
    {
        init();

        invertedLists_.clear();
    }

    void loadIndex_()
    {
        std::ifstream fin(dataFile_.c_str());
        boost::archive::text_iarchive ia(fin);

        // read list count
        size_t count;
        ia >> count;

        for (size_t i = 0; i < count; i++)
        {
            boost::shared_ptr<InvertedList> invertedList(new InvertedList());
            ia >> *invertedList;
            invertedLists_.insert(invertedlists_t::value_type(invertedList->termid, invertedList));
        }
        fin.close();
    }

public:
    /// for test
    void printWikiIndex()
    {
        cout << "---- Wiki Index -----" <<endl;
        for (invertedlists_iterator_t it = invertedLists_.begin(); it != invertedLists_.end(); it++)
        {
            cout << "["<<it->second->termid << " " << it->second->len << "] --> ";
            std::vector<InvertItem>::iterator iit;
            for (iit = it->second->list.begin(); iit != it->second->list.end(); iit++)
            {
                cout << "(" << iit->conceptId <<","<<iit->weight<<") ";
            }
            cout << endl;
        }
    }

private:

    struct InvertItem
    {
        uint32_t conceptId;
        float weight;

        InvertItem() {}
        InvertItem(uint32_t conceptId, float w)
        : conceptId(conceptId), weight(w) {}

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & conceptId;
            ar & weight;
        }
    };

    struct InvertedList
    {
        uint32_t termid;
        uint32_t len; ///< list length, or Document Frequence(DF).
        std::vector<InvertItem> list;

        InvertedList() {}
        InvertedList(uint32_t termid, size_t size=0)
        : termid(termid), len(0)
        {
            //list.resize(size); //xxx
        }

        void insertDoc(uint32_t docid, float w)
        {
            len ++;
            list.push_back(InvertItem(docid, w));
        }

        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & termid;
            ar & len;
            ar & list;
        }
    };

private:
    typedef rde::hash_map<uint32_t, boost::shared_ptr<InvertedList> > invertedlists_t;
    typedef rde::hash_map<uint32_t, boost::shared_ptr<InvertedList> >::iterator invertedlists_iterator_t;

    invertedlists_t invertedLists_;
};

NS_IDMLIB_SSP_END

#endif /* MEM_WIKI_INDEX_H_ */
