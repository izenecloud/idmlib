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
#include <boost/bind.hpp>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarityIndex
{
public:
    enum DocSimInitType {
        CREATE,
        LOAD
    };

public:
	DocumentSimilarityIndex(
	        const std::string& docSimPath,
	        weight_t thresholdSim = 0.0f,
	        DocSimInitType initType = CREATE)
	: docSimPath_(docSimPath)
	, thresholdSim_(thresholdSim)
	{
		idmlib::util::FSUtil::normalizeFilePath(docSimPath_);

		if (initType == CREATE) {
		    idmlib::util::FSUtil::del(docSimPath_);
		}
		conceptDocIndex_.reset(new doc_doc_matrix_file_io(docSimPath_));
		//conceptDocIndex_->setCacheSize(10000);
		conceptDocIndex_->Open();
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
	void InertDocument(docid_t& docid, interpretation_vector_type& interDocVec)
	{
#ifdef SSP_BUIDER_TEST
		idmlib::util::TimeChecker timer("DocumentSimilarityIndex::InertDocument");
#endif
		basicInvertedIndexJoin_(docid, interDocVec);

	}

	void FinishInert()
	{
		conceptDocIndex_->Flush();
		buildSimIndex();
	}

private:
	/**
	 * @brief a basic Inverted Index Join approach
	 */
	void basicInvertedIndexJoin_(docid_t& docid, interpretation_vector_type& interDocVec);

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
	std::string docSimPath_;
	boost::shared_ptr<doc_doc_matrix_file_io> conceptDocIndex_; // inverted index

	weight_t thresholdSim_; // threshold

	// test, todo
	std::map<docid_t, std::vector<pair<docid_t, weight_t> > > simDocIndex_;

};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_INDEX_H_ */
