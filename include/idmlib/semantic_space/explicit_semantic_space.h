/**
 * @file idmlib/semantic_space/explicit_semantic_space.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @breif For Explicit Semantic Analysis
 * inverted index(word-concept, or term-doc) of Wikipedia for
 * explicit semantic interpreting.
 */
#ifndef EXPLICIT_SEMANTIC_SPACE_H_
#define EXPLICIT_SEMANTIC_SPACE_H_

#include <set>
#include <cmath>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <util/file_object.h>

using namespace izenelib::am;

NS_IDMLIB_SSP_BEGIN

class ExplicitSemanticSpace : public SemanticSpace
{
public:
	ExplicitSemanticSpace(
			const std::string& sspPath,
			SemanticSpace::eSSPInitType initType = SemanticSpace::CREATE)
	: SemanticSpace(sspPath, initType)
	{
		pTermConceptIndex_.reset(new term_doc_matrix_file_oi(sspPath_));
		pTermConceptIndex_->Open();

#ifdef RESET_MATRIX_INDEX
		termCount_ = MATRIX_INDEX_START;
		docCount_ = MATRIX_INDEX_START;
#endif

	}

public:
	/// @brief Incrementally process every document
	void ProcessDocument(docid_t& docid, std::vector<termid_t>& termids)
	{
		docList_.push_back(docid);
		doProcessDocument(docid, termids);
	}

	/// @brief Post process after all documents are processed
	void ProcessSpace()
	{
		calcWeight();
		SaveSpace();
	}

	void SaveSpace()
	{
		SemanticSpace::SaveSpace();
		pTermConceptIndex_->Flush();
	}

public:
	count_t getDocNum()
	{
		return docList_.size();
	}

	void getVectorByTermid(termid_t& termid, doc_sp_vector& docsVec)
	{
		pTermConceptIndex_->GetVector(termid, docsVec);
	}


	bool getTermIndex(termid_t& termid, index_t& termidx)
	{
#ifdef RESET_MATRIX_INDEX
		termid_index_map::iterator iter = termid2Index_.find(termid);
		if (iter != termid2Index_.end()) {
			termidx = iter->second;
			return true;
		}
		else
			return false;
#else
		termidx = termid;
		return true;
#endif
	}

	weight_t getTermDocWeight(termid_t& termid, index_t& docIdx)
	{
		index_t termIdx;
		bool ret = getTermIndex(termid, termIdx);
		if (!ret)
			return 0; // term not existed in wiki index
/*
		if (docIdx >= docid2Index_.size()) {
			DLOG(WARNING) << "No doc index " << docIdx << endl;
			return 0;
		}
		cout << "termid: " << termid << " term-doc-index: [" << termIdx << ", " << docIdx << "]" << endl;
		doc_vector& docVec =termdocM_[ termIdx ];
		if (docIdx >= docVec.size())
			   return 0;
		boost::shared_ptr<sDocUnit>& pDocUnit = docVec[ docIdx ];

		if (pDocUnit.get())
			return pDocUnit->tf; // weight = tf*idf
		else
			return 0;
			*/
	}

	bool getTermIds(std::set<termid_t>& termIds);

	bool getTermVector(termid_t termId, std::vector<docid_t> termVec);

	void Print();


private:

	index_t getIndexFromTermId(termid_t termid) {
#ifdef RESET_MATRIX_INDEX
		termid_index_map::iterator iter = termid2Index_.find(termid);
		if ( iter != termid2Index_.end()) {
			return iter->second;
		}
		else {
			termid2Index_[termid] = termCount_;
			return (termCount_++);
		}
#else
		return termid;
#endif
	}

	index_t getIndexFromDocId(termid_t& docid) {
#ifdef RESET_MATRIX_INDEX
		docid_index_map::iterator iter = docid2Index_.find(docid);
		if ( iter != docid2Index_.end()) {
			return iter->second;
		}
		else {
			docid2Index_[termid] = docCount_;
			return (docCount_++);
		}
#else
		return docid;
#endif
	}

	void doProcessDocument(docid_t& docid, std::vector<termid_t>& termids);

	void updateTermConceptIndex(termid_t& term_index, docid_t& doc_index, weight_t& weight)
	{
		doc_sp_vector docVec;
		pTermConceptIndex_->GetVector(term_index, docVec);
		docVec.value.push_back(std::make_pair(doc_index, weight));
		pTermConceptIndex_->SetVector(term_index, docVec);
	}

	void calcWeight();


private:
	static const weight_t thresholdWegt_ = 0.0f; // threshold weight of term to concepts

private:
#ifdef RESET_MATRIX_INDEX
	index_t termCount_; // term(row) index
	index_t docCount_;  // doc(column) index
	termid_index_map termid2Index_;
	docid_index_map docid2Index_;
#endif

	// inverted index of Wikipedia concepts, permanent
	boost::shared_ptr<term_doc_matrix_file_oi> pTermConceptIndex_;
	//term_doc_matrix termdocM_;

	// statistics of a document, temporary
	typedef std::map<termid_t, weight_t> term_doc_tf_map;
	term_doc_tf_map termid2doctf_;
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_SPACE_H_ */
