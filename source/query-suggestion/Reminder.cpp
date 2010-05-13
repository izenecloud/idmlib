/*
 * Reminder.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/Reminder.h>
#include <LA.h>
#include <vector>
#include <functional>
#include <algorithm>
#include <util/functional.h>
#include <queue>

namespace idmlib
{

Reminder::Reminder(const std::string& path, size_t realTimeNum, size_t popularNum):
	remindQueryIndex_(path+"/remind.index"),
    realTimeNum_(realTimeNum),
   popularNum_(popularNum)

{
	backgroundLangModel_.reset(new LANG_MODEL_TYPE(path+"/back_lang.model"));
	backgroundLangModel_->open();
	referLangModel_.reset(new REFER_LANG_MODEL_TYPE(path+"/refer_lang.model"));
	referLangModel_->open();
	latestSliceCount_.reset(new QUERY_COUNT_TYPE(path+"/latest_slice_count.model"));
	latestSliceCount_->open();
	periodCount_.reset(new QUERY_COUNT_TYPE(path+"/period_count.model"));
	periodCount_->open();
	remindQueryIndex_.open();
	initializeLa();
}

Reminder::~Reminder()
{
    backgroundLangModel_->flush();
    cleanData();
}

void Reminder::flush()
{
	backgroundLangModel_->flush();
	remindQueryIndex_.commit();
}

void Reminder::cleanData()
{
    referLangModel_->clear();
    latestSliceCount_->clear();
    periodCount_->clear();
    latestSliceLangModel_.clear();
    periodLangModel_.clear();
    referEntropyScore_.clear();
}

bool Reminder::initializeBackGroundLangModel(const boost::shared_ptr<LANG_MODEL_TYPE>& langModel)
{
	backgroundLangModel_=langModel;
	if(!backgroundLangModel_->open())
	{
        std::cout<<"Error in opening the language model"<<std::endl;
		return false;
	}
	return true;
}

bool Reminder::updateBackGroundLangModel(uint32_t key, int value)
{
    uint32_t freq=0;
    backgroundLangModel_->get(key, freq);
    backgroundLangModel_->update(key, freq+value);
	return true;
}

bool Reminder::updateReferLangModel(uint32_t timeId, uint32_t key, uint32_t value)
{
    TIME_SERIES_FREQ_TYPE vecFreq;
    referLangModel_->get(key, vecFreq);
    vecFreq.push_back(std::make_pair(timeId, value));
    referLangModel_->update(key, vecFreq);
	return true;
}

bool Reminder::indexQueryLog(uint32_t timeId, const std::list<std::pair<wiselib::UString,int> >& logItems, bool isLastestTimeId)
{
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
        	    uint32_t key=HashFunction<wiselib::UString>::generateHash64(termList[j]);
           	    if(isLastestTimeId)
        	    {
            	    if(latestSliceLangModel_.find(key)==latestSliceLangModel_.end())
            	    {
            	    	latestSliceLangModel_[key]=iter->second;
            	    }
            	    else
            	    {
            	    	latestSliceLangModel_[key]+=iter->second;
            	    }
        	    }
           	    else
           	    {
            	    if(periodLangModel_.find(key)==periodLangModel_.end())
            	    {
            	    	periodLangModel_[key]=iter->second;
            	    }
            	    else
            	    {
            	    	periodLangModel_[key]+=iter->second;
            	    }
//            	    updateBackGroundLangModel(key, iter->second);
            	    updateReferLangModel(timeId,key,iter->second);
           	    }
        	}
        }
        if(isLastestTimeId)
        	latestSliceCount_->update(iter->first, iter->second);
        uint32_t freq=0;
        periodCount_->get(iter->first, freq);
        periodCount_->update(iter->first, freq+iter->second);
	}
	return true;
}

bool Reminder::indexQuery()
{
	std::cout<<"start indexing reminding queries"<<std::endl;
    computeReferEntropyScore();
    indexPopularQuery();
    indexRealTimeQuery();
    flush();
    cleanData();
	return true;
}

bool Reminder::indexPopularQuery()
{
    typedef std::pair<wiselib::UString, float> value_type;
    typedef std::vector<value_type> sequence_type;
    typedef izenelib::util::second_greater<value_type> greater_than;
    std::priority_queue<value_type, sequence_type, greater_than> queue;

    QUERY_COUNT_CURSORTYPE locn = periodCount_->get_first_locn();
    wiselib::UString key;
    uint32_t val;
    while (periodCount_->get(locn, key, val) ) {
    	periodCount_->seq(locn);
    	float rankScore=0;
    	rankPopularQuery(key, val, rankScore);
    	value_type element(key, rankScore);
    	if (queue.size() < popularNum_)
    	{
            queue.push(element);
        }
        else if (element.second > queue.top().second)
        {
            queue.push(element);
            queue.pop();
        }
    }

    std::vector<wiselib::UString> result;
    result.reserve(queue.size());
    while (!queue.empty())
    {
    	result.push_back(queue.top().first);
        queue.pop();
    }
    std::vector<wiselib::UString> popularQueryResult;
    popularQueryResult. reserve(result.size());
    if(result.size()>0)
    {
        for(size_t i=result.size();i>0;i--)
        {
        	popularQueryResult.push_back(result[i-1]);
        }
    }
    remindQueryIndex_.update(static_cast<int>(POP), popularQueryResult);
	return true;

}

bool Reminder::indexRealTimeQuery()
{
    typedef std::pair<wiselib::UString, float> value_type;
    typedef std::vector<value_type> sequence_type;
    typedef izenelib::util::second_greater<value_type> greater_than;
    std::priority_queue<value_type, sequence_type, greater_than> queue;

    QUERY_COUNT_CURSORTYPE locn = latestSliceCount_->get_first_locn();
    wiselib::UString key;
    uint32_t val;
    while (latestSliceCount_->get(locn, key, val) ) {
    	latestSliceCount_->seq(locn);
    	float rankScore=0;
    	rankRealTimeQuery(key, val, rankScore);
    	value_type element(key, rankScore);
    	if (queue.size() < realTimeNum_)
    	{
            queue.push(element);
        }
        else if (element.second > queue.top().second)
        {
            queue.push(element);
            queue.pop();
        }
    }

    std::vector<wiselib::UString> result;
    result.reserve(queue.size());
    while (!queue.empty())
    {
    	result.push_back(queue.top().first);
        queue.pop();
    }
    std::vector<wiselib::UString> realTimeQueryResult;
    realTimeQueryResult. reserve(result.size());
    if(result.size()>0)
    {
        for(size_t i=result.size();i>0;i--)
        {
        	realTimeQueryResult.push_back(result[i-1]);
        }
    }
	remindQueryIndex_.update(static_cast<int>(REAL), realTimeQueryResult);
	return true;

}

bool Reminder::rankPopularQuery(const wiselib::UString& query, int freq, float& score)
{
	std::vector<wiselib::UString> termList;
	if(!getAnalyzedTerms(query, termList))
		return false;
	std::vector<uint32_t> termIdList(termList.size());
    for(uint32_t j=0;j<termList.size();j++)
    {
    	uint32_t key=HashFunction<wiselib::UString>::generateHash64(termList[j]);
    	termIdList[j]=key;
    }
    rankPopularQuery(termIdList, score);
    score=score*freq;

    return true;
}

bool Reminder::rankRealTimeQuery(const wiselib::UString& query, int freq, float& score)
{
	std::vector<wiselib::UString> termList;
	if(!getAnalyzedTerms(query, termList))
		return false;
	std::vector<uint32_t> termIdList(termList.size());
    for(uint32_t j=0;j<termList.size();j++)
    {
    	uint32_t key=HashFunction<wiselib::UString>::generateHash64(termList[j]);
    	termIdList[j]=key;
    }
    rankRealTimeQuery(termIdList, score);
    score=score*freq;
//    score=freq;
    float novelty=0;
    getNoveltyScore(termIdList, novelty);
    score=novelty*log(score+0.01);

    return true;
}

bool Reminder::rankPopularQuery(const std::vector<uint32_t>& termIdList, float& score)
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

bool Reminder::rankRealTimeQuery(const std::vector<uint32_t>& termIdList, float& score)
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

bool Reminder::rankPopularTerm(uint32_t termId, float& score)
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

bool Reminder::rankRealTimeTerm(uint32_t termId, float& score)
{
	const int mu=1000;
	uint32_t backFreq=0;
//	if(!backgroundLangModel_->get(termId, backFreq))
//	{
//		score=0;
//		return true;
//	}
	float fFreq=0;
	fFreq=static_cast<float>(backFreq+5)/100000;

	//This should be the overall number of terms of the period query log.
	//Just use some reasonable number to simulate it in experiment.

	uint32_t sliceScore=0;
	if(latestSliceLangModel_.find(termId)==latestSliceLangModel_.end())
		sliceScore=0;
	else
		sliceScore=latestSliceLangModel_[termId];
    score=log(1+static_cast<float>(sliceScore)/(1+mu*fFreq));
	return true;
}

bool Reminder::initializeLa()
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

bool Reminder::getAnalyzedTerms(const wiselib::UString& logItem, std::vector<wiselib::UString>& termList)
{
	la::TermList laTermList;
	la_.process_search( logItem, laTermList);
    for (la::TermList::iterator p = laTermList.begin(); p != laTermList.end(); p++)
    {
        termList.push_back(p->text_);
    }
    return true;
}

bool Reminder::computeReferEntropyScore()
{
	std::cout<<"start to compute entropy score"<<std::endl;
	REFER_LANG_MODEL_CURSOR_TYPE locn = referLangModel_->get_first_locn();
    uint32_t key;
    TIME_SERIES_FREQ_TYPE vecFreq;
    while (referLangModel_->get(locn, key, vecFreq) ) {
    	referLangModel_->seq(locn);
    	float entropy=0;
    	computeReferEntropyScore(key, vecFreq, entropy);
    	referEntropyScore_.insert(std::make_pair(key, entropy));
    }
    return true;
}

bool Reminder::computeReferEntropyScore(uint32_t key, const TIME_SERIES_FREQ_TYPE& vecFreq, float& score)
{
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

bool Reminder::getReferEntropyScore(uint32_t key, float& score)
{
    if(referEntropyScore_.find(key)==referEntropyScore_.end())
    {
    	score=0;
    	return false;
    }
    else
    {
    	score=referEntropyScore_[key];
    }
    return true;
}

bool Reminder::getNoveltyScore(const std::vector<uint32_t>& termIdList, float& score)
{
	float klScore=0;
//	float maxScore=klScore;
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
		if(latestSliceLangModel_.find(termIdList[i])!=latestSliceLangModel_.end())
			sliceScore=latestSliceLangModel_[termIdList[i]];
        klScore=(sliceScore+0.01)/(periodScore+0.01);
//		if(klScore>maxScore)
//		{
//			maxScore=klScore;
//		}
		avgScore+=klScore;
//		scoreList.push_back(klScore);
	}
	if(termIdList.size()!=0)
		avgScore/=termIdList.size();
	else
		avgScore=0;

//	score=maxScore;
	score=avgScore;
	return true;
}

bool Reminder::getRealTimeQuery(std::vector<wiselib::UString>& realTimeQueries)
{
	return remindQueryIndex_.get(static_cast<int>(REAL), realTimeQueries);

}

bool Reminder::getPopularQuery(std::vector<wiselib::UString>& popularQueries)
{
	return remindQueryIndex_.get(static_cast<int>(POP), popularQueries);

}

}
