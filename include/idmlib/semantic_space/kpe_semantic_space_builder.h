/**
 * @file kpe_semantic_space_builder.h
 * @author Zhongxia Li
 * @date Mar 14, 2011
 * @brief 
 */
#ifndef KPE_SEMANTIC_INTERPRETER_H_
#define KPE_SEMANTIC_INTERPRETER_H_

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space_builder.h>

NS_IDMLIB_SSP_BEGIN

class KpeSemanticSpaceBuilder : public SemanticSpaceBuilder
{
public:
	KpeSemanticSpaceBuilder(
			const std::string& scdPath,
			const std::string& outPath,
			boost::shared_ptr<SemanticSpace>& pSSpace )
	: SemanticSpaceBuilder(scdPath, outPath, pSSpace)
	{
	}

public:
	bool buildInvertedIndex();

private:
	bool getScdFileList(const std::string& scdPath, std::vector<std::string>& fileList);

};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_INTERPRETER_H_ */
