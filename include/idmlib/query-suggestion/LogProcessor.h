/*
 * LogProcessor.h
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
#include <idmlib/query-suggestion/ScoreItem.h>
#include <util/izene_serialization.h>
#include <LA.h>
#include <vector>
#include <list>


namespace idmlib
{

typedef std::vector<std::pair<uint32_t, uint32_t> > TIME_SERIES_FREQ_TYPE;

class LogProcessor
{
public:
	typedef izenelib::am::sdb_fixedhash<uint64_t, uint32_t, izenelib::util::ReadWriteLock> LANG_MODEL_TYPE;
	typedef izenelib::am::sdb_btree<uint64_t, TIME_SERIES_FREQ_TYPE, izenelib::util::ReadWriteLock> REFER_LANG_MODEL_TYPE;
	typedef ScoreItem<wiselib::UString> LogScoreItem;
public:
	LogProcessor(const std::string& path);
	virtual ~LogProcessor();
	bool initializeBackGroundLangModel(const boost::shared_ptr<LANG_MODEL_TYPE>& langModel);
	bool updateBackGroundLangModel();
	bool updateReferLangModel(uint32_t timeId, uint64_t key, uint32_t freq);
	bool constructSliceLangModel(uint32_t timeId,
			const std::list<std::pair<wiselib::UString,int> >& logItems);
	bool rankPopularQuery(const std::list<std::pair<wiselib::UString,int> >& logItems, std::vector<LogScoreItem>& scoreItemList);
	bool rankRealTimeQuery(const std::list<std::pair<wiselib::UString,int> >& logItems, std::vector<LogScoreItem>& scoreItemList);
	bool warmup();


private:
	bool rankPopularQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem);
	bool rankPopularQuery(const std::vector<uint64_t>& termIdList, float& score);
	bool rankPopularTerm(uint64_t termId, float& score);
	bool rankRealTimeQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem);
	bool rankRealTimeQuery(const std::vector<uint64_t>& termIdList, float& score);
	bool rankRealTimeTerm(uint64_t termId, float& score);
	bool getAnalyzedTerms(const wiselib::UString& logItem, std::vector<wiselib::UString>& termList);
	bool getReferEntropyScore(uint64_t key, float& score);
	bool getNoveltyScore(const std::vector<uint64_t>& termIdList, float& score);
	bool initializeLa();

private:

	la::LA la_;

    boost::unordered_map<uint64_t, uint32_t> sliceLangModel_;
    boost::unordered_map<uint64_t, uint32_t> periodLangModel_;
	boost::shared_ptr<LANG_MODEL_TYPE> backgroundLangModel_;
	boost::shared_ptr<REFER_LANG_MODEL_TYPE> referLangModel_;

};

}


MAKE_FEBIRD_SERIALIZATION(idmlib::TIME_SERIES_FREQ_TYPE)




#endif /* LOGPROCESSOR_H_ */
