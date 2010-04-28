/*
 * Recommender.h
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#ifndef RECOMMENDER_H_
#define RECOMMENDER_H_

#include "LogProcessor.h"
#include <boost/shared_ptr.hpp>
#include <wiselib/ustring/UString.h>


#include <vector>
#include <list>
#include <string>

namespace idmlib
{

/**
 *@brief Recommender is used to give query irrevalent suggestions according to Query Log.
 */

class Recommender
{
public:
	typedef ScoreItem<wiselib::UString> LogScoreItem;
public:
	/**
	 * @brief The constructor.
	 * @param realTimeNum  The number of real-Time queries need to be returned.
	 * @param popularNum   The number of popular queries need to be returned.
	 */
	Recommender(
			const string& path,
			int realTimeNum=DEFAULT_REAL_TIME_QUERY_NUM,
			int popularNum=DEFAULT_POPULAR_QUERY_NUM);
	virtual ~Recommender();

	/**
	 *
	 */
	bool indexLog(const std::list<std::pair<wiselib::UString,int> >& logItems);

	/**
	 *
	 */
	bool getRealTimeQuery(std::vector<wiselib::UString>& realTimeQueries);

	/**
	 *
	 */
	bool getPopularQuery(std::vector<wiselib::UString>& popularQueries);

	void flush();


private:
//	Recommender();
	bool computeRealTimeQuery(std::vector<wiselib::UString>& realTimeQueries);
	bool computePopularQuery(std::vector<wiselib::UString>& popularQueries);

private:
//	boost::shared_ptr<LogProcessor> logProcessor_;
	LogProcessor* logProcessor_;
	int realTimeNum_;
	int popularNum_;
	std::vector<wiselib::UString> realQueries_;

private:

	static const int DEFAULT_REAL_TIME_QUERY_NUM=10;
	static const int DEFAULT_POPULAR_QUERY_NUM=50;


};

}

#endif /* RECOMMENDER_H_ */
