#ifndef IDMLIB_B5M_NGRAMDICTIONARY_H_
#define IDMLIB_B5M_NGRAMDICTIONARY_H_
#include "b5m_helper.h"
#include "b5m_types.h"
#include "product_matcher.h"
#include <am/sequence_file/ssfr.h>
#include <sf1common/ScdWriter.h>
#include <boost/shared_ptr.hpp>
#include <util/izene_serialization.h>
//#define NGRAMP_DEBUG

NS_IDMLIB_B5M_BEGIN

using izenelib::util::UString;


template <class Value>
class NgramDictionary {

    typedef uint32_t term_t;
    typedef std::vector<term_t> word_t;
    typedef std::map<word_t, Value> Map;

public:
    typedef typename Map::iterator iterator;
    void Insert(const word_t& key, const Value& v)
    {
        //boost::unique_lock<boost::mutex> lock(mutex_);
        map_.insert(std::make_pair(key, v));
    }

    iterator begin()
    {
        return map_.begin();
    }

    iterator end()
    {
        return map_.end();
    }

    std::size_t Size() const
    {
        return map_.size();
    }

    void Load(const std::string& path)
    {
        izenelib::am::ssf::Reader<> reader(path);
        reader.Open();
        Value value;
        while(reader.Next(value))
        {
            map_.insert(std::make_pair(value.word, value));
            //map_[pair.first] = pair.second;
        }
        reader.Close();
        std::cerr<<"[LOADED]"<<map_.size()<<std::endl;
    }

    void Flush(const std::string& path)
    {
        izenelib::am::ssf::Writer<> writer(path);
        writer.Open();
        for(typename Map::const_iterator it = map_.begin();it!=map_.end();it++)
        {
            writer.Append(it->second);
        }
        writer.Close();
    }


    //forward maximum match
    void Search(const word_t& key, term_t prefix, std::vector<Value>& values)
    {
        word_t found;
        Value foundv;
        word_t frag(1, prefix);
        std::size_t pos=0;
        std::size_t last_found_pos = 0;
        while(true)
        {
            if(pos>=key.size()) break;
            frag.push_back(key[pos]);
            int status = -1;
            typename Map::const_iterator it = map_.lower_bound(frag);
            if(it==map_.end()) status = -1;
            else
            {
                if(boost::algorithm::starts_with(it->first, frag))
                {
                    if(it->first==frag) status = 1;
                    else status = 0;
                }
                else status = -1;
            }
            if(status==-1)
            {
                if(!found.empty())
                {
                    values.push_back(foundv);
                    found.resize(0);
                    //pos = last_found_pos+1;
                }
                pos = last_found_pos+1;
                last_found_pos = pos;
                //pos = std::max(last_found_pos, pos+1-frag.size()-1);
                //++pos;
                frag.resize(1);
            }
            else if(status==0)
            {
                //continue
                ++pos;
            }
            else
            {
                found = frag;
                foundv = it->second;
                //foundv.word.assign(it->first.begin()+1, it->first.end());//now nv has 'word'
                //all mode
                //values.push_back(foundv);
                //found.clear();

                //min mode
                //values.push_back(foundv);
                //found.clear();
                //frag.resize(0);

                last_found_pos = pos;
                ++pos;
            }
            //std::cerr<<"[POS]"<<status<<","<<pos<<","<<last_found_pos<<","<<frag.size()<<","<<found.size()<<","<<foundv.word.size()<<std::endl;
        }
        if(!found.empty())
        {
            values.push_back(foundv);
            found.resize(0);
        }
    }


private:
    Map map_;
    boost::mutex mutex_;

};

NS_IDMLIB_B5M_END

#endif

