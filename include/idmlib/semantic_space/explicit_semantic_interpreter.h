/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief Explicit Semantic Analysis, refer to
 *
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
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/document_vector_space.h>
#include <idmlib/semantic_space/semantic_space_builder.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{
	//typedef std::map<termid_t, weight_t> term_map;

public:

	SemanticInterpreter(boost::shared_ptr<SemanticSpace>& pSSpaceWiki)
	: sspWikiIndex_(pSSpaceWiki)
	{
	    conceptNum_ = sspWikiIndex_->getDocNum();
	}

	 ~SemanticInterpreter()
	{

	}

public:

	/**
	 * Interpret semantic of a text (document)
	 * maps fragments of natural language text (document) into
	 * a interpretation vector
	 * @param[IN] text a input natural language text or a document
	 * @param[OUT] interVector interpretation vector of the input text (document)
	 * @return true on success, false on failure.
	 */
	bool interpret(const UString& text, std::vector<weight_t>& interVector)
	{
//		if (!pSSpace_)
//			return false;
		/*
		term_vector termVec;
		pSSPBuilder_->getDocTerms(text, termVec);

		termMap_.clear();
		getDocTerms(termVec, termMap_);

		interVector.resize(topicNum_, 0.0f);
		for (index_t docIdx = 0; docIdx < topicNum_; docIdx ++)
		{
			weight_t wTopic = 0.0f; // weight of text to topic with index 'docIdx'
			termid_t termid;
			for (term_map::iterator iter = termMap_.begin(); iter != termMap_.end(); iter ++)
			{
				termid = iter->first;
				weight_t w = pSSpace_->getTermDocWeight(termid, docIdx);
				cout << w << endl;
				if (w > 0.000001f)
					wTopic += iter->second * w; // iter->second(weight) should be tf.idf
			}

			interVector[docIdx] = wTopic;
		}
*/

		return true;
	}

	/// Remove
	bool interpret(boost::shared_ptr<SemanticSpace>& pDocSSpace)
	{
		std::vector<docid_t>& docList = pDocSSpace->getDocList();

		// interpret all documents
		std::vector<docid_t>::iterator docIter;
		for (docIter = docList.begin(); docIter != docList.end(); docIter ++) {
			term_sp_vector representDocVec;
			pDocSSpace->getVectorByDocid(*docIter, representDocVec);

			// interpret a document
			interpretation_vector_type interpretationDocVec;
			interpret(representDocVec, interpretationDocVec);
			stringstream ss;
			ss << "interpretation vector(" << *docIter << ")";
			idmlib::ssp::PrintSparseVec(interpretationDocVec, ss.str());
		}

		return true;
	}

	/**
	 * @brief Convert a document from representation vector to interpretation vector of ESA
	 * Given repreDocVec <wi> with weights <vi>, for wi weight to concept cj is kj,
	 * Then interDocVec is <dcj>, where dcj = SUM vi*kj for all wi in <wi>,
	 * dcj is the weight of DOC to CONCEPT cj.
	 * @param[IN] repreDocVec
	 * @param[OUT] interDocVec
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

	//term_map termMap_; // <termid, tf>
};

typedef SemanticInterpreter ExplicitSemanticInterpreter;

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
