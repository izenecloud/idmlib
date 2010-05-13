/*
 * Reminder.h
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#ifndef LOGPROCESSOR_H_
#define LOGPROCESSOR_H_

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <sdb/SequentialDB.h>
#include <wiselib/ustring/UString.h>
#include <util/izene_serialization.h>
#include <LA.h>
#include <vector>
#include <list>


namespace idmlib
{

typedef std::vector<std::pair<uint32_t, uint32_t> > TIME_SERIES_FREQ_TYPE;

class Reminder
{
public:
	typedef izenelib::am::sdb_fixedhash<uint32_t, uint32_t, izenelib::util::ReadWriteLock> LANG_MODEL_TYPE;
	typedef izenelib::am::sdb_btree<uint32_t, TIME_SERIES_FREQ_TYPE, izenelib::util::ReadWriteLock> REFER_LANG_MODEL_TYPE;
	typedef izenelib::am::sdb_btree<uint32_t, TIME_SERIES_FREQ_TYPE, izenelib::util::ReadWriteLock>::SDBCursor REFER_LANG_MODEL_CURSOR_TYPE;
	typedef izenelib::am::sdb_btree<wiselib::UString, uint32_t> QUERY_COUNT_TYPE;
	typedef izenelib::am::sdb_btree<wiselib::UString, uint32_t>::SDBCursor QUERY_COUNT_CURSORTYPE;
	typedef enum{POP=0, REAL=1} REMIND_TYPE;
	typedef izenelib::am::sdb_btree<int, std::vector<wiselib::UString>, izenelib::util::ReadWriteLock> REMIND_INDEX_TYPE;

public:
	Reminder(const std::string& path,
			size_t realTimeNum=DEFAULT_REAL_TIME_QUERY_NUM,
			size_t popularNum=DEFAULT_POPULAR_QUERY_NUM);
	virtual ~Reminder();

	bool initializeBackGroundLangModel(const boost::shared_ptr<LANG_MODEL_TYPE>& langModel);

	/**
	 *
	 */
	bool indexQueryLog(uint32_t timeId,
			const std::list<std::pair<wiselib::UString,int> >& logItems,
			bool isLastestTimeId=false);
	/**
	 *
	 */
	bool indexQuery();

	/**
	 *
	 */
	bool getRealTimeQuery(std::vector<wiselib::UString>& realTimeQueries);

	/**
	 *
	 */
	bool getPopularQuery(std::vector<wiselib::UString>& popularQueries);

	/**
	 *
	 */
	void flush();


private:
	Reminder(){}
	bool indexPopularQuery();
	bool indexRealTimeQuery();
	bool updateBackGroundLangModel(uint32_t key, int value);
	bool updateReferLangModel(uint32_t timeId, uint32_t key, uint32_t value);
	bool rankPopularQuery(const wiselib::UString& query, int freq, float& score);
	bool rankPopularQuery(const std::vector<uint32_t>& termIdList, float& score);
	bool rankPopularTerm(uint32_t termId, float& score);
	bool rankRealTimeQuery(const wiselib::UString& query, int freq, float& score);
	bool rankRealTimeQuery(const std::vector<uint32_t>& termIdList, float& score);
	bool rankRealTimeTerm(uint32_t termId, float& score);
	bool getAnalyzedTerms(const wiselib::UString& logItem, std::vector<wiselib::UString>& termList);
	bool getReferEntropyScore(uint32_t key, float& score);
	bool getNoveltyScore(const std::vector<uint32_t>& termIdList, float& score);
	bool computeReferEntropyScore();
	bool computeReferEntropyScore(uint32_t key, const TIME_SERIES_FREQ_TYPE& vecFreq, float& score);
	bool initializeLa();
	void cleanData();

private:

	la::LA la_;
    boost::unordered_map<uint32_t, uint32_t> latestSliceLangModel_;
    boost::unordered_map<uint32_t, uint32_t> periodLangModel_;
    boost::unordered_map<uint32_t, float> referEntropyScore_;
	boost::shared_ptr<LANG_MODEL_TYPE> backgroundLangModel_;
	boost::shared_ptr<REFER_LANG_MODEL_TYPE> referLangModel_;
	boost::shared_ptr<QUERY_COUNT_TYPE> latestSliceCount_;
	boost::shared_ptr<QUERY_COUNT_TYPE> periodCount_;
	REMIND_INDEX_TYPE remindQueryIndex_;
	size_t realTimeNum_;
	size_t popularNum_;
private:
	static const size_t DEFAULT_REAL_TIME_QUERY_NUM=50;
	static const size_t DEFAULT_POPULAR_QUERY_NUM=50;

};

}


MAKE_FEBIRD_SERIALIZATION(idmlib::TIME_SERIES_FREQ_TYPE)




#endif /* LOGPROCESSOR_H_ */
