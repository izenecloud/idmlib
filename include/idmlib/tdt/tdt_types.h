///
/// @file tdt_types.h
/// @brief Some definition used for TDT algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-04-19
/// @date Updated 2011-04-19
///

#ifndef _IDMLIB_TDT_TYPES_H_
#define _IDMLIB_TDT_TYPES_H_

#include "../idm_types.h"
#include "../util/Util.hpp"
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <am/external_sort/izene_sort.hpp>
#include <am/sequence_file/SimpleSequenceFile.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/gregorian/greg_serialize.hpp>
NS_IDMLIB_TDT_BEGIN

#define LOG_BEGIN2(item, pReader) p=0; MEMLOG( (item+std::string(" total: %lu items.")).c_str(), (pReader)->Count());
#define LOG_PRINT2(item, count) if( p % count == 0 ) { MEMLOG( (item+ std::string(" processed %lu items.")).c_str(), p);}
#define LOG_END2() 
typedef uint64_t hash_t;
typedef std::pair<uint32_t, uint32_t> id2count_t;
typedef std::vector<uint32_t> idvec_t;
typedef std::vector<id2count_t> id2countvec_t;
typedef izenelib::am::SSFType<uint32_t, uint32_t, uint16_t, true> TermListSSFType;

typedef TermListSSFType::WriterType TermListWriter;

typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashSSFType;

typedef HashSSFType::WriterType HashWriter;

typedef boost::gregorian::date TimeIdType;

typedef boost::gregorian::date_duration DD;

class DateRange
{
    public:
        DateRange()
        {
        }
        
        DateRange(const TimeIdType& p1, const TimeIdType& p2)
        :start(p1), end(p2)
        {
        }
        TimeIdType start;
        TimeIdType end;
        
        bool Contains(const TimeIdType& time) const
        {
            return time>=start && time<=end;
        }
        
        int RangeCount() const
        {
            DD dd = end-start;
            int days = dd.days();
            return days;
        }
};

class TermInNgram
{
    public:
        TermInNgram():id(0), tag(0)
        {
        }
        TermInNgram(uint32_t p1):id(p1), tag(0)
        {
        }
        TermInNgram(uint32_t p1, char p2):id(p1), tag(p2)
        {
        }
    uint32_t id;
    char tag;
    
//     friend class boost::serialization::access;
//     template<class Archive>
//     void serialize(Archive & ar, const unsigned int version)
//     {
//         ar & id & tag;
//     }

    bool operator<(const TermInNgram& tn) const
    {
        return id<tn.id;
    }
    
    bool operator==(const TermInNgram& tn) const
    {
        return id==tn.id;
    }
    
    bool operator!=(const TermInNgram& tn) const
    {
        return id!=tn.id;
    }
};

class Ngram
{
  public:
    Ngram():term_list(), docid(0)
    {
    }
    
    Ngram(const std::vector<TermInNgram>& p1, uint32_t p2):term_list(p1), docid(p2)
    {
    }
    
    Ngram(const std::vector<TermInNgram>& p1, uint32_t p2, const TermInNgram& p3):term_list(p1), docid(p2), left_term(p3)
    {
    }
    
    std::vector<TermInNgram> term_list;
    uint32_t docid;
    TermInNgram left_term;
        
        
    void ToUint32List(std::vector<uint32_t>& list) const
    {
        for(std::size_t i=0;i<term_list.size();i++)
        {
            list.push_back(term_list[i].id);
        }
        list.push_back(0);
        list.push_back(docid);
        uint32_t num_of_32bit = term_list.size()/4;
        if(term_list.size()%4>0) num_of_32bit++;
        uint32_t extended_size = num_of_32bit*4;
        char* data = new char[extended_size];
        char* p_data = data;
        uint32_t s = 0;
//         std::cout<<"[TO-tag] ";
        for(;s<term_list.size();s++)
        {
            char tag = term_list[s].tag;
//             std::cout<<tag<<",";
            memcpy( p_data, &tag, 1 );
            p_data++;
        }
        
        for(;s<extended_size;s++)
        {
            char tag = 0;
            memcpy( p_data, &tag, 1 );
            p_data++;
        }
        p_data = data;
        for(s=0;s<num_of_32bit;s++)
        {
            uint32_t v = 0;
            memcpy( &v, p_data, 4 );
            list.push_back(v);
            p_data+=4;
        }
        delete[] data;
        if(left_term.id!=0)
        {
            list.push_back(left_term.id);
            list.push_back((uint32_t)left_term.tag);
//             std::cout<<left_term.tag;
        }
//         std::cout<<std::endl;
    }
    
    uint32_t GetInc(const Ngram& ngram) const
    {
        uint32_t count = term_list.size()>ngram.term_list.size()? ngram.term_list.size():term_list.size();
        uint32_t i=0;
        for(;i<count;i++)
        {
            if(term_list[i]!=ngram.term_list[i]) break;
        }
        return i;
    }
    
    static Ngram ParseUint32List(const std::vector<uint32_t>& list)
    {
//         std::cout<<"[parse-source] ";
//         for(uint32_t i=0;i<list.size();i++)
//         {
//             std::cout<<list[i]<<",";
//         }
//         std::cout<<std::endl;
        
        Ngram ngram;
        std::size_t i=0;
        for(;i<list.size();i++)
        {
            if(list[i]==0) break;
            TermInNgram tn(list[i], 0);
            ngram.term_list.push_back(tn);
        }
        i++;
        ngram.docid = list[i];
        i++;
        uint32_t num_of_32bit = ngram.term_list.size()/4;
        if(ngram.term_list.size()%4>0) num_of_32bit++;
//         uint32_t extended_size = num_of_32bit*4;
        uint32_t v = 0;
//         std::cout<<"[PARSE-tag] ";
        for(uint32_t s=0;s<ngram.term_list.size();s++)
        {
            uint32_t shift = s%4;
            if(shift==0)
            {
                v = list[i];
                i++;
            }
            char* p_tag = (char*)&v;
            p_tag += shift;
            memcpy( &ngram.term_list[s].tag, p_tag, 1);
//             std::cout<<ngram.term_list[s].tag<<",";
        }
        if(i<list.size()-1)
        {
            ngram.left_term.id = list[i];
            i++;
            ngram.left_term.tag = (char)list[i];
//             std::cout<<ngram.left_term.tag;
        }
//         std::cout<<std::endl;
//         std::cout<<"[I]"<<i<<","<<list.size()<<std::endl;
        return ngram;
    }
};

class NgramInCollection
{
public:
    NgramInCollection():inc(0), freq(0)
    {
    }
    
    uint32_t inc;
    std::vector<TermInNgram> term_list;
    id2countvec_t docitem_list;
    uint32_t freq;
    std::vector<std::pair<TermInNgram, uint32_t> > lc_list;//left context list
//     id2countvec_t lc_list;

//     friend class boost::serialization::access;
//     template<class Archive>
//     void serialize(Archive & ar, const unsigned int version)
//     {
//         ar & inc & term_list & docitem_list & freq & lc_list;
//     }
  
    uint32_t GetInc(const Ngram& ngram) const
    {
        uint32_t count = term_list.size()>ngram.term_list.size()? ngram.term_list.size():term_list.size();
        uint32_t i=0;
        for(;i<count;i++)
        {
            if(term_list[i]!=ngram.term_list[i]) break;
        }
        return i;
    }
    
    NgramInCollection& operator+=(const Ngram& ngram)
    {
        if(term_list.empty())
        {
            term_list = ngram.term_list;
        }
        docitem_list.push_back(std::make_pair(ngram.docid, 1));
        lc_list.push_back(std::make_pair(ngram.left_term, 1));
        ++freq;
        return *this;
    }
    
    bool IsEmpty() const
    {
        return term_list.empty();
    }
    
    void Clear()
    {
        inc = 0;
        term_list.clear();
        docitem_list.clear();
        freq = 0;
        lc_list.clear();
    }
    
    void Flush()
    {
        idmlib::util::accumulateList(docitem_list);
        idmlib::util::accumulateList(lc_list);
    }
};


class CandidateItem
{
  public:
  CandidateItem()
  {
  }
  CandidateItem(const idvec_t& p1, const id2countvec_t& p2, uint32_t p3, const id2countvec_t& p4, const id2countvec_t& p5)
  :termid_list(p1), docitem_list(p2), freq(p3), lc_list(p4), rc_list(p5)
  {
  }
  idvec_t termid_list;
  id2countvec_t docitem_list;
  uint32_t freq;
  id2countvec_t lc_list;//left context list
  id2countvec_t rc_list;//right context list
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & termid_list & docitem_list & freq & lc_list & rc_list;
  }
};

  

typedef std::pair<hash_t, uint8_t> Hash2HashItem;


typedef std::pair<uint32_t, uint8_t> Hash2CountItem;


typedef boost::tuple<uint32_t, uint32_t, uint32_t, uint32_t> HashCountListItem;



class OutputItem
{
  public:
  OutputItem()
  {
  }
  OutputItem(const izenelib::util::UString & p1, const id2countvec_t& p2)
  :text(p1), timeitem_list(p2)
  {
  }
  izenelib::util::UString text;
  id2countvec_t timeitem_list;
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & text & timeitem_list;
  }
};

class BurstItem
{
    public:
        BurstItem():period(0)
        {
        }
        
        TimeIdType start_time;
        uint32_t period;
        std::vector<double> weight;
        
        bool empty() const
        {
            return period==0;
        }
        
        void reset()
        {
            start_time = TimeIdType();
            period = 0;
            weight.clear();
        }
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & start_time & period & weight;
        }
};

class TrackResult
{
    public:
        izenelib::util::UString text;
        std::vector<BurstItem> burst;
        std::vector<uint32_t> ts;
        TimeIdType start_time;
        
        void LimitTsInRange(const TimeIdType& start, const TimeIdType& end)
        {
            DateRange range(start, end);
            std::vector<uint32_t> new_ts;
            TimeIdType new_start;
            bool set_new = false;
            for(uint32_t shift = 0;shift<ts.size();shift++)
            {
                TimeIdType time = GetTimeByShift(start_time, shift);
                if(range.Contains(time))
                {
                    if(!set_new)
                    {
                        new_start = time;
                        set_new = true;
                    }
                    new_ts.push_back(ts[shift]);
                }
            }
            start_time = new_start;
            ts = new_ts;
        }
        
        inline TimeIdType GetTimeByShift(uint32_t shift)
        {
            return GetTimeByShift(start_time, shift);
        }
        
        inline TimeIdType GetTimeByShift(const TimeIdType& start, uint32_t shift)
        {
            return start+DD(shift);
        }
        
        friend class boost::serialization::access;
        template<class Archive>
        void serialize(Archive & ar, const unsigned int version)
        {
            ar & text & burst & ts & start_time;
        }
};

        
        
class BackupDocument
{
public:
    BackupDocument()
    {
    }
    BackupDocument(const TimeIdType& ptime, uint32_t pdocid, const izenelib::util::UString& ptitle,                    const izenelib::util::UString& pcontent)
    :time(ptime), docid(pdocid), title(ptitle), content(pcontent)
    {
    }
    TimeIdType time;
    uint32_t docid;
    izenelib::util::UString title;
    izenelib::util::UString content;
    
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & time & docid & title & content;
    }
};



typedef std::pair<TrackResult, std::vector<izenelib::util::UString> > TopicInfoType;
        
NS_IDMLIB_TDT_END

#endif 
