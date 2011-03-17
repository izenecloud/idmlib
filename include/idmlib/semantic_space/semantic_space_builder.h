/**
 * @file semantic_space_builder.h
 * @author Zhongxia Li
 * @date Mar 14, 2011
 * @brief 
 */
#ifndef SEMANTIC_SPACE_BUILDER_H_
#define SEMANTIC_SPACE_BUILDER_H_

#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>

NS_IDMLIB_SSP_BEGIN

class SemanticSpaceBuilder
{
public:
	SemanticSpaceBuilder(
			const std::string& scdPath,
			const std::string& outPath,
			boost::shared_ptr<SemanticSpace>& pSSpace )
	: scdPath_(scdPath)
	, outPath_(outPath)
	, pSSpace_(pSSpace)
	{

	}

	virtual ~SemanticSpaceBuilder()
	{

	}

public:
	virtual bool buildInvertedIndex() = 0;

	boost::shared_ptr<SemanticSpace>& getSemanticSpace()
	{
		return pSSpace_;
	}

protected:
	std::string scdPath_;
	std::string outPath_;
	boost::shared_ptr<SemanticSpace> pSSpace_;
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_SPACE_BUILDER_H_ */
