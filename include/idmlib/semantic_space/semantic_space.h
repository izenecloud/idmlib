/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief Vector space base
 */
#ifndef SEMANTIC_SPACE_H_
#define SEMANTIC_SPACE_H_

#include <set>
#include <iostream>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class SemanticSpace
{
public:
	SemanticSpace(const std::string& filePath)
	{
	}
	virtual ~SemanticSpace() {}

public:
	virtual void processDocument(docid_t& docid, term_vector& terms) = 0;

	virtual void processSpace() = 0;

	virtual count_t getDocNum() { return 0; } ;

	virtual bool getTermIndex(termid_t& termid, index_t& termidx) { return false; }

	virtual weight_t getTermDocWeight(termid_t& termid, index_t& docIdx) { return 0; };

	virtual bool getTerms(std::set<std::string>& termSet) { return false; }

	virtual bool getTermIds(std::set<termid_t>& termIds) {};

	virtual bool getTermVector(termid_t termId, std::vector<docid_t> termVec) {};

	// todo, SparseVector
	virtual bool getTermSparseVector(termid_t termId, std::vector<docid_t> termSparseVec) { return false; }

};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_SPACE_H_ */
