/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief
 */
#ifndef SEMANTIC_SPACE_H_
#define SEMANTIC_SPACE_H_

#include <set>
#include <vector>
#include <iostream>

#include <idmlib/idm_types.h>

NS_IDMLIB_SSP_BEGIN

typedef uint32_t termid_t;
typedef uint32_t docid_t;
typedef uint32_t count_t;

class SemanticSpace
{
public:
	SemanticSpace()
	{
	}
	virtual ~SemanticSpace() {}

public:
	virtual void processDocument(/*document*/) {};

	virtual void processSpace() = 0;

	virtual bool getTerms(std::set<std::string>& termSet) { return false; }

	virtual bool getTermIds(std::set<termid_t>& termIds) = 0;

	virtual bool getTermVector(termid_t termId, std::vector<docid_t> termVec) = 0;

	// todo, SparseVector
	virtual bool getTermSparseVector(termid_t termId, std::vector<docid_t> termSparseVec) { return false; }

protected:
	// KPE

	// Matrix
	// MatrixBuilder
	// termIndexMap<string, I>
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_SPACE_H_ */
