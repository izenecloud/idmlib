/*
 * LogProcessor.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/LogProcessor.h>
#include <LA.h>
#include <vector>
#include <functional>
#include <algorithm>

namespace idmlib
{

LogProcessor::LogProcessor(const std::string& path)
{
	backgroundLangModel_.reset(new LANG_MODEL_TYPE(path+"/back_lang.model"));
	backgroundLangModel_->open();
	referLangModel_.reset(new REFER_LANG_MODEL_TYPE(path+"/refer_lang.model"));
	referLangModel_->open();
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

bool LogProcessor::updateReferLangModel(uint32_t timeId, uint64_t key, uint32_t freq)
{
	return true;
}

bool LogProcessor::constructSliceLangModel(uint32_t timeId, const std::list<std::pair<wiselib::UString,int> >& logItems)
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
        	    if(sliceLangModel_.find(key)==sliceLangModel_.end())
        	    {
        	    	sliceLangModel_[key]=iter->second;
        	    }
        	    else
        	    {
        	    	sliceLangModel_[key]+=iter->second;
        	    }
        	    if(periodLangModel_.find(key)==periodLangModel_.end())
        	    {
        	    	periodLangModel_[key]=iter->second;
        	    }
        	    else
        	    {
        	    	periodLangModel_[key]+=iter->second;
        	    }

//        	    //temporarily construct the backgroundModel;
//                uint32_t freq=0;
//        	    backgroundLangModel_->get(key, freq);
//        	    backgroundLangModel_->update(key, freq+iter->second);
        	    //temporarily construct the reference model for experiment.
        	    TIME_SERIES_FREQ_TYPE vecFreq;
        	    referLangModel_->get(key, vecFreq);
        	    vecFreq.push_back(std::make_pair(timeId, iter->second));
        	    referLangModel_->update(key, vecFreq);
        	}
        }
	}
	return true;

}

bool LogProcessor::rankPopularQuery(const std::list<std::pair<wiselib::UString,int> >& logItems, std::vector<LogScoreItem>& scoreItemList)
{
	scoreItemList.resize(logItems.size());
	std::list<std::pair<wiselib::UString,int> >::const_iterator iter=logItems.begin();
	for(;iter!=logItems.end();iter++)
	{
		LogScoreItem scoreItem;
		rankPopularQuery(iter->first, iter->second, scoreItem);
		scoreItemList.push_back(scoreItem);
	}
	std::sort(scoreItemList.begin(), scoreItemList.end());

	return true;

}

bool LogProcessor::rankRealTimeQuery(const std::list<std::pair<wiselib::UString,int> >& logItems, std::vector<LogScoreItem>& scoreItemList)
{
	scoreItemList.resize(logItems.size());
	std::list<std::pair<wiselib::UString,int> >::const_iterator iter=logItems.begin();
	for(;iter!=logItems.end();iter++)
	{
		LogScoreItem scoreItem;
		rankRealTimeQuery(iter->first, iter->second, scoreItem);
		scoreItemList.push_back(scoreItem);
	}
	std::sort(scoreItemList.begin(), scoreItemList.end());

	return true;

}

bool LogProcessor::rankPopularQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem)
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
    rankPopularQuery(termIdList, score);
    score=score*freq;
    scoreItem.itemId_=query;
    scoreItem.score_=score;

    return true;
}

bool LogProcessor::rankRealTimeQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem)
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
    float score=0;
    rankRealTimeQuery(termIdList, score);
    score=score*freq;
    float novelty=0;
    getNoveltyScore(termIdList, novelty);
//    std::cout<<"novelty score:"<<novelty<<std::endl;
    score=novelty*log(score+0.01);
    scoreItem.itemId_=query;
    scoreItem.score_=score;

    return true;
}

bool LogProcessor::rankPopularQuery(const std::vector<uint64_t>& termIdList, float& score)
{
	score=0;
	for(uint32_t i=0;i<termIdList.size();i++)
	{
		float termScore;
		rankPopularTerm(termIdList[i], termScore);
		score+=termScore;
	}
	return true;
}

bool LogProcessor::rankRealTimeQuery(const std::vector<uint64_t>& termIdList, float& score)
{
	score=0;
	for(uint32_t i=0;i<termIdList.size();i++)
	{
		float termScore;
		rankRealTimeTerm(termIdList[i], termScore);
		score+=termScore;
	}
	return true;
}

bool LogProcessor::rankPopularTerm(uint64_t termId, float& score)
{
	const int mu=1000;
	uint32_t backFreq=0;
	float fFreq;
	backgroundLangModel_->get(termId, backFreq);
	//This should be the overall number of terms of the period query log.
	// Just use some reasonable number to simulate it in experiment.
	fFreq=static_cast<float>(backFreq+5)/100000;
	float weight=0;
	getReferEntropyScore(termId, weight);
	uint32_t periodScore=0;
	if(periodLangModel_.find(termId)==periodLangModel_.end())
		periodScore=0;
	else
		periodScore=periodLangModel_[termId];
    score=log(1+static_cast<float>(periodScore*weight)/(1+mu*fFreq));
	return true;
}

bool LogProcessor::rankRealTimeTerm(uint64_t termId, float& score)
{
	const int mu=1000;
	uint32_t backFreq=0;
	float fFreq;
	backgroundLangModel_->get(termId, backFreq);
	//This should be the overall number of terms of the period query log.
	//Just use some reasonable number to simulate it in experiment.
	fFreq=static_cast<float>(backFreq+5)/100000;
	uint32_t sliceScore=0;
	if(sliceLangModel_.find(termId)==sliceLangModel_.end())
		sliceScore=0;
	else
		sliceScore=sliceLangModel_[termId];
    score=log(1+static_cast<float>(sliceScore)/(1+mu*fFreq));
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

bool LogProcessor::getReferEntropyScore(uint64_t key, float& score)
{
    TIME_SERIES_FREQ_TYPE vecFreq;
    referLangModel_->get(key, vecFreq);
    int totalCount=0;
    for(uint32_t i=0;i<vecFreq.size();i++)
    {
    	totalCount+=vecFreq[i].second;
    }
    if(totalCount==0)
    {
    	score=0.001;
    	return false;
    }
    for(uint32_t i=0;i<vecFreq.size();i++)
    {
    	if(vecFreq[i].second!=0)
    	{
    	    float prob=vecFreq[i].second/(float)totalCount;
    	    score+=prob*log(prob);
    	}
    }
    score=-score;
	return true;
}

bool LogProcessor::getNoveltyScore(const std::vector<uint64_t>& termIdList, float& score)
{
	float klScore=0;
	float maxScore=klScore;
	float avgScore=klScore;
//	std::vector<float> scoreList;
	for(size_t i=0;i<termIdList.size();i++)
	{
		uint32_t periodScore=0;
		if(periodLangModel_.find(termIdList[i])==periodLangModel_.end())
			periodScore=0;
		else
			periodScore=periodLangModel_[termIdList[i]];
        if(periodScore<1)
        	periodScore=0;

		uint32_t sliceScore=0;
		if(sliceLangModel_.find(termIdList[i])!=sliceLangModel_.end())
			sliceScore=sliceLangModel_[termIdList[i]];
        klScore=(sliceScore+0.01)/(periodScore+0.01);
		if(klScore>maxScore)
		{
			maxScore=klScore;
		}
		avgScore+=klScore;
//		scoreList.push_back(klScore);
	}
	if(termIdList.size()!=0)
		avgScore/=termIdList.size();
	else
		avgScore=0;
//	std::sort(scoreList.begin(), scoreList.end(), std::greater<float>());
//	score=0;
//	int count=1;
//	for(size_t i=0;i<scoreList.size()&&i<count;i++)
//	{
//		score+=scoreList[i];
//	}
//	score/=count;
	score=maxScore;
//	score=avgScore;
	return true;
}

}
