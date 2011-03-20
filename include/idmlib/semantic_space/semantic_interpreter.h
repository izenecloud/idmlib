/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief 
 */
#ifndef IDMLIB_SSP_SEMANTIC_INTERPRETER_H_
#define IDMLIB_SSP_SEMANTIC_INTERPRETER_H_

#include <iostream>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <util/ustring/UString.h>
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/semantic_space_builder.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{
	typedef std::map<termid_t, weight_t> term_map;

public:
	SemanticInterpreter(
			boost::shared_ptr<SemanticSpace>& pSSpace,
			boost::shared_ptr<SemanticSpaceBuilder>& pSSPBuilder)
	: pSSpace_(pSSpace)
	, pSSPBuilder_(pSSPBuilder)
	{
		topicNum_ = pSSpace_->getDocNum();
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
	bool interpret(UString& text, std::vector<weight_t>& interVector)
	{
		if (!pSSpace_)
			return false;
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
				weight_t w = pSSpace_->getWegtTermDoc(termid, docIdx);
				if (w > 0.000001f)
					wTopic += iter->second * w;
			}

			interVector[docIdx] = wTopic;
		}


		return true;
	}

private:
	void getDocTerms(term_vector& termVec, term_map& termMap)
	{
		term_map::iterator miter;
		for (term_vector::iterator iter = termVec.begin(); iter != termVec.end(); iter++ )
		{
			//cout << iter->get()->termid << " - ";
			miter = termMap.find(iter->get()->termid);
			if (miter != termMap.end())
			{
				miter->second += 1;
			}
			else {
				termMap[iter->get()->termid] = 1;
			}
		}
	}

private:
	boost::shared_ptr<SemanticSpace> pSSpace_;
	boost::shared_ptr<SemanticSpaceBuilder> pSSPBuilder_;
	count_t topicNum_;

	term_map termMap_; // <termid, tf>
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
