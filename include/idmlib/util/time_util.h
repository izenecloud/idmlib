#ifndef IDMLIB_UTIL_TIMEUTIL_H_
#define IDMLIB_UTIL_TIMEUTIL_H_
#include <string>
#include <time.h>
#include <util/ustring/UString.h>
#include <am/3rdparty/rde_hash.h>
#include "../idm_types.h"
#include <boost/date_time/gregorian/gregorian.hpp>
NS_IDMLIB_UTIL_BEGIN


class TimeUtil
{
    public:
        static bool GetDateByUString(const izenelib::util::UString& text, boost::gregorian::date& date)
        {
            std::string str;
            text.convertString(str, izenelib::util::UString::UTF_8);
            return GetDateByString(str, date);
        }
        
        
        static bool GetDateByString(const std::string& text, boost::gregorian::date& date)
        {
            boost::gregorian::date r ;
            try
            {
                r = boost::gregorian::from_string(text);
            }
            catch(std::exception& ex)
            {
//                 r = boost::gregorian::date();
            }
            if(!r.is_not_a_date())
            {
                date = r;
                return true;
            }
            if(text.length()<8) return false;
            std::string cand_text = text.substr(0,8);
            try
            {
                r = boost::gregorian::from_undelimited_string(cand_text);
            }
            catch(std::exception& ex)
            {
                
            }
            if(!r.is_not_a_date())
            {
                date = r;
                return true;
            }
            return false;
        }
        
        static bool GetDateByInt(int64_t time_stamp, boost::gregorian::date& date)
        {
            time_t t = time_stamp;
            struct tm *gm = gmtime(&t);
            date = boost::gregorian::date_from_tm(*gm);
            return true;
        }
        
        static double GetCurrentTimeMS()
        {
            timeval now;
            gettimeofday(&now,0);

            time_t s = now.tv_sec;
            time_t mms = now.tv_usec;

            double ret = s + (double)mms/1000000.0;
            return ret;
        }

        static time_t GetCurrentTimeS()
        {
            return time(NULL);
        }

};
    
NS_IDMLIB_UTIL_END

#endif
