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

NS_IDMLIB_KPE_BEGIN
#define LOG_BEGIN(item, pReader) p=0; std::cout<<item<<" total: "<<(pReader)->getItemCount()<<" items"<<std::endl;
#define LOG_PRINT(item, count) if( p % count == 0 ) { std::cout << "\r"; std::cout<<item<<" read "<<p<<" items"<<std::flush;}
#define LOG_END() std::cout<<std::endl;
typedef uint64_t hash_t;
typedef std::vector<uint32_t> idvec_t;
typedef izenelib::am::SSFType<uint32_t, uint32_t, uint16_t, true> TermListSSFType;

typedef TermListSSFType::WriterType TermListWriter;

typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashSSFType;

typedef HashSSFType::WriterType HashWriter;

typedef std::pair<uint32_t, uint32_t> id2count_t;

typedef boost::tuple<uint32_t, std::vector<uint32_t> ,std::vector<id2count_t>, 
        uint32_t, std::vector<id2count_t > > data_t;

typedef boost::tuple<idvec_t, idvec_t, idvec_t, uint32_t, idvec_t, idvec_t, idvec_t, idvec_t > CandidateItem;
    
typedef izenelib::am::SSFType<hash_t, CandidateItem, uint32_t> CandidateSSFType;

typedef std::pair<hash_t, uint8_t> Hash2HashItem;

typedef izenelib::am::SSFType<hash_t, Hash2HashItem, uint16_t> Hash2HashSSFType;

typedef std::pair<uint32_t, uint8_t> Hash2CountItem;

typedef izenelib::am::SSFType<hash_t, Hash2CountItem, uint8_t> Hash2CountSSFType;

typedef izenelib::am::SimpleSequenceFileMerger<hash_t, uint32_t, Hash2HashItem, uint8_t, uint16_t> HashCountMerger;

typedef boost::tuple<uint32_t, uint32_t, uint32_t, uint32_t> HashCountListItem;

typedef izenelib::am::SSFType<hash_t, HashCountListItem, uint8_t> HashCountListSSFType;



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
        
        
NS_IDMLIB_KPE_END

#endif 
