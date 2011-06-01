///
/// @file storage.h
/// @brief for tdt index and retrieval
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-05-24
/// @date Updated 2011-05-24
///

#ifndef _IDMLIB_TDT_STORAGE_H_
#define _IDMLIB_TDT_STORAGE_H_

#include "../idm_types.h"

#include <algorithm>
#include <cmath>
#include <am/tokyo_cabinet/tc_hash.h>
#include "tdt_types.h"
#include <boost/date_time/gregorian/gregorian.hpp>
NS_IDMLIB_TDT_BEGIN

class Storage
{
typedef uint32_t TopicIdType;
typedef izenelib::util::UString StringType;
typedef std::vector<StringType> VectorStringType;

typedef izenelib::am::tc_hash<TimeIdType, VectorStringType> Time2TopicsContainerType;
typedef izenelib::am::tc_hash<StringType, TopicInfoType> TopicsContainerType;

    public:
        
        Storage(const std::string& dir)
        :dir_(dir), time2topics_container_(NULL), topics_container_(NULL)
        {
            
        }
        
        ~Storage()
        {
            if(time2topics_container_!=NULL)
            {
                delete time2topics_container_;;
            }
            if(topics_container_!=NULL)
            {
                delete topics_container_;;
            }
        }
        
        bool Open()
        {
            try
            {
                boost::filesystem::create_directories(dir_);
            }
            catch(std::exception& ex)
            {
                std::cerr<<ex.what()<<std::endl;
            }
            time2topics_container_ = new Time2TopicsContainerType(dir_+"/time2topics_container");
            if(!time2topics_container_->open()) 
            {
                std::cerr<<"time2topics_container open error"<<std::endl;
                return false;
            }
            
            topics_container_ = new TopicsContainerType(dir_+"/topics_container");
            if(!topics_container_->open()) 
            {
                std::cerr<<"topics_container open error"<<std::endl;
                return false;
            }
            
            return true;
        }
        
        bool Add(const TrackResult& tr, const std::vector<izenelib::util::UString>& similar_topics)
        {
            TopicInfoType info(tr, similar_topics);
            topics_container_->update(tr.text, info);
            for(uint32_t i=0;i<tr.burst.size();i++)
            {
                const BurstItem& bi = tr.burst[i];
                for(uint32_t t=0;t<bi.period;t++)
                {
                    TimeIdType time = bi.start_time+DD(t);
                    if(!AddTimeTopic_(time, tr.text)) return false;
                }
            }
            return true;
        }
        
        bool GetTopicsInTimeRange(const TimeIdType& start, const TimeIdType& end, VectorStringType& vec)
        {
            DD dd = end - start;
            int days = dd.days();
            if(days<0) return false;
            for(int i=0;i<days;i++)
            {
                TimeIdType time = start+DD(i);
                VectorStringType tmp;
                time2topics_container_->get(time, tmp);
                vec.insert(vec.end(), tmp.begin(), tmp.end());
            }
            std::sort(vec.begin(), vec.end());
//             VectorStringType::iterator uit = std::unique(vec.begin(), vec.end());
            vec.erase(std::unique(vec.begin(), vec.end()), vec.end()); 
            return true;
        }
        
        bool GetTopicInfo(const StringType& text, TopicInfoType& info)
        {
            return topics_container_->get(text, info);
        }
        
        void Flush()
        {
            topics_container_->flush();
            time2topics_container_->flush();
        }
        
        
        
    private:
        
        bool AddTimeTopic_(const TimeIdType& time, const StringType& text)
        {
            VectorStringType vec;
            time2topics_container_->get(time, vec);
            vec.push_back(text);
            time2topics_container_->update(time, vec);
            return true;
        }
        
    private:
        std::string dir_;
        Time2TopicsContainerType* time2topics_container_;
        TopicsContainerType* topics_container_;
};

NS_IDMLIB_TDT_END

#endif 
