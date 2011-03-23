/**
 * @file idmlib/semantic_space/explicit_semantic_space.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief Explicit Semantic Analysis, refer to
 * Evgeniy Gabrilovich and Shaul Markovitch. (2007).
 * "Computing Semantic Relatedness using Wikipedia-based Explicit Semantic Analysis,"
 * Proceedings of The 20th International Joint Conference on Artificial Intelligence
 * (IJCAI), Hyderabad, India, January 2007.
 */
#ifndef EXPLICIT_SEMANTIC_SPACE_H_
#define EXPLICIT_SEMANTIC_SPACE_H_

#include <set>
#include <cmath>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class ExplicitSemanticSpace : public SemanticSpace
{
public:
	ExplicitSemanticSpace(const std::string& filePath)
	: SemanticSpace(filePath)
	, termCount_(0)
	, docCount_(0)
	{}

public:
	void processDocument(docid_t& docid, term_vector& terms);

	void processSpace();

	count_t getDocNum()
	{
		return docid2Index_.size();
	}

	bool getTermIndex(termid_t& termid, index_t& termidx)
	{
		termid_index_map::iterator iter = termid2Index_.find(termid);
		if (iter != termid2Index_.end()) {
			termidx = iter->second;
			return true;
		}
		else
			return false;
	}

	weight_t getTermDocWeight(termid_t& termid, index_t& docIdx)
	{
		index_t termIdx;
		bool ret = getTermIndex(termid, termIdx);
		if (!ret)
			return 0; // term not existed in wiki index

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
	}

	bool getTermIds(std::set<termid_t>& termIds);

	bool getTermVector(termid_t termId, std::vector<docid_t> termVec);

	void print();


private:
	void buildTermIndex(docid_t& docid, term_vector& terms);

	void buildDocIndex(docid_t& docid, term_vector& terms);

	index_t getOrAddTermIndex(termid_t& termid) {
		termid_index_map::iterator iter = termid2Index_.find(termid);
		if ( iter != termid2Index_.end()) {
			return iter->second;
		}
		else {
			termid2Index_[termid] = termCount_;
			return (termCount_++);
		}
	}

	template<typename mapT, typename mapIterT>
	index_t getOrAddIndex(mapT& id2Index, mapIterT& iter, termid_t& id, index_t& index_count) {
		iter = id2Index.find(id);
		if (iter != id2Index.end()) {
			return iter->second;
		}
		else {
			id2Index[id] = index_count;
			return (index_count++);
		}
	}

	void calcWeight()
	{
		boost::shared_ptr<sDocUnit> pDoc;
		count_t doc_cnt = docid2Index_.size();

		for (termid_t t = 0; t < termdocM_.size(); t ++) {
			for (docid_t dt = 0; dt < termdocM_[t].size(); dt ++) {
				pDoc = termdocM_[t][dt];
				if (pDoc) {
					// weight = tf * idf, std::log() = ln()
					pDoc->tf = pDoc->tf * std::log((weight_t)doc_cnt / termidx2DF_[t]);
				}
			}
		}
	}

private:
	static const count_t thresholdTF_ = 3; // TF >
	static const count_t thresholdDF_ = 1000; // < ?
	static const weight_t thresholdWegt_ = 0.0f;

private:
	index_t termCount_; // term(row) index
	index_t docCount_;  // doc(column) index
	termid_index_map termid2Index_;
	docid_index_map docid2Index_;

	typedef std::map<index_t, count_t> term_df_map;
	term_df_map termidx2DF_;

	term_doc_matrix termdocM_; // in memory
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_SPACE_H_ */
