/**
 * @file idmlib/semantic_space/document_vector_space.h
 * @author Zhongxia Li
 * @date Mar 23, 2011
 * @brief
 */

#ifndef DOCUMENT_VECTOR_SPACE_H_
#define DOCUMENT_VECTOR_SPACE_H_

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>

NS_IDMLIB_SSP_BEGIN

class DocumentVectorSpace : public SemanticSpace
{
public:
	DocumentVectorSpace(const std::string& sspPath)
	: sspPath_(sspPath)
	, SemanticSpace(sspPath)
	{

	}

	void ProcessDocument(docid_t& docid, std::vector<termid_t>& termids) {};

	void ProcessSpace() {}

private:
	std::string sspPath_;

};

NS_IDMLIB_SSP_END

#endif /* DOCUMENT_VECTOR_SPACE_H_ */
