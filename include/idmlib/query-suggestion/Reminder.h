/*
 * Reminder.h
 *
 *  Created on: 2010-05-14
 *      Author: jinglei
 */

#ifndef LOGPROCESSOR_H_
#define LOGPROCESSOR_H_

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <sdb/SequentialDB.h>
#include <util/ustring/UString.h>
#include <util/izene_serialization.h>
#include <util/functional.h>

#include <algorithm>
#include <queue>
#include <vector>
#include <list>

namespace idmlib
{

typedef std::vector<std::pair<uint32_t, uint32_t> > TIME_SERIES_FREQ_TYPE;

/**
 * @brief Reminder functions as a query reminding mechanism \n
 *  to provide the popular query and real time query.
 */
template<typename IDManagerType>
class Reminder
{
	typedef izenelib::am::sdb_fixedhash<uint32_t, uint32_t,
			izenelib::util::ReadWriteLock> LANG_MODEL_TYPE;
	typedef izenelib::am::sdb_btree<uint32_t, TIME_SERIES_FREQ_TYPE,
			izenelib::util::ReadWriteLock> REFER_LANG_MODEL_TYPE;
	typedef izenelib::am::sdb_btree<uint32_t, TIME_SERIES_FREQ_TYPE,
			izenelib::util::ReadWriteLock>::SDBCursor
			REFER_LANG_MODEL_CURSOR_TYPE;
	typedef izenelib::am::sdb_btree<izenelib::util::UString, uint32_t>
			QUERY_COUNT_TYPE;
	typedef izenelib::am::sdb_btree<izenelib::util::UString, uint32_t>::SDBCursor
			QUERY_COUNT_CURSORTYPE;
	typedef enum
	{
		POP = 0, REAL = 1
	} REMIND_TYPE;
	typedef izenelib::am::sdb_btree<int, std::vector<izenelib::util::UString>,
			izenelib::util::ReadWriteLock> REMIND_INDEX_TYPE;
	typedef std::pair<izenelib::util::UString, float> VALUE_TYPE;
	typedef std::vector<VALUE_TYPE> SEQUENCE_TYPE;
	typedef izenelib::util::second_greater<VALUE_TYPE> greater_than;
	typedef std::priority_queue<VALUE_TYPE, SEQUENCE_TYPE, greater_than> QUEUE_TYPE;

public:

	/**
	 * @brief The constructor
	 * @param path The root path to store the files of query reminding.
	 * @param realTimeNum The number of reatime queries reminded in the result.
	 * @param popularNum The number of popular queries reminded in the result.
	 */
	Reminder(IDManagerType* idManager, const std::string& path,
			size_t realTimeNum = DEFAULT_REAL_TIME_QUERY_NUM,
			size_t popularNum = DEFAULT_POPULAR_QUERY_NUM) :
		idManager_(idManager), remindQueryIndex_(path + "/remind.index"),
				realTimeNum_(realTimeNum), popularNum_(popularNum)
	{
		//		backgroundLangModel_.reset(new LANG_MODEL_TYPE(path+"/back_lang.model"));
		//		backgroundLangModel_->open();
		referLangModel_.reset(new REFER_LANG_MODEL_TYPE(path
				+ "/refer_lang.model"));
		referLangModel_->open();
		latestSliceCount_.reset(new QUERY_COUNT_TYPE(path
				+ "/latest_slice_count.model"));
		latestSliceCount_->open();
		periodCount_.reset(new QUERY_COUNT_TYPE(path + "/period_count.model"));
		periodCount_->open();
		remindQueryIndex_.open();
		totalSliceTermCount_ = 0;
		totalPeriodTermCount_ = 0;
		//		initializeLa();
	}

	/**
	 * @brief The destructor.
	 */
	virtual ~Reminder()
	{
		//	    backgroundLangModel_->flush();
		cleanData();
	}

	/*
	 * @brief Store the data to disk, mainly for the indexed reminded query.
	 */
	void flush()
	{
		//		backgroundLangModel_->flush();
		remindQueryIndex_.commit();
	}

	/**
	 * @brief Initialize the background language model according to a trained one.
	 */
	bool initializeBackGroundLangModel(
			const boost::shared_ptr<LANG_MODEL_TYPE>& langModel)
	{
		//		backgroundLangModel_=langModel;
		//		if(!backgroundLangModel_->open())
		//		{
		//	        std::cout<<"Error in opening the language model"<<std::endl;
		//			return false;
		//		}
		return true;
	}

	/**
	 * @brief Index the query log for a specific time slice.\n
	 * The period language model and refer model will be updated.\n
	 * If the log slice is the last time slice of the query log. \n
	 * If The slice language model will be updated.
	 * @param timeId the identifier for the indexed time slice.
	 * @param logItems the <string, frequency> pair of the set of queries in this time slice.
	 * @param isLatestTimeId whether the current slice log updated is the latest one.
	 */
	bool indexQueryLog(uint32_t timeId, const std::list<std::pair<
			izenelib::util::UString, uint32_t> >& logItems, bool isLastestTimeId =
			false)
	{
		std::list<std::pair<izenelib::util::UString, uint32_t> >::const_iterator iter =
				logItems.begin();
		for (; iter != logItems.end(); iter++)
		{
			std::vector<izenelib::util::UString> strList;
			std::vector<uint32_t> termList;
			idManager_->getAnalysisTermList(iter->first, strList, termList);

			for (uint32_t j = 0; j < termList.size(); j++)
			{
				izenelib::util::UString& ustrTerm = strList[j];
				if (ustrTerm.isChineseChar(0))
				{
					uint32_t key = termList[j];
					if (isLastestTimeId)
					{
						if (latestSliceLangModel_.find(key)
								== latestSliceLangModel_.end())
						{
							latestSliceLangModel_[key] = iter->second;
							totalSliceTermCount_++;
						}
						else
						{
							latestSliceLangModel_[key] += iter->second;
						}
					}
					else
					{
						if (periodLangModel_.find(key)
								== periodLangModel_.end())
						{
							periodLangModel_[key] = iter->second;
							totalPeriodTermCount_++;
						}
						else
						{
							periodLangModel_[key] += iter->second;
						}
						//            	    updateBackGroundLangModel(key, iter->second);
						updateReferLangModel(timeId, key, iter->second);
					}
				}
			}
			if (isLastestTimeId)
				latestSliceCount_->update(iter->first, iter->second);
			else
			{
				uint32_t freq = 0;
				periodCount_->get(iter->first, freq);
				periodCount_->update(iter->first, freq + iter->second);
			}
		}
		return true;
	}

	/**
	 * @brief Compute the popular query and real time queries and update the index for query.
	 */
	bool indexQuery()
	{
		std::cout << "start indexing reminding queries" << std::endl;
		std::cout << "totalSliceTermCount_: " << totalSliceTermCount_
				<< std::endl;
		std::cout << "totalPeriodTermCount_: " << totalPeriodTermCount_
				<< std::endl;
		computeReferEntropyScore();
		indexPopularQuery();
		indexRealTimeQuery();
		flush();
		cleanData();
		return true;
	}

	/**
	 * @brief Get the result of realTimeQueries.
	 */
	bool getRealTimeQuery(std::vector<izenelib::util::UString>& realTimeQueries)
	{
		return remindQueryIndex_.get(static_cast<int> (REAL), realTimeQueries);
	}

	/**
	 * @brief Get the result of popularQueries.
	 */
	bool getPopularQuery(std::vector<izenelib::util::UString>& popularQueries)
	{
		return remindQueryIndex_.get(static_cast<int> (POP), popularQueries);
	}

private:

	/**
	 *@brief Clean the temporary data used, mainly for various language models constructed.
	 */
	void cleanData()
	{
		referLangModel_->clear();
		latestSliceCount_->clear();
		periodCount_->clear();
		latestSliceLangModel_.clear();
		periodLangModel_.clear();
		referEntropyScore_.clear();
		totalSliceTermCount_ = 0;
		totalPeriodTermCount_ = 0;
		referNormalizeScore_ = 0;
	}

	/**
	 * @brief Update the statistics of the background language model.
	 */
	bool updateBackGroundLangModel(uint32_t key, uint32_t value)
	{
		//	    uint32_t freq=0;
		//	    backgroundLangModel_->get(key, freq);
		//	    backgroundLangModel_->update(key, freq+value);
		return true;
	}

	/**
	 * @brief Update the reference model which is used as a prior weight for computing popularity.
	 * @param timeId the identifier for the time slices.
	 * @param key  the identifier for a specific word.
	 * @param value the value associated with the word, a frequency here.
	 */
	bool updateReferLangModel(uint32_t timeId, uint32_t key, uint32_t value)
	{
		TIME_SERIES_FREQ_TYPE vecFreq;
		referLangModel_->get(key, vecFreq);
		vecFreq.push_back(std::make_pair(timeId, value));
		referLangModel_->update(key, vecFreq);
		return true;
	}

	/**
	 * @brief Index the popular queries.
	 */
	bool indexPopularQuery()
	{
		const size_t candidateNum=popularNum_*5;
		QUEUE_TYPE queue;
		QUERY_COUNT_CURSORTYPE locn = periodCount_->get_first_locn();
		izenelib::util::UString key;
		uint32_t val;
		while (periodCount_->get(locn, key, val))
		{
			periodCount_->seq(locn);
			float rankScore = 0;
			rankPopularQuery(key, val, rankScore);
			VALUE_TYPE element(key, rankScore);
			if (queue.size() <candidateNum )
			{
				queue.push(element);
			}
			else if (element.second > queue.top().second)
			{
				queue.push(element);
				queue.pop();
			}
		}

		std::vector<izenelib::util::UString> result;
		result.reserve(queue.size());
		while (!queue.empty())
		{
			result.push_back(queue.top().first);
			queue.pop();
		}
		std::vector<izenelib::util::UString> popularQueryResult;
		popularQueryResult. reserve(result.size());
		if (result.size() > 0)
		{
			for (size_t i = result.size(); i > 0; i--)
			{
				popularQueryResult.push_back(result[i - 1]);
			}
		}
		deduplicate(popularQueryResult, popularNum_);
		remindQueryIndex_.update(static_cast<int> (POP), popularQueryResult);
		return true;

	}

	/**
	 * @brief Index the real time queries
	 */
	bool indexRealTimeQuery()
	{
		const size_t candidateNum=realTimeNum_*10;
		QUEUE_TYPE queue;
		QUERY_COUNT_CURSORTYPE locn = latestSliceCount_->get_first_locn();
		izenelib::util::UString key;
		uint32_t val;
		while (latestSliceCount_->get(locn, key, val))
		{
			latestSliceCount_->seq(locn);
			float rankScore = 0;
			rankRealTimeQuery(key, val, rankScore);
			VALUE_TYPE element(key, rankScore);
			if (queue.size() < candidateNum)
			{
				queue.push(element);
			}
			else if (element.second > queue.top().second)
			{
				queue.push(element);
				queue.pop();
			}
		}

		std::vector<izenelib::util::UString> result;
		result.reserve(queue.size());
		while (!queue.empty())
		{
			result.push_back(queue.top().first);
			queue.pop();
		}
		std::vector<izenelib::util::UString> realTimeQueryResult;
		realTimeQueryResult. reserve(result.size());
		if (result.size() > 0)
		{
			for (size_t i = result.size(); i > 0; i--)
			{
				realTimeQueryResult.push_back(result[i - 1]);
			}
		}
		deduplicate(realTimeQueryResult, realTimeNum_);
		remindQueryIndex_.update(static_cast<int> (REAL), realTimeQueryResult);
		return true;

	}

//	bool deduplicate(QUEUE_TYPE)

	bool rankPopularQuery(const izenelib::util::UString& query, uint32_t freq,
			float& score)
	{
		std::vector<uint32_t> termIdList;
		idManager_->getAnalysisTermIdList(query, termIdList);
		rankPopularQuery(termIdList, score);
//		score = score * freq;
		score=score*sqrt(freq+0.01);

		return true;
	}

	bool rankRealTimeQuery(const izenelib::util::UString& query, uint32_t freq,
			float& score)
	{
		std::vector<uint32_t> termIdList;
		idManager_->getAnalysisTermIdList(query, termIdList);
		rankRealTimeQuery(termIdList, score);
		score *= freq;
		float novelty = 0;
		getNoveltyScore(termIdList, novelty);
//		score *= novelty;
		score=novelty*sqrt(score+0.01);
		return true;
	}

	bool rankPopularQuery(const std::vector<uint32_t>& termIdList, float& score)
	{
		score = 0;
		for (uint32_t i = 0; i < termIdList.size(); i++)
		{
			float termScore;
			rankPopularTerm(termIdList[i], termIdList.size(), termScore);
			score += termScore;
		}
		return true;
	}

	bool rankRealTimeQuery(const std::vector<uint32_t>& termIdList,
			float& score)
	{
		score = 0;
		for (uint32_t i = 0; i < termIdList.size(); i++)
		{
			float termScore;
			rankRealTimeTerm(termIdList[i], termScore);
			score += termScore;
		}
		return true;
	}

	bool rankPopularTerm(uint32_t termId, size_t queryLength, float& score)
	{
		float weight = 0;
		getReferEntropyScore(termId, weight);
		uint32_t periodScore = 0;
		if (periodLangModel_.find(termId) == periodLangModel_.end())
			periodScore = 0;
		else
			periodScore = periodLangModel_[termId];
		const size_t alpha = 200;
		score = log(static_cast<float> ((periodScore + alpha * weight))
				/ (totalPeriodTermCount_ + alpha * referNormalizeScore_ + 1)
				+ 1);
		//	    std::cout<<"popular score: "<<score<<std::endl;
		return true;
	}

	bool rankRealTimeTerm(uint32_t termId, float& score)
	{
		float sliceScore = 0;
		if (latestSliceLangModel_.find(termId) != latestSliceLangModel_.end())
			sliceScore = latestSliceLangModel_[termId];
		score = log(static_cast<float> (sliceScore)
				/ (totalSliceTermCount_ + 1) + 1);
		//		std::cout<<"real time score: "<<score<<std::endl;
		return true;
	}

	bool computeReferEntropyScore()
	{
		std::cout << "start to compute entropy score" << std::endl;
		REFER_LANG_MODEL_CURSOR_TYPE locn = referLangModel_->get_first_locn();
		uint32_t key;
		TIME_SERIES_FREQ_TYPE vecFreq;
		while (referLangModel_->get(locn, key, vecFreq))
		{
			referLangModel_->seq(locn);
			float entropy = 0;
			computeReferEntropyScore(key, vecFreq, entropy);
			referEntropyScore_.insert(std::make_pair(key, entropy));
			referNormalizeScore_ += entropy;
		}
		return true;
	}

	bool computeReferEntropyScore(uint32_t key,
			const TIME_SERIES_FREQ_TYPE& vecFreq, float& score)
	{
		uint32_t totalCount = 0;
		for (uint32_t i = 0; i < vecFreq.size(); i++)
		{
			totalCount += vecFreq[i].second;
		}
		if (totalCount == 0)
		{
			score = 0.001;
			return false;
		}
		for (uint32_t i = 0; i < vecFreq.size(); i++)
		{
			if (vecFreq[i].second != 0)
			{
				float prob = vecFreq[i].second / (float) totalCount;
				score += prob * log(prob);
			}
		}
		score = -score;
		return true;
	}

	bool getReferEntropyScore(uint32_t key, float& score)
	{
		if (referEntropyScore_.find(key) == referEntropyScore_.end())
		{
			score = 0;
			return false;
		}
		else
		{
			score = referEntropyScore_[key];
		}
		return true;
	}

	bool getNoveltyScore(const std::vector<uint32_t>& termIdList, float& score)
	{
		float klScore = 0;
		float avgScore = klScore;
		for (size_t i = 0; i < termIdList.size(); i++)
		{
			uint32_t periodScore = 0;
			if (periodLangModel_.find(termIdList[i]) == periodLangModel_.end())
				periodScore = 0;
			else
				periodScore = periodLangModel_[termIdList[i]];
			if (periodScore < 1)
				periodScore = 0;

			uint32_t sliceScore = 0;
			if (latestSliceLangModel_.find(termIdList[i])
					!= latestSliceLangModel_.end())
				sliceScore = latestSliceLangModel_[termIdList[i]];
			//			float sliceProb=static_cast<float>(sliceScore)/totalSliceTermCount_;
			//			float periodProb=static_cast<float>(periodScore)/totalPeriodTermCount_;
			//	        klScore=sliceProb*log((sliceProb+0.00001)/(periodProb+0.00001));
			//	        klScore=log((sliceProb+0.00001)/(periodProb+0.00001));
			//			klScore=sqrt((sliceProb+0.00001)/(periodProb+0.00001));
			//			klScore=(sliceProb+0.00001)/(periodProb+0.00001);
			klScore = static_cast<float> (sliceScore + 1) / (periodScore + 1);
			avgScore += klScore;
		}
		if (termIdList.size() != 0)
			avgScore /= termIdList.size();
		else
			avgScore = 0;

		score = avgScore;
		return true;
	}

	bool deduplicate(std::vector<izenelib::util::UString>& candidates, size_t resultNum)
	{
        std::vector<izenelib::util::UString> selectedItems;
        for(size_t i=0;i<candidates.size()&&selectedItems.size()<resultNum;i++)
        {
        	if(!isDuplicate(candidates[i], selectedItems))
        	{
        		selectedItems.push_back(candidates[i]);
        	}
        }
        candidates.swap(selectedItems);
		return true;
	}

	bool isDuplicate(const izenelib::util::UString& candidate, const std::vector<izenelib::util::UString>& selectedItems)
	{
		for(size_t i=0;i<selectedItems.size();i++)
		{
			if(isDuplicate(candidate, selectedItems[i]))
				return true;
		}
		return false;
	}

	bool isDuplicate(const izenelib::util::UString& candidate1, const izenelib::util::UString& candidate2)
	{
		std::vector<uint32_t> termIdList1;
		idManager_->getAnalysisTermIdList(candidate1, termIdList1);
		std::vector<uint32_t> termIdList2;
		idManager_->getAnalysisTermIdList(candidate2, termIdList2);
		{
			for(size_t i=0;i<termIdList1.size();i++)
			{
				if(isDuplicate(termIdList1[i], termIdList2))
					return true;
			}
		}
		return false;

	}

	bool isDuplicate(uint32_t term, const std::vector<uint32_t>& referTerms)
	{
		for(size_t i=0;i<referTerms.size();i++)
		{
			if(term==referTerms[i])
				return true;
		}
		return false;
	}

private:

	IDManagerType* idManager_;
	boost::unordered_map<uint32_t, uint32_t> latestSliceLangModel_;
	boost::unordered_map<uint32_t, uint32_t> periodLangModel_;
	boost::unordered_map<uint32_t, float> referEntropyScore_;
	//	boost::shared_ptr<LANG_MODEL_TYPE> backgroundLangModel_;
	boost::shared_ptr<REFER_LANG_MODEL_TYPE> referLangModel_;
	boost::shared_ptr<QUERY_COUNT_TYPE> latestSliceCount_;
	boost::shared_ptr<QUERY_COUNT_TYPE> periodCount_;
	REMIND_INDEX_TYPE remindQueryIndex_;
	size_t realTimeNum_;
	size_t popularNum_;
	size_t totalSliceTermCount_;
	size_t totalPeriodTermCount_;
	float referNormalizeScore_;
private:
	static const size_t DEFAULT_REAL_TIME_QUERY_NUM = 50;
	static const size_t DEFAULT_POPULAR_QUERY_NUM = 50;

};

}

MAKE_FEBIRD_SERIALIZATION( idmlib::TIME_SERIES_FREQ_TYPE)

#endif /* LOGPROCESSOR_H_ */
