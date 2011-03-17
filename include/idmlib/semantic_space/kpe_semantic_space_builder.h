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
#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <idmlib/keyphrase-extraction/kpe_output.h>

NS_IDMLIB_SSP_BEGIN

class KpeSemanticSpaceBuilder : public SemanticSpaceBuilder
{
	typedef idmlib::kpe::KPEOutput<true, true, true> OutputType ;
	typedef OutputType::function_type function_type ;
public:
	typedef idmlib::kpe::KPEAlgorithm<OutputType> kpe_type;

public:
	KpeSemanticSpaceBuilder(
			const std::string& collectionPath,
			const std::string& outPath,
			boost::shared_ptr<SemanticSpace>& pSSpace )
	: SemanticSpaceBuilder(collectionPath, outPath, pSSpace)
	{
	}

protected:
	bool getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec);

private:
	bool initKpe();

private:
	// key-phrase extraction
	kpe_type* kpe_;
    function_type callback_func_;
    kpe_type::ScorerType* scorer_;

};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_INTERPRETER_H_ */
