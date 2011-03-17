/**
 * @file document_similarity.h
 * @author Zhongxia Li
 * @date Mar 11, 2011
 * @brief Document Similarity Compute and Index
 */
#ifndef DOCUMENT_SIMILARITY_H_
#define DOCUMENT_SIMILARITY_H_

#include <iostream>
#include <string>

#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>

using namespace idmlib::ssp;

NS_IDMLIB_SIM_BEGIN

class DocumentSimilarity
{
public:
	DocumentSimilarity(const boost::shared_ptr<SemanticSpace> pSSP, const std::string& docSimDir)
	: pSSP_(pSSP)
	, docSimDir_(docSimDir)
	{
	}

	~DocumentSimilarity()
	{
	}

public:
	bool buildSimIndex();

	bool Load();

	bool GetSimDocIdList(
			uint32_t docId,
			uint32_t maxNum,
			std::vector<std::pair<uint32_t, float> >& result);

private:
	boost::shared_ptr<SemanticSpace> pSSP_;

	std::string resDir_;

	std::string docSimDir_;
};

NS_IDMLIB_SIM_END

#endif /* DOCUMENT_SIMILARITY_H_ */
