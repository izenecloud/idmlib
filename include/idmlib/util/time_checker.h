/**
 * @file time_checker.h
 * @author Zhongxia Li
 * @date Apr 7, 2011
 * @brief 
 */

#ifndef TIME_CHECKER_H_
#define TIME_CHECKER_H_

//#define IMD_TIME_CHECKER
#ifdef IMD_TIME_CHECKER

#include <idmlib/idm_types.h>

NS_IDMLIB_UTIL_BEGIN

typedef map<string, std::pair<double, int> > TimerPool; // <timername, (sum,count)>
static TimerPool timerPool;
static ofstream of("TimeChecker.out");

class TimeChecker
{
public:
    timeval start_;
    timeval end_;
    std::string msg_;
private:
    bool printed_;
public:
    TimeChecker( const std::string& msg = std::string("") )
    : msg_(msg)
    , printed_(false)
    {
        gettimeofday(&start_, 0);
        gettimeofday(&end_, 0);
    }

    ~TimeChecker()
    {
        gettimeofday(&end_,0);

        if (!printed_)
            Print();
    }

    void StartPoint()
    {
        //start_ = time(NULL);
        gettimeofday(&start_,0);
    }

    void EndPoint()
    {
        //end_ = time(NULL);
        gettimeofday(&end_,0);
    }

    double Interval()
    {
        time_t s = (end_.tv_sec - start_.tv_sec); // seconds
        time_t mms = (end_.tv_usec - start_.tv_usec); // microseconds (fraction part)
        if (mms < 0) {
            mms += 1000000;
            s -= 1;
        }

        return (s + (double)mms / 1000000.0);
    }

    string ToTimeString()
    {
        double s = Interval();
        stringstream ss;
        ss << "[" << msg_ << " ] time elapsed: " << static_cast<int>(s) / 60 << " minutes " << s << " seconds " << endl;
        return ss.str();
    }

    void Print(bool toFile = true)
    {
        printed_ = true;
        time_t s = (end_.tv_sec - start_.tv_sec); // seconds
        time_t mms = (end_.tv_usec - start_.tv_usec); // microseconds (fraction part)
        if (mms < 0) {
            mms += 1000000;
            s -= 1;
        }
        time_t m = s / 60;
        double sec = s % 60 + mms/1000000.0;
        std::cout << "[" << msg_ << "] time elapsed: "
                  << m << " minutes " << sec << " seconds (" << Interval()<< ", " <<Interval() / 60 << " s)" << endl;

        // ....
//        if (toFile) {
//            of << ToTimeString();
//        }
        TimeChecker::AddTimer(msg_, Interval());
    }

   /**
    * timer analysis (timer count, sum, average)
    */
public:
    static void AddTimer(string& msg, double interval)
    {
        TimerPool::iterator iter = timerPool.find(msg);
        if (iter == timerPool.end())
        {
            timerPool.insert(std::make_pair(msg, std::make_pair(interval, 1)));
        }
        else {
            iter->second.first += interval;
            iter->second.second ++;
        }
    }

    static string ToAllString()
    {
        stringstream ss;
        ss << "timer                                            | called  | total time  | average time      " << endl;
        for (TimerPool::iterator iter = timerPool.begin(); iter != timerPool.end(); iter ++)
        {
            ss << "[" << setw(48) << iter->first <<"]"
             << setw(5) << iter->second.second
             << setw(15) << iter->second.first
             << setw(15) << iter->second.first / iter->second.second << endl;
        }
        return ss.str();
    }

    static void ReportToStd()
    {
        cout << ToAllString();
    }

    static void ReportToFile()
    {
        of << ToAllString();
        of.close();
    }
};

NS_IDMLIB_UTIL_END


#endif

#endif /* TIME_CHECKER_H_ */

