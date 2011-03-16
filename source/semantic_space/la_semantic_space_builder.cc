#include <idmlib/semantic_space/la_semantic_space_builder.h>
#include <util/scd_parser.h>


using namespace idmlib::ssp;

bool LaSemanticSpaceBuilder::getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec)
{
	pTermIdList_.clear();
	pLA_->process(pIdManager_.get(), ustrDoc, pTermIdList_);

	for ( TermIdList::iterator iter = pTermIdList_.begin(); iter != pTermIdList_.end(); iter++ )
	{
		boost::shared_ptr<sTermUnit> pTerm(new sTermUnit());
		pTerm->termid = iter->termid_;
		//cout << iter->termid_ << "  " << iter->docId_ << endl;
		termVec.push_back(pTerm);
	}

	return true;
}


/// Private

bool LaSemanticSpaceBuilder::initLA(const std::string& laResPath)
{
	pLA_.reset(new la::LA());
	boost::shared_ptr<la::MultiLanguageAnalyzer> ml_analyzer( new la::MultiLanguageAnalyzer() );

	boost::shared_ptr<la::Analyzer> ch_analyzer( new la::ChineseAnalyzer(laResPath, false) );
	la::ChineseAnalyzer* pch = dynamic_cast<la::ChineseAnalyzer*>(ch_analyzer.get());
	if (!pch) {
		return false;
	}
	pch->setExtractSpecialChar(false, true);
	pch->setAnalysisType(la::ChineseAnalyzer::minimum_match_no_overlap);

	ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::CHINESE, ch_analyzer );
	pLA_->setAnalyzer(ml_analyzer);

	return true;
}
