#include <idmlib/semantic_space/la_semantic_space_builder.h>
#include <util/scd_parser.h>


using namespace idmlib::ssp;

bool LaSemanticSpaceBuilder::getDocTerms(const izenelib::util::UString& ustrDoc, term_vector& termVec)
{
	//termIdList_.clear();
	//pLA_->process(pIdManager_.get(), ustrDoc, termIdList_);

	termList_.clear();
	//pLA_->process(ustrDoc, termList_);
	pIdmAnalyzer_->GetTermList(ustrDoc, termList_, false);

	termid_t termid;
	for ( la::TermList::iterator iter = termList_.begin(); iter != termList_.end(); iter++ )
	{
		if ( filter(iter->text_) )
			continue;

		boost::shared_ptr<sTermUnit> pTerm(new sTermUnit());
		pIdManager_->getTermIdByTermString(iter->text_, termid);
		pTerm->termid = termid;
		termVec.push_back(pTerm);

		//cout << iter->textString()  << "(" << termid << ") "; //
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
	pch->setExtractSpecialChar(true, true);
	pch->setAnalysisType(la::ChineseAnalyzer::minimum_match_no_overlap);

	ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::CHINESE, ch_analyzer );
	ml_analyzer->setDefaultAnalyzer( ch_analyzer );
	pLA_->setAnalyzer(ml_analyzer);

	return true;
}

bool LaSemanticSpaceBuilder::filter(const izenelib::util::UString& ustrTerm)
{
	return false;
}
