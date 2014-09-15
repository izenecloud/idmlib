#ifndef IDMLIB_B5M_COMMENTSERVER_H_
#define IDMLIB_B5M_COMMENTSERVER_H_

#include <string>
#include <vector>
#include "b5m_types.h"
#include "b5m_helper.h"
#include <types.h>
#include <am/sequence_file/ssfr.h>
#include <glog/logging.h>
#include <boost/date_time/posix_time/posix_time.hpp>

NS_IDMLIB_B5M_BEGIN

class CommentServer
{
public:
    typedef uint128_t Key;
    typedef uint32_t Value;
    typedef izenelib::am::succinct::fujimap::Fujimap<Key, Value> Data;
    CommentServer(const std::string& path)
    : path_(path)
      , data_path_(path+"/data"), log_path_(path+"/log"), tmp_path_(path+"/tmp")
      , db_(NULL), ofs_(NULL), is_open_(false), has_modify_(false)
    {
    }

    ~CommentServer()
    {
        if(!flush())
        {
            LOG(ERROR)<<"flush error"<<std::endl;
        }
        if(db_!=NULL)
        {
            delete db_;
        }
        if(ofs_!=NULL) delete ofs_;
    }

    bool is_open() const
    {
        return is_open_;
    }

    bool open()
    {
        if(is_open_) return true;
        boost::filesystem::create_directories(path_);
        if(boost::filesystem::exists(tmp_path_))
        {
            boost::filesystem::remove_all(tmp_path_);
        }
        db_ = new Data(tmp_path_.c_str());
        db_->initFP(32);
        db_->initTmpN(30000000);
        if(boost::filesystem::exists(data_path_))
        {
            db_->load(data_path_.c_str());
        }
        if(boost::filesystem::exists(log_path_))
        {
            std::ifstream ifs(log_path_.c_str());
            std::string line;
            std::size_t count=0;
            while(getline(ifs, line))
            {
                boost::algorithm::trim(line);
                std::vector<std::string> vec;
                boost::algorithm::split(vec, line, boost::is_any_of(","));
                if(vec.size()!=2) continue;
                if(vec[0].length()!=32) continue;
                Key key = B5MHelper::StringToUint128(vec[0]);
                Value value = boost::lexical_cast<Value>(vec[1]);
                db_->setInteger(key, value, true);
                count++;
            }
            ifs.close();
            LOG(INFO)<<"recover "<<count<<" records from log"<<std::endl;
            if(count>0)
            {
                //if(db_->build()==-1)
                //{
                //    LOG(ERROR)<<"db build error"<<std::endl;
                //    return false;
                //}
                boost::filesystem::remove_all(data_path_);
                db_->save(data_path_.c_str());
                const static int wait_ms = 100;
                boost::this_thread::sleep( boost::posix_time::milliseconds(wait_ms) );
                //B-00-201109081700-11111-I-C.SCD
                boost::posix_time::ptime now(boost::posix_time::microsec_clock::local_time());
                //20020131T100001,123456789
                std::string iso_str = boost::posix_time::to_iso_string(now);
                std::string new_log_path = path_+"/"+iso_str+".log";
                boost::filesystem::rename(log_path_, new_log_path);
            }
            boost::filesystem::remove_all(log_path_);
        }
        ofs_ = new std::ofstream(log_path_.c_str());
        is_open_ = true;
        return true;
    }

    void set(const std::string& skey, const Value& v)
    {
        Value ev;
        if(get(skey, ev))
        {
            if(ev==v) return;
        }
        Key key = B5MHelper::StringToUint128(skey);
        boost::unique_lock<boost::shared_mutex> lock(mutex_);
        db_->setInteger(key, v, true);
        (*ofs_)<<skey<<","<<v<<std::endl;
        has_modify_ = true;
    }
    bool get(const std::string& skey, Value& v)
    {
        Key key = B5MHelper::StringToUint128(skey);
        {
            boost::shared_lock<boost::shared_mutex> lock(mutex_);
            v = db_->getInteger(key);
        }
        if(v==(Value)izenelib::am::succinct::fujimap::NOTFOUND)
        {
            return false;
        }
        return true;
    }



    bool flush()
    {
        LOG(INFO)<<"try flush comment server.."<<std::endl;
        if(has_modify_)
        {
            boost::unique_lock<boost::shared_mutex> lock(mutex_);
            //LOG(INFO)<<"building fujimap.."<<std::endl;
            //if(db_->build()==-1)
            //{
            //    LOG(ERROR)<<"build error"<<std::endl;
            //    return false;
            //}
            boost::filesystem::remove_all(data_path_);
            db_->save(data_path_.c_str());
            if(ofs_!=NULL)
            {
                ofs_->close();
                delete ofs_;
            }
            boost::filesystem::remove_all(log_path_);
            ofs_ = new std::ofstream(log_path_.c_str());
            has_modify_ = false;
        }
        else
        {
            LOG(INFO)<<"comment server not change, ignore"<<std::endl;
        }
        return true;
    }

private:

private:

    std::string path_;
    std::string data_path_;
    std::string log_path_;
    std::string tmp_path_;
    Data* db_;
    std::ofstream* ofs_;
    bool is_open_;
    bool has_modify_;
    boost::shared_mutex mutex_;
};
NS_IDMLIB_B5M_END

#endif


