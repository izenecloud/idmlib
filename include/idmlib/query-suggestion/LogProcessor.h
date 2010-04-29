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
#include <LA.h>

namespace idmlib
{

class LogProcessor
{
public:
	typedef izenelib::am::sdb_fixedhash<uint64_t, uint32_t, izenelib::util::ReadWriteLock> LANG_MODEL_TYPE;
	typedef ScoreItem<wiselib::UString> LogScoreItem;
public:
	LogProcessor(const std::string& path);
	virtual ~LogProcessor();
	bool initializeBackGroundLangModel(const boost::shared_ptr<LANG_MODEL_TYPE>& langModel);
	bool updateBackGroundLangModel();
	bool constructSliceLangModel(const std::list<std::pair<wiselib::UString,int> >& logItems);
	bool rankQuery(
			const std::list<std::pair<wiselib::UString,int> >& logItems,
			std::vector<LogScoreItem>& scoreItemList);
	bool warmup();


private:
	bool rankQuery(const wiselib::UString& query, int freq, LogScoreItem& scoreItem);
	bool rankQuery(const std::vector<uint64_t>& termIdList, float& score);
	bool rankTerm(uint64_t termId, float& score);
	bool getAnalyzedTerms(const wiselib::UString& logItem, std::vector<wiselib::UString>& termList);
	bool initializeLa();

private:

	la::LA la_;

    boost::unordered_map<uint64_t, uint32_t> sliceLangModel_;
	boost::shared_ptr<LANG_MODEL_TYPE> backgroundLangModel_;



};

}

#endif /* LOGPROCESSOR_H_ */
