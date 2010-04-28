/*
 * LogProcessor.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/LogProcessor.h>
#include <LA.h>

namespace idmlib
{

LogProcessor::LogProcessor(const std::string& path)
{
	backgroundLangModel_.reset(new LANG_MODEL_TYPE(path+"/back_lang.model"));
	backgroundLangModel_->open();
}

LogProcessor::~LogProcessor()
{
	// TODO Auto-generated destructor stub
}

//HashFunction<wiselib::UString>::generateHash64(gram);

bool LogProcessor::initializeBackGroundLangModel(const boost::shared_ptr<LANG_MODEL_TYPE>& langModel)
{
	backgroundLangModel_=langModel;
	if(!backgroundLangModel_->open())
	{
        std::cout<<"Error in opening the language model"<<std::endl;
		return false;
	}
	return true;
}

bool LogProcessor::updateBackGroundLangModel()
{
	return true;

}

bool LogProcessor::constructSliceLangModel(const std::list<std::pair<wiselib::UString,int> >& logItems)
{
	updateBackGroundLangModel();
	sliceLangModel_.clear();
	std::list<std::pair<wiselib::UString,int> >::const_iterator iter=logItems.begin();

	for(;iter!=logItems.end();iter++)
	{
		std::vector<wiselib::UString> termList;
		if(!getAnalyzedTerms(iter->first, termList))
			return false;
        for(uint32_t j=0;j<termList.size();j++)
        {
        	if(termList[j].isChineseChar(0))
        	{
        	    uint64_t key=HashFunction<wiselib::UString>::generateHash64(termList[j]);
        	    sliceLangModel_[key]+=iter->second;
//                uint32_t freq=0;
//        	    backgroundLangModel_->get(key, freq);
//        	    backgroundLangModel_->update(key, freq+iter->second);
        	}
        }
	}
	return true;

}

bool LogProcessor::rankQuery(const std::list<std::pair<wiselib::UString,int> >& logItems, std::vector<LogScoreItem>& scoreItemList)
{
	scoreItemList.resize(logItems.size());
	std::list<std::pair<wiselib::UString,int> >::const_iterator iter=logItems.begin();
	for(;iter!=logItems.end();iter++)
	{
		LogScoreItem scoreItem;
		rankQuery(iter->first, iter->second, scoreItem);
		scoreItemList.push_back(scoreItem);
	}
	std::sort(scoreItemList.begin(), scoreItemList.end());
//	for(uint32_t i=0;i<scoreItemList.size();i++)
//	{
//		scoreItemList[i].itemId_.displayStringValue(wiselib::UString::UTF_8);
//		std::cout<<" : "<<scoreItemList[i].score_<<std::endl;
//	}

	return true;

}

bool LogProcessor::rankQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem)
{
	std::vector<wiselib::UString> termList;
	if(!getAnalyzedTerms(query, termList))
		return false;
	std::vector<uint64_t> termIdList(termList.size());
    for(uint32_t j=0;j<termList.size();j++)
    {
    	uint64_t key=HashFunction<wiselib::UString>::generateHash64(termList[j]);
    	termIdList[j]=key;
    }
    float score;
    rankQuery(termIdList, score);
    float fSize=termIdList.size();
//    score=score*sqrt(freq)*log(1+1.0F/(std::abs(fSize-4))+1);
//    score=freq;
    score=score*sqrt(freq+0.01);
    scoreItem.itemId_=query;
    scoreItem.score_=score;

    return true;
}

bool LogProcessor::rankQuery(const std::vector<uint64_t>& termIdList, float& score)
{
	score=0;
	for(uint32_t i=0;i<termIdList.size();i++)
	{
		float termScore;
		rankTerm(termIdList[i], termScore);
		score+=termScore;
	}
	return true;
}

bool LogProcessor::rankTerm(uint64_t termId, float& score)
{
	const int mu=1000;
	uint32_t backFreq=0;
	float fFreq;
	backgroundLangModel_->get(termId, backFreq);
//	if (backFreq==0)
//	{
//		fFreq=0.0000001;
//	}
//	else
//	{
		fFreq=static_cast<float>(backFreq+5)/100000;
//	}

    score=log(1+static_cast<float>(sliceLangModel_[termId])/(1+mu*fFreq));
	return true;
}

bool LogProcessor::initializeLa()
{
	const int min=2;
	const int max=2;
	la::TokenizeConfig config;
	la_.setTokenizerConfig( config );
	boost::shared_ptr<la::Analyzer> analyzer;
	analyzer.reset( new la::NGramAnalyzer( min, max, 16 ) );
	static_cast<la::NGramAnalyzer*>(analyzer.get())->setApartFlag( la::NGramAnalyzer::NGRAM_APART_ALL_ );
	la_.setAnalyzer( analyzer );
	return true;

}

bool LogProcessor::getAnalyzedTerms(const wiselib::UString& logItem, std::vector<wiselib::UString>& termList)
{
	la::TermList laTermList;
	la_.process_search( logItem, laTermList);
    for (la::TermList::iterator p = laTermList.begin(); p != laTermList.end(); p++)
    {
        termList.push_back(p->text_);
    }
    return true;
}

}
