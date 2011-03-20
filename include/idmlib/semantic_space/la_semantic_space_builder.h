/**
 * @file la_semantic_space_builder.h
 * @author Zhongxia Li
 * @date Mar 15, 2011
 * @brief 
 */
#ifndef LA_SEMANTIC_SPACE_BUILDER_H_
#define LA_SEMANTIC_SPACE_BUILDER_H_

#include <la/LA.h>
#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space_builder.h>
#include <idmlib/semantic_space/term_doc_matrix_defs.h>
#include <idmlib/util/idm_analyzer.h>
#include <ir/index_manager/index/LAInput.h>

using namespace izenelib::ir::indexmanager;

NS_IDMLIB_SSP_BEGIN

class LaSemanticSpaceBuilder : public SemanticSpaceBuilder
{
public:
	LaSemanticSpaceBuilder(
			const std::string& collectionPath,
			boost::shared_ptr<SemanticSpace>& pSSpace,
			const std::string& laResPath,
			docid_t maxDoc = MAX_DOC_ID)
	: SemanticSpaceBuilder(collectionPath, pSSpace, maxDoc)
	, laResPath_(laResPath)
	{
		// collectionPath_ is normalized file path
		indexPath_ = collectionPath_ + "";
		normalizeFilePath(laResPath_);

		//initLA(laResPath_);
		pIdmAnalyzer_.reset(
				new idmlib::util::IDMAnalyzer(
						laResPath,
						la::ChineseAnalyzer::minimum_match_no_overlap)
		);
	}

	bool getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec);

private:
	bool initLA(const std::string& laResPath);

	bool filter(const izenelib::util::UString& ustrTerm);

private:
	std::string indexPath_;
	std::string laResPath_;

	boost::shared_ptr<la::LA> pLA_;
	boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
	TermIdList termIdList_;
	la::TermList termList_;
};

NS_IDMLIB_SSP_END

#endif /* LA_SEMANTIC_SPACE_BUILDER_H_ */
