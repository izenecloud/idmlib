/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief Explicit Semantic Analysis, refer to
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */
#ifndef IDMLIB_SSP_SEMANTIC_INTERPRETER_H_
#define IDMLIB_SSP_SEMANTIC_INTERPRETER_H_

#include <iostream>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <util/ustring/UString.h>
#include <ir/index_manager/index/IndexReader.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/index_manager/index/LAInput.h>
#include <ir/index_manager/index/Indexer.h>
#include <ir/id_manager/IDManager.h>
#include <idmlib/idm_types.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{

public:
    /**
     * Interpreter for Explicit Semantic Analysis
     * @param pSSpaceWiki inverted index of Wiki concepts resource
     *
     * @deprecated The SemanticSpace with db storage it's quite slow,
     * so it is replaced by a solution which using Indexer of SF1.
     */
	SemanticInterpreter(boost::shared_ptr<SemanticSpace>& pSSpaceWiki)
	: sspWikiIndex_(pSSpaceWiki)
	{
	    conceptNum_ = sspWikiIndex_->getDocNum();
	}

	/**
	 * Interpreter for Explicit Semantic Analysis
	 * @param pIndexer inverted index of Wiki concepts resource
	 */
	SemanticInterpreter(boost::shared_ptr<Indexer>& pWikiIndexer, const string& laResPath, const string& colBasePath)
	:pWikiIndexer_(pWikiIndexer)
	{
	    // Indexer
	    BOOST_ASSERT(pWikiIndexer_);
	    conceptNum_ = pWikiIndexer_->getIndexReader()->numDocs();
	    cout << "[SemanticInterpreter] init, wiki concepts: " << conceptNum_ << endl;

	    // LA
	    pIdmAnalyzer_.reset(
	            new idmlib::util::IDMAnalyzer(laResPath,la::ChineseAnalyzer::minimum_match_no_overlap)
	            );
	    BOOST_ASSERT(pIdmAnalyzer_);

	    laInput_.reset(new TermIdList());

	    // ID Manager
	    string idDir = colBasePath + "/collection-data/default-collection-dir/id";
	    pIdManager_.reset( new izenelib::ir::idmanager::IDManager(idDir) );
	    BOOST_ASSERT(pIdManager_);
	}

	 ~SemanticInterpreter()
	{

	}

public:

	/**
	 * Interpret semantic of a text (document)
	 * @brief maps fragments of natural language text (document) into a interpretation vector
	 * @param[IN] text a input natural language text or a document
	 * @param[OUT] interVector interpretation vector of the input text (document)
	 * @return true on success, false on failure.
	 */
	bool interpret(const UString& doc, std::vector<weight_t>& interVector)
	{
	    // make forward index of doc
	    laInput_->resize(0);
	    pIdmAnalyzer_->GetTermIdList(pIdManager_.get(), doc, *laInput_);

		return true;
	}

	/**
	 * @brief Convert a document from representation vector to interpretation vector of ESA
	 * Given repreDocVec <wi> with weights <vi>, for wi weight to concept cj is kj,
	 * Then interDocVec is <dcj>, where dcj = SUM vi*kj for all wi in <wi>,
	 * dcj is the weight of DOC to CONCEPT cj.
	 * @param[IN] repreDocVec
	 * @param[OUT] interDocVec
	 *
	 * @deprecated may be not used
	 */
	bool interpret(term_sp_vector& repreDocVec, interpretation_vector_type& interDocVec)
	{
		/* Here for performance, did not calculate the weight of a doc to a concept at a time,
		   but incrementally accumulate weights to each concepts by traverse entries of words(terms)
		   of repreDocVec on inverted index of wiki concepts (word-concept), also used a concept-weight
		   map to speed up accumulating. */
		std::map<docid_t, weight_t> conceptWeightMap;

		termid_t termid;
		weight_t wdoc;
		term_sp_vector::ValueType::iterator tIter;
		docid_t conceptId;
		weight_t wcon;
		doc_sp_vector::ValueType::iterator cIter;
		// for all representation words(terms) of the document
		for (tIter = repreDocVec.value.begin(); tIter != repreDocVec.value.end(); tIter ++)
		{
			termid = tIter->first;
			wdoc = tIter->second;

			// for entry of current word(term), calculate sub weights to concepts
			doc_sp_vector consVec;
			sspWikiIndex_->getVectorByTermid(termid, consVec);
			for (cIter = consVec.value.begin(); cIter != consVec.value.end(); cIter ++)
			{
				conceptId = cIter->first;
				wcon = cIter->second;

				if (conceptWeightMap.find(conceptId) != conceptWeightMap.end())
				{
					conceptWeightMap[conceptId] += wdoc * wcon;
				}
				else {
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
                interDocVec.value.push_back(make_pair(cwIter->first, w));
        }

		return true;
	}

private:
	static const weight_t thresholdWegt_ = 0.02;

private:
	boost::shared_ptr<SemanticSpace> sspWikiIndex_;
	count_t conceptNum_;

	count_t topicNum_;

	// wiki indexer
	boost::shared_ptr<Indexer> pWikiIndexer_;
	// LA
	boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
	boost::shared_ptr<TermIdList> laInput_;
	//
	boost::shared_ptr<izenelib::ir::idmanager::IDManager> pIdManager_;
};

typedef SemanticInterpreter ExplicitSemanticInterpreter;

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
