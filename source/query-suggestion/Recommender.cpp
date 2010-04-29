/*
 * Recommender.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/Recommender.h>
#include <idmlib/query-suggestion/ScoreItem.h>


namespace idmlib
{

Recommender::Recommender(
		const string& path,
		int realTimeNum,
		int popularNum):
    realTimeNum_(realTimeNum),
    popularNum_(popularNum)
{
	logProcessor_=new LogProcessor(path);
}

Recommender::~Recommender()
{
	if(logProcessor_)
		delete logProcessor_;
}

bool Recommender::indexLog(const std::list<std::pair<wiselib::UString,int> >& logItems)
{
	if(!logProcessor_->constructSliceLangModel(logItems))
		return false;
	std::vector<LogScoreItem> scoreItems;
	if(!logProcessor_->rankQuery(logItems,scoreItems))
		return false;
	int indexedRealNum=realTimeNum_<scoreItems.size() ? realTimeNum_:scoreItems.size();
	realQueries_.clear();
	for(uint32_t i=0;i<indexedRealNum;i++)
	{
		realQueries_.push_back(scoreItems[i].itemId_);
	}
	flush();
	return true;

}

bool Recommender::getRealTimeQuery(std::vector<wiselib::UString>& realTimeQueries)
{
    realTimeQueries=realQueries_;
    return true;
}

bool Recommender::getPopularQuery(std::vector<wiselib::UString>& popularQueries)
{
	return true;
}

void Recommender::flush()
{

}

}
