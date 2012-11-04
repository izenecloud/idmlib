///
/// @file algorithm_def.h
/// @brief Some definition used for KPE algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-09
/// @date Updated 2010-04-09
///

#ifndef _IDMKPEALGORITHM_DEF_H_
#define _IDMKPEALGORITHM_DEF_H_

#include "../idm_types.h"
#include "../util/Util.hpp"
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <am/external_sort/izene_sort.hpp>
#include <am/sequence_file/SimpleSequenceFile.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
NS_IDMLIB_KPE_BEGIN
// #define LOG_BEGIN(item, pReader) p=0; std::cout<<item<<" total: "<<(pReader)->getItemCount()<<" items"<<std::endl;
// #define LOG_PRINT(item, count) if( p % count == 0 ) { std::cout << "\r"; std::cout<<item<<" read "<<p<<" items"<<std::flush;}
// #define LOG_END() std::cout<<std::endl;
#define LOG_BEGIN(item, pReader) p=0; MEMLOG( (item+std::string(" total: %lu items.")).c_str(), (pReader)->getItemCount());
#define LOG_PRINT(item, count) if( p % count == 0 ) { MEMLOG( (item+ std::string(" processed %lu items.")).c_str(), p);}
#define LOG_END() 

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

class KPStatus
{
    public:
        static const int RETURN = 1;
        static const int NON_KP = 2;
        static const int CANDIDATE = 3;
        static const int KP = 4;
};

class Data
{
  public:
  uint32_t inc;
  idvec_t termid_list;
  id2countvec_t docitem_list;
  uint32_t freq;
  id2countvec_t lc_list;//left context list
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & inc & termid_list & docitem_list & freq & lc_list;
  }
};

// typedef boost::tuple<uint32_t, std::vector<uint32_t> ,std::vector<id2count_t>, 
//         uint32_t, std::vector<id2count_t > > data_t;

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

// typedef boost::tuple<idvec_t, id2countvec_t, uint32_t, id2countvec_t, id2countvec_t > CandidateItem;
    
typedef izenelib::am::SSFType<hash_t, CandidateItem, uint32_t> CandidateSSFType;

typedef std::pair<hash_t, uint8_t> Hash2HashItem;

typedef izenelib::am::SSFType<hash_t, Hash2HashItem, uint16_t> Hash2HashSSFType;

typedef std::pair<uint32_t, uint8_t> Hash2CountItem;

typedef izenelib::am::SSFType<hash_t, Hash2CountItem, uint8_t> Hash2CountSSFType;

typedef izenelib::am::SimpleSequenceFileMerger<hash_t, uint32_t, Hash2HashItem, uint8_t, uint16_t> HashCountMerger;

typedef boost::tuple<uint32_t, uint32_t, uint32_t, uint32_t> HashCountListItem;

typedef izenelib::am::SSFType<hash_t, HashCountListItem, uint8_t> HashCountListSSFType;


class KPItem
{
  public:
  KPItem()
  {
  }
  KPItem(const izenelib::util::UString & p1, const id2countvec_t& p2, uint8_t p3, const id2countvec_t& p4, const id2countvec_t& p5)
  :text(p1), docitem_list(p2), score(p3), lc_list(p4), rc_list(p5)
  {
  }
  izenelib::util::UString text;
  id2countvec_t docitem_list;
  uint8_t score;
  id2countvec_t lc_list;//left context list
  id2countvec_t rc_list;//right context list
  
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version)
  {
      ar & text & docitem_list & score & lc_list & rc_list;
  }
};

// typedef boost::tuple<izenelib::util::UString, std::vector<id2count_t>, uint8_t, std::vector<id2count_t>, std::vector<id2count_t> > KPItem;

typedef izenelib::am::SSFType<hash_t, KPItem, uint32_t> KPSSFType;


typedef izenelib::am::SimpleSequenceFileMerger<hash_t, CandidateItem, HashCountListItem, uint32_t, uint8_t> merger_t;        
        
        
        
        
        
class SortedFragmentItem
{
    public:
        SortedFragmentItem()
        {
        }
        SortedFragmentItem(uint32_t type, const std::vector<uint32_t>& termIdList)
        : type_(type), termIdList_(termIdList), docItemList_(0), freq_(0), prefixTermList_(0)
        {
            
            
        }
        
        static uint32_t parseInc(const std::vector<uint32_t>& value)
        {
            return value[0];
        }
        
        static std::vector<uint32_t> parseTermIdList(const std::vector<uint32_t>& value)
        {
            uint32_t termLen = value[2];
            uint32_t len = termLen;
            uint32_t start = 3;
            std::vector<uint32_t> tmpIdList(value.begin()+start, value.begin()+start+len);
            return tmpIdList;
            
        }
        
        static std::vector<id2count_t> parseDocItemList(const std::vector<uint32_t>& value)
        {
            
            uint32_t termLen = value[2];
            uint32_t len = value[3+termLen];;
            uint32_t start = 4+termLen;
            std::vector<id2count_t> result(len);
            for(uint32_t i=0;i<len;i++)
            {
                result[i].first = value[start+i];
            }
            start = start+len;
            for(uint32_t i=0;i<len;i++)
            {
                result[i].second = value[start+i];
            }
            
            return result;
        }
        
        static uint32_t parseFreq(const std::vector<uint32_t>& value)
        {
            
            return value[1];
        }
        
        static std::vector<std::pair<uint32_t, uint32_t> > parsePrefixTermList(const std::vector<uint32_t>& value)
        {
            uint32_t termLen = value[2];
            uint32_t docLen = value[3+termLen]*2;
            uint32_t start = 4+termLen+docLen;
            std::vector<std::pair<uint32_t, uint32_t> > tmpIdList( (value.size()-start) /2 );
            std::pair<uint32_t, uint32_t> pairItem;
            for(uint32_t i=0;i<tmpIdList.size();i++)
            {
                
                pairItem.first = value[start];
                pairItem.second = value[start+1];
                start += 2;
                tmpIdList[i] = pairItem;
            }
            return tmpIdList;
        }
        
        void addDocId(uint32_t docId, uint32_t count = 1)
        {
            docItemList_.push_back(id2count_t(docId, count) );
            freq_ += count;
        }

        
        void addPrefixTerm(uint32_t term, uint32_t freq = 1 )
        {
            prefixTermList_.push_back(std::make_pair(term, freq));
        }
        
        void getIncrement(const std::vector<uint32_t>& termIdList, uint32_t& inc)
        {
            uint32_t len = std::min(termIdList_.size(), termIdList.size());
            inc = len;
            for(uint32_t i=0;i<len;i++)
            {
                if(termIdList_[i]!=termIdList[i])
                {
                    inc = i;
                    break;
                }
            }

        }
        
        
        void getValue(std::vector<uint32_t>& value)
        {
            value.resize(3);
            value[0] = type_;
            value[1] = freq_;
            value[2] = termIdList_.size()-type_;
            value.insert(value.end(), termIdList_.begin()+type_, termIdList_.end() );
            std::vector<uint32_t> docIdList;
            std::vector<uint32_t> tfInDocList;
            idmlib::util::getIdAndCountList(docItemList_, docIdList, tfInDocList);
            
            value.push_back(docIdList.size());
            value.insert( value.end(), docIdList.begin(), docIdList.end() );
            value.insert( value.end(), tfInDocList.begin(), tfInDocList.end() );
            
            if( prefixTermList_.size() >0 )
            {
                std::sort(prefixTermList_.begin(), prefixTermList_.end());
                std::pair<uint32_t, uint32_t> output(0,0);
                for(uint32_t i=0;i<prefixTermList_.size();i++)
                {
                    if(i==0)
                    {
                        output.first = prefixTermList_[i].first;
                        output.second = prefixTermList_[i].second;
                    }
                    else
                    {
                        if( output.first == prefixTermList_[i].first )
                        {
                            output.second += prefixTermList_[i].second;
                        }
                        else
                        {
                            value.push_back(output.first);
                            value.push_back(output.second);
                            output.first = prefixTermList_[i].first;
                            output.second = prefixTermList_[i].second;
                        }
                    }
                }
                value.push_back(output.first);
                value.push_back(output.second);
            }
        }
    public:
        uint32_t type_;
        std::vector<uint32_t> termIdList_;
        std::vector<id2count_t> docItemList_;
        uint32_t freq_;
        std::vector<id2count_t > prefixTermList_;
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

    NgramInCollection& operator+=(const NgramInCollection& ngram)
    {
        if(term_list.empty())
        {
            term_list = ngram.term_list;
        }
        docitem_list.insert(docitem_list.end(), ngram.docitem_list.begin(), ngram.docitem_list.end());
        lc_list.insert(lc_list.end(), ngram.lc_list.begin(), ngram.lc_list.end());
        freq+=ngram.freq;
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

struct DocKpItem
{
    uint32_t id;
    izenelib::util::UString text;
    double score;
};
        
NS_IDMLIB_KPE_END

#endif 
