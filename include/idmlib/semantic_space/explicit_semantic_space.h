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
#include <vector>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class ExplicitSemanticSpace : public SemanticSpace
{
public:
	ExplicitSemanticSpace()
	{}

public:
	void processDocument(doc_terms_map& doc);

	void processSpace();

	bool getTermIds(std::set<termid_t>& termIds);

	bool getTermVector(termid_t termId, std::vector<docid_t> termVec);
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_SPACE_H_ */
