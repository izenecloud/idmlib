/*
 * Segmenter.cc
 *
 *  Created on: Mar 22, 2010
 *      Author: eric
 */
#include <idmlib/dc/LaMgrFactory.h>

namespace idmlib {

la::LA* LaMgrFactory::getLaMgr()
{
	static la::LA* laMgr;
	if (!laMgr)
	{
		laMgr = new la::LA();
		la::TokenizeConfig config;
		config.addUnites("=");
		laMgr->setTokenizerConfig(config);
		boost::shared_ptr<la::Analyzer> analyzer;
		analyzer.reset( new la::NGramAnalyzer(2, 2, 10000000000) );
		static_cast<la::NGramAnalyzer*>(analyzer.get())->setApartFlag(la::NGramAnalyzer::NGRAM_APART_CJK_);
		laMgr->setAnalyzer(analyzer);
	}
	return laMgr;
}

}
