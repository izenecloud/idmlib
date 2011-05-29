/**
 * @file document_similarity_index.h
 * @author Zhongxia Li
 * @date Mar 23, 2011
 * @brief For computing document similarity and building index
 */
#ifndef DOCUMENT_SIMILARITY_INDEX_H_
#define DOCUMENT_SIMILARITY_INDEX_H_

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/explicit_semantic_interpreter.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <am/leveldb/Table.h>
#include <cache/IzeneCache.h>
#include <boost/bind.hpp>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarityIndex
{
	typedef izenelib::am::tc_hash<docid_t, doc_sp_vector> IndexStorageType;
	//typedef izenelib::am::leveldb::Table<docid_t, InterpretVector > IndexStorageType;

    typedef izenelib::cache::IzeneCache<
    		docid_t,
			boost::shared_ptr<doc_sp_vector >,
			izenelib::util::ReadWriteLock,
			izenelib::cache::RDE_HASH,
			izenelib::cache::LFU
    > RowCacheType;

public:
    enum DocSimInitType {
        CREATE,
        LOAD
    };

    enum StorageType {
        DB_TOKYO_CABINET,
        DB_LevelDB,
    };

public:
	DocumentSimilarityIndex(
	        const std::string& docSimPath,
	        weight_t thresholdSim = 0.0f,
	        StorageType storageType = DB_TOKYO_CABINET,
	        DocSimInitType initType = CREATE)
	: docSimPath_(docSimPath)
	, storageType_(storageType)
	, thresholdSim_(thresholdSim)
	, row_cache_(100)
	{
		idmlib::util::FSUtil::normalizeFilePath(docSimPath_);

		if (initType == CREATE) {
		    idmlib::util::FSUtil::del(docSimPath_);
		}

		if (storageType == DB_TOKYO_CABINET)
		{
            index_tc_.reset(new doc_doc_matrix_file_io(docSimPath_));
            //index_tc_->setCacheSize(10000);
            index_tc_->Open();
		}
		else if (storageType == DB_LevelDB)
		{
            ;
		}
	}

public:
	/**
	 * @brief Cosine similarity join
	 * We use consine similarity metric, and incrementally building an inverted index
	 * to do <b>All Pair</b> similarity search for efficiency.
	 *
	 * Reference:
	 * [1] Bayardo, R.J., Ma, Y., Srikant, R.: Scaling up all pairs similarity search.
	 * In: WWW(2007) 436 D. Lee et al.
	 * [2] Dongjoo Lee, Jaehui Park, Junho Shim, Sang-goo Lee. An Efficient Similarity
	 * Join Algorithm with Cosine Similarity Predicate. In DEXA (2)(2010)
	 *
	 */

	/**
	 * @deprecated
	 */
	void InertDocument(docid_t& docid, interpretation_vector_type& interDocVec)
	{
#ifdef SSP_TIME_CHECKER
		idmlib::util::TimeChecker timer("DocumentSimilarityIndex::InertDocument");
#endif
		basicInvertedIndexJoin_(docid, interDocVec);

	}

	void InertDocument(docid_t& docid, InterpretVector& interVec)
	{
	    basicInvertedIndexJoin_(docid, interVec);
	}

	void FinishInert()
	{
	    if (storageType_ == DB_TOKYO_CABINET) {
	        index_tc_->Flush();
	    }
	    else if (storageType_ == DB_LevelDB) {
	    	index_.flush();
	    	index_.close();
	    }
//		buildSimIndex();
	}

private:
	/**
	 * @brief a basic Inverted Index Join approach
	 * @deprecated
	 */
	void basicInvertedIndexJoin_(docid_t& docid, interpretation_vector_type& interDocVec);

	/**
	 * @brief a basic Inverted Index Join approach
	 * @param docid
	 * @param interVec
	 */
	void basicInvertedIndexJoin_(docid_t& docid, InterpretVector& interVec);

	void accumulate_weight_tc_(std::map<docid_t, weight_t>& docWegtMap, docid_t& docid, docid_t& conceptId, weight_t& conW);

	void accumulate_weight_(std::map<docid_t, weight_t>& docWegtMap, docid_t& docid, docid_t& conceptId, weight_t& conW);

	/**
	 * @brief
	 */
	void MMJoin_(docid_t& docid, interpretation_vector_type& interDocVec)
	{}

private:
	void buildSimIndex()
	{
	    /// test print,  todo persistence
	    std::map<docid_t, std::vector<pair<docid_t, weight_t> > >::iterator miter;
	    for (miter = simDocIndex_.begin(); miter != simDocIndex_.end(); miter++) {
	        cout << "[SIM] [" << miter->first << "] ";
	        std::vector<pair<docid_t, weight_t> >& docList = miter->second;
	        std::sort(docList.begin(),docList.end(), sort_second());

	        std::vector<pair<docid_t, weight_t> >::iterator viter;
	        for (viter = docList.begin(); viter != docList.end(); viter++) {
	            cout << "(" << viter->first << "," <<viter->second << ") ";
	        }
	        cout << endl;
	    }
	}

	void outputSimDocPair(docid_t doc1, docid_t doc2, weight_t weight)
	{
        /// test
        if (simDocIndex_.find(doc1) != simDocIndex_.end())
        {
            if (simDocIndex_[doc1].size() >= 20) // max docs
                return;
            simDocIndex_[doc1].push_back(std::make_pair(doc2, weight));
        }
        else {
            std::vector<pair<docid_t, weight_t> > docVec;
            docVec.push_back(std::make_pair(doc2, weight));
            simDocIndex_.insert(std::make_pair(doc1, docVec));
        }
	}

private:
	bool getConceptIndex(docid_t conceptId, InterpretVector& interVec)
	{
		doc_sp_vector conceptIndexVec;
		bool ret;
        if (storageType_ == DB_TOKYO_CABINET) {
        	ret = index_tc_->GetVector(conceptId, conceptIndexVec);
        }
        else if (storageType_ == DB_LevelDB) {
        	ret = index_.get(conceptId, conceptIndexVec);
        }

    	if (ret)
    		interVec = conceptIndexVec.value;
    	return ret;
	}

    void updateConceptIndex(docid_t conceptId, InterpretVector& interVec)
    {
//        boost::shared_ptr<InterpretVector > rowdata;
//        if (!row_cache_.getValueNoInsert(item, rowdata))
//        {
//            rowdata = loadRow(conceptId);
//            row_cache_.insertValue(conceptId, rowdata);
//        }

    	doc_sp_vector conceptIndexVec;
    	conceptIndexVec.value = interVec;

        if (storageType_ == DB_TOKYO_CABINET) {
        	index_tc_->SetVector(conceptId, conceptIndexVec);
        }
        else if (storageType_ == DB_LevelDB) {
        	index_.update(conceptId, conceptIndexVec);
        }
    }

//    bool getRow(docid_t conceptId, InterpretVector& interVec)
//    {
//        return index_.get(conceptId, interVec);
//    }
//
//    void saveRow(docid_t conceptId, InterpretVector& interVec)
//    {
//    	index_.update(conceptId, *rowdata);
//    }

private:
	std::string docSimPath_;

	StorageType storageType_;

	// inverted index for score accumulation of all pair similarity search,
	// in different storage types
	boost::shared_ptr<doc_doc_matrix_file_io> index_tc_; // TOKYO CABINET DB
	IndexStorageType index_;

    RowCacheType row_cache_;

	weight_t thresholdSim_; // threshold

	// test, todo
	std::map<docid_t, std::vector<pair<docid_t, weight_t> > > simDocIndex_;

};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_INDEX_H_ */
