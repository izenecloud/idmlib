///
/// @file algorithm.hpp
/// @brief The first version of KPE algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef _IDMKPEALGORITHM_HPP_
#define _IDMKPEALGORITHM_HPP_

#include "input.hpp"
#include "output.hpp"
#include "scorer.hpp"
#include "algorithm_def.h"
#include "../idm_types.h"
#include <boost/type_traits.hpp>
#include "../util/FSUtil.hpp"
#include <stack>
NS_IDMLIB_KPE_BEGIN

template <class IDManagerType, class Output>
class Algorithm1 : public boost::noncopyable
{
typedef uint32_t id_type;
typedef char pos_type;
typedef izenelib::util::UString string_type;
public:
typedef Scorer<IDManagerType> ScorerType;

    Algorithm1(IDManagerType* idManager, const Output& outputParam, const std::string& working)
    : idManager_(idManager), output_(outputParam), dir_(working), maxLen_(7)
    , cache_size_(0), cache_vec_(cache_size_)
    , lastDocId_(0), allTermCount_(0), docCount_(0)
    , scorer_(NULL), outside_scorer_(false)
    {
      init_();
    }
    
    ~Algorithm1()
    {
        release_();
        if( !outside_scorer_ && scorer_ != NULL)
        {
            delete scorer_;
            scorer_ = NULL;
        }
    }
    
    void set_cache_size(uint32_t size)
    {
      cache_size_ = size;
      cache_vec_.resize(cache_size_);
      cache_vec_.resize(0);
    }
    
    void tune(uint8_t maxLen)
    {
        maxLen_ = maxLen;
    }
    
    void load(const std::string& resPath)
    {
      if( scorer_ == NULL )
      {
        scorer_ = new ScorerType(idManager_);
        scorer_->load(resPath);
        outside_scorer_ = false;
      }
    }
    
    void set_scorer(ScorerType* scorer)
    {
      scorer_ = scorer;
      outside_scorer_ = true;
    }
    
    void insert(const std::vector<id_type>& idList, const std::vector<pos_type>& posInfoList, const std::vector<uint32_t>& positionList, uint32_t docId = 1)
    {
        if( idList.size() == 0 ) return;
        std::vector<string_type> termList(idList.size());
        for(uint32_t i=0;i<idList.size();i++)
        {
            bool b = idManager_->getTermStringByTermId(idList[i], termList[i]);
            if(!b)
            {
                std::cout<<"Can not get term string for id : "<<idList[i]<<std::endl;
                return;
            }
        }
        
        uint32_t begin = 0;
        while( addTerms_(docId, termList, idList, posInfoList, positionList, begin) ) {}
        
    }


    void insert(const std::vector<Term>& termList, uint32_t docId = 1 )
    {
        if( termList.size() == 0 ) return;
        for(uint32_t i=0;i<termList.size();i++)
        {
            if( termList[i].id_ == 0 )
            {
                bool b = idManager_->getTermIdByTermString(termList[i].text_, termList[i].tag_, termList[i].id_ );
                if(!b)
                {
                    std::string str;
                    termList[i].text_.convertString(str, izenelib::util::UString::UTF_8);
                    std::cout<<"Can not get term id for string : "<<str<<std::endl;
                    return;
                }
            }
        }
//         std::cout<<"{$$$insert} ";
//         for(uint32_t i=0;i<termList.size();i++)
//         {
//             std::string str;
//             termList[i].text_.convertString(str, izenelib::util::UString::UTF_8);
//             std::cout<<"["<<str<<","<<termList[i].tag_<<","<<termList[i].id_<<","<<termList[i].position_<<"]";
//         }
//         std::cout<<std::endl;
        uint32_t begin = 0;
        while( addTerms_(docId, termList, begin) ) {}
    }
    
    void insert(const std::vector<string_type>& termList, const std::vector<pos_type>& posInfoList, const std::vector<uint32_t>& positionList, uint32_t docId = 1)
    {
        if( termList.size() == 0 ) return;
        std::vector<id_type> idList(termList.size());
        for(uint32_t i=0;i<termList.size();i++)
        {
            bool b = idManager_->getTermIdByTermString(termList[i], posInfoList[i], idList[i] );
            if(!b)
            {
                std::string str;
                termList[i].convertString(str, izenelib::util::UString::UTF_8);
                std::cout<<"Can not get term id for string : "<<str<<std::endl;
                return;
            }
        }
        
        uint32_t begin = 0;
        while( addTerms_(docId, termList, idList, posInfoList, positionList, begin) ) {}
        
    }
    
    void insert( const izenelib::util::UString& article, uint32_t docId = 1)
    {
        std::vector<string_type> termList;
        std::vector<uint32_t> idList;
        std::vector<pos_type> posInfoList;
        std::vector<uint32_t> positionList;
        idManager_->getAnalysisTermIdList(article, termList, idList, posInfoList, positionList);
        uint32_t begin = 0;
        while( addTerms_(docId, termList, idList, posInfoList, positionList, begin) ) {}
    }
    
    void close()
    {
      izenelib::util::ClockTimer clocker;
        releaseCachedHashItem_();
        pTermListWriter_->close();
        pHashWriter_->close();
        std::string inputItemPath = pTermListWriter_->getPath();
        std::string hashItemPath = pHashWriter_->getPath();
        delete pTermListWriter_;
        pTermListWriter_ = NULL;
        delete pHashWriter_;
        pHashWriter_ = NULL;
        uint32_t docCount = getDocCount();
        minFreq_ = 1;
        minDocFreq_ = 1;
        if( docCount >=10 )
        {
            minFreq_ = (uint32_t)std::floor( std::log( (double)docCount )/2 );
            if( minFreq_ < 3 ) minFreq_ = 3;
            
            minDocFreq_ = (uint32_t)std::floor( std::log( (double)docCount )/4 );
            if( minDocFreq_ < 2 ) minDocFreq_ = 2;
        }
        uint64_t p = 0;
        typename HashSSFType::SorterType hsorter;
        hsorter.sort(hashItemPath);
        std::cout<<"[KPE1] "<<clocker.elapsed()<<" seconds."<<std::endl;
        clocker.restart();
        typedef izenelib::am::SSFType<hash_t, uint32_t, uint8_t> HashItemCountSSFType;
        typename HashItemCountSSFType::WriterType hcwriter(dir_+"/HCWRITER");
        hcwriter.open();
        {
            HashSSFType::ReaderType hreader(hashItemPath);
            hreader.open();
            hash_t hashId;
            hash_t saveId;
            uint32_t count = 0;
            uint32_t icount = 1;
            LOG_BEGIN("Hash", &hreader);
            while( true )
            {
              bool b = true;
              if( cache_size_>0)
              {
                b = hreader.next(hashId, icount);
              }
              else
              {
                b = hreader.nextKey(hashId);
              }
              if( !b ) break;
              if(p==0)
              {
                  saveId = hashId;
              }
              else
              {
                  if( saveId != hashId )
                  {
                      hcwriter.append(saveId, count);
                      saveId = hashId;
                      count = 0;
                  }
              }
              count += icount;
              p++;
              LOG_PRINT("Hash", 100000);
            }
            LOG_END();
            hcwriter.append(saveId, count);
            hreader.close();
            idmlib::util::FSUtil::del(hashItemPath);
            
        }
        hcwriter.close();
        std::cout<<"[KPE2] "<<clocker.elapsed()<<" seconds."<<std::endl;
        clocker.restart();
        typename TermListSSFType::SorterType fisorter;
        fisorter.sort(inputItemPath);
        std::cout<<"[KPE3] "<<clocker.elapsed()<<" seconds."<<std::endl;
        clocker.restart();
        typedef izenelib::am::SSFType<uint32_t, uint32_t, uint32_t> SortedFragmentItemSSFType;
        typename SortedFragmentItemSSFType::WriterType swriter(dir_+"/SWRITER");
        swriter.open();
        {
            typename TermListSSFType::ReaderType fireader(inputItemPath);
            fireader.open();
            std::vector<uint32_t> keyList;

            LOG_BEGIN("Suffix", &fireader);
            SortedFragmentItem sItem;
            std::vector<uint32_t> lastTermIdList;
            while( fireader.nextKeyList(keyList) )
            {
                uint32_t zeroPos = 0;
                for(uint32_t i=0;i<keyList.size();i++)
                {
                    if( keyList[i] == 0 )
                    {
                        zeroPos = i;
                        break;
                    }
                }
                std::vector<uint32_t> termIdList(keyList.begin(), keyList.begin()+zeroPos);
                uint32_t docId = 0;
                uint32_t prefixTermId = 0;
                bool hasPrefix = false;
                if( zeroPos == keyList.size()-2 )
                {
                    docId = keyList[keyList.size()-1];
                    
                }
                else if( zeroPos == keyList.size()-3 )
                {
                    docId = keyList[keyList.size()-2];
                    prefixTermId = keyList[keyList.size()-1];
                    hasPrefix = true;
                }
                else
                {
                    //impossible, just a reminder.
                }
                if( p == 0 )
                {
                    SortedFragmentItem tmp(0, termIdList);
                    std::swap(sItem, tmp);
                }
                else
                {
                    uint32_t inc;
                    sItem.getIncrement(termIdList, inc);
                    if( inc != termIdList.size() )//the same termidlist
                    {
                        std::vector<uint32_t> value;
                        sItem.getValue(value);
                        swriter.append( value );
                        
                        std::vector<uint32_t> incTermIdList(sItem.termIdList_.begin(),
                        sItem.termIdList_.begin()+inc);
                        incTermIdList.insert(incTermIdList.end(), 
                        termIdList.begin()+inc, termIdList.end());
                        SortedFragmentItem tmp(inc, incTermIdList);
                        std::swap(sItem, tmp);
                    }
                }
                sItem.addDocId(docId, 1);
                if(hasPrefix)
                    sItem.addPrefixTerm(prefixTermId);
                
                p++;
                LOG_PRINT("Suffix", 10000);
            }
            LOG_END();
            std::vector<uint32_t> value;
            sItem.getValue(value);
            swriter.append(value);
            fireader.close();
            std::cout<<"Total output: "<<swriter.getItemCount()<<std::endl;
            idmlib::util::FSUtil::del(inputItemPath);
        }
        swriter.close();
        std::cout<<"[KPE4] "<<clocker.elapsed()<<" seconds."<<std::endl;
        clocker.restart();
        typename SortedFragmentItemSSFType::ReaderType reader(swriter.getPath());
        reader.open();
        std::vector<uint32_t> keyList;
        typename CandidateSSFType::WriterType hclWriter(dir_+"/HCLWRITER");
        hclWriter.open();
        typename Hash2HashSSFType::WriterType h2hWriter(dir_+"/H2HWRITER");
        h2hWriter.open();
        std::vector<uint32_t> termIdList(0);
        std::vector<uint32_t> docIdList(0);

        LOG_BEGIN("AAA", &reader);
        
        std::vector<data_t > data;
        double aaa_time = 0.0;
        izenelib::util::ClockTimer aaa_clocker;
        
        std::vector<uint32_t> completeTermIdList;
        while( reader.nextKeyList(keyList) )
        {
            uint32_t inc = SortedFragmentItem::parseInc(keyList);
            std::vector<uint32_t> tmpTermIdList = SortedFragmentItem::parseTermIdList(keyList);
            std::vector<id2count_t> tmpDocItemList = SortedFragmentItem::parseDocItemList(keyList);
            uint32_t tmpFreq = SortedFragmentItem::parseFreq(keyList);
            std::vector<id2count_t > prefixTermList = SortedFragmentItem::parsePrefixTermList(keyList);
            
            completeTermIdList.resize(inc);
            completeTermIdList.insert( completeTermIdList.end(), tmpTermIdList.begin(), tmpTermIdList.end() );
            
            if( inc==0 )
            {
                aaa_clocker.restart();
                getCandidateLabel2_(data, &hclWriter, &h2hWriter);
                aaa_time += aaa_clocker.elapsed();
                data.resize(0);
            }
//             data.push_back(boost::make_tuple(inc, tmpTermIdList, tmpDocItemList, tmpFreq, prefixTermList));
            data.push_back(boost::make_tuple(inc, completeTermIdList, tmpDocItemList, tmpFreq, prefixTermList));
            p++;
            LOG_PRINT("AAA", 1000);
                
        }
        aaa_clocker.restart();
        getCandidateLabel2_(data, &hclWriter, &h2hWriter);
        aaa_time += aaa_clocker.elapsed();
        LOG_END();
        hclWriter.close();
        h2hWriter.close();
        std::cout<<"[KPE5] "<<clocker.elapsed()<<" seconds."<<std::endl;
        std::cout<<"[KPE5-AAA] "<<aaa_time<<" seconds."<<std::endl;
        clocker.restart();
        {
            typename CandidateSSFType::SorterType sorter;
            sorter.sort(hclWriter.getPath());
        }
        {
            typename Hash2HashSSFType::SorterType sorter;
            sorter.sort(h2hWriter.getPath());
        }
        HashCountMerger hcMerger;
        hcMerger.setObj(hcwriter.getPath(), h2hWriter.getPath() );
        hash_t key = 0;
        std::vector<uint32_t> valueList1;
        std::vector<Hash2HashItem> valueList2;
        typename Hash2CountSSFType::WriterType h2cWriter( dir_+"/H2CWRITER" );
        h2cWriter.open();
        while( hcMerger.next( key, valueList1, valueList2) )
        {
            if( valueList1.size() == 0 )
            {
                //impossible, just a reminder.
                continue;
            }
            if( valueList1.size() > 1 )
            {
                //impossible, just a reminder.
                continue;
            }
            for(uint32_t i=0;i<valueList2.size();i++)
            {
                h2cWriter.append( valueList2[i].first, Hash2CountItem(valueList1[0],valueList2[i].second) );
                
            }
        }
        h2cWriter.close();
        {
            typename Hash2CountSSFType::SorterType sorter;
            sorter.sort( h2cWriter.getPath() );
        }
//         sleep(1000);
        typename HashCountListSSFType::WriterType hcltWriter( dir_+"/HCLTWRITER" );
        hcltWriter.open();
        {
            typename Hash2CountSSFType::ReaderType h2cReader(h2cWriter.getPath());
            h2cReader.open();
            Hash2CountItem hash2CountValue;
            LOG_BEGIN("HashCount", &h2cReader);
            HashCountListItem output(0, 0, 0, 0);
            hash_t key;
            hash_t saveKey;
            while( h2cReader.next(key, hash2CountValue) )
            {
                if(p==0)
                {
                    saveKey = key;
                }
                else
                {
                    if(saveKey != key )
                    {
                        hcltWriter.append(saveKey, output);
                        output = boost::make_tuple(0,0,0,0);
                        saveKey = key;
                    }
                }
                if( hash2CountValue.second == 1 )
                {
                    boost::get<0>(output) = hash2CountValue.first;
                }
                else if( hash2CountValue.second == 2 )
                {
                    boost::get<1>(output) = hash2CountValue.first;
                }
                else if( hash2CountValue.second == 3 )
                {
                    boost::get<2>(output) = hash2CountValue.first;
                }
                else if( hash2CountValue.second == 4 )
                {
                    boost::get<3>(output) = hash2CountValue.first;
                }
                else
                {
                    //impossible, just a reminder.
                }
                p++;
                LOG_PRINT("HashCount", 10000);
            }
            hcltWriter.append(saveKey, output);
            LOG_END();
            h2cReader.close();
            idmlib::util::FSUtil::del(h2cReader.getPath());
        }
        hcltWriter.close();
//             typename HashCandidateLabelItemSSFType::ReaderType tmpReader(hclWriter.getPath());
//             tmpReader.open();
//             while( tmpReader.next() )
//             {
//                 hash_t key;
//                 bool b = tmpReader.getCurrentKey(key);
//                 std::cout<<"TMP :: KEY :: "<<b<<"|||"<<key<<std::endl;
//             }
//             tmpReader.close();
        {
            merger_t finalMerger;
            finalMerger.setObj( hclWriter.getPath(), hcltWriter.getPath() );
            hash_t key = 0;
            std::vector<CandidateItem> valueList1;
            std::vector<HashCountListItem> valueList2;
            uint32_t n = getTermCount_();
            while( finalMerger.next(key, valueList1, valueList2) )
            {
                if( valueList1.size()==0 )
                {
                    //impossible, just a reminder.
                    continue;
                }
                if( valueList1.size()>1 )
                {
                    //we may face to a hash conflict, ignore them
                    continue;
                }
                
                if( valueList2.size()!=1 )
                {
                    //impossible, just a reminder.
                    continue;
                }
                CandidateItem& hlItem = valueList1[0];
                HashCountListItem& hclItem = valueList2[0];
                std::vector<uint32_t> termIdList = boost::get<0>(hlItem);
                
                uint32_t f = boost::get<3>(hlItem);
                idvec_t& leftTermIdList = hlItem.get<4>();
                idvec_t& leftTermCountList = hlItem.get<5>();
                idvec_t& rightTermIdList = hlItem.get<6>();
                idvec_t& rightTermCountList = hlItem.get<7>();
                if( termIdList.size()==1 )
                {
                    std::vector<uint32_t> docIdList = boost::get<1>(hlItem);
                    std::vector<uint32_t> tfInDocList = boost::get<2>(hlItem);
                    insertKP_( termIdList, docIdList, tfInDocList, getScore(f, termIdList.size(),0.0) , leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList);
                }
                else if(termIdList.size()==2 )
                {
                    uint32_t f1 = boost::get<0>(hclItem);
                    uint32_t f2 = boost::get<1>(hclItem);
                    if( f1==0 || f2==0 )
                    {
                        //impossible, just a reminder.
                        continue;
                    }
                    double logL = SS::logL(f,f1,f2,n);
                    double mi = SS::mi(f,f1,f2,n);
//                     std::cout<<"LogL: "<<logL<<" , MI: "<<mi<<std::endl;
                    if( logL>=10 && mi>=5 )
                    {
                        std::vector<uint32_t> docIdList = boost::get<1>(hlItem);
                        std::vector<uint32_t> tfInDocList = boost::get<2>(hlItem);
                        insertKP_( termIdList, docIdList, tfInDocList, getScore(f, termIdList.size(),logL) , leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList);
                    }
                }
                else if(termIdList.size()>2)
                {
                    uint32_t f1 = boost::get<0>(hclItem);
                    uint32_t f2 = boost::get<1>(hclItem);
                    uint32_t f3 = boost::get<2>(hclItem);
                    uint32_t f4 = boost::get<3>(hclItem);
                    if( f1==0 || f2==0 || f3==0 || f4==0 )
                    {
                        //impossible, just a reminder.
                        continue;
                    }
                    double logL = SS::logL(f,f1,f2,n);
                    if( logL>=10 && SS::logL(f,f1,f4,n)>=10 
                        && SS::logL(f,f3,f2,n)>=10 )
                    {
                        std::vector<uint32_t> docIdList = boost::get<1>(hlItem);
                        std::vector<uint32_t> tfInDocList = boost::get<2>(hlItem);
                        insertKP_( termIdList, docIdList,  tfInDocList, getScore(f, termIdList.size(),logL) , leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList);
                    }
                }
            }
        }
        //do clean
        {
            scorer_->flush();
            release_();
            init_();
        }
    }
    
    uint32_t getDocCount()
    {
        return docCount_;
    }
    
private:
    
    void init_()
    {
        boost::filesystem::create_directories(dir_);
        pTermListWriter_ = new TermListWriter(dir_+"/pInputItemWriter");
        pTermListWriter_->open();
        pHashWriter_ = new HashWriter(dir_+"/pHashItemWriter");
        pHashWriter_->open();
        lastDocId_ = 0; 
        allTermCount_ = 0; 
        docCount_ = 0;
    }
    
    void release_()
    {
        if( pTermListWriter_!= NULL)
        {
            delete pTermListWriter_;
            pTermListWriter_ = NULL;
        }
        if( pHashWriter_!= NULL)
        {
            delete pHashWriter_;
            pHashWriter_ = NULL;
        }
        boost::filesystem::remove_all(dir_);
    }
    
    
    bool addTerms_(uint32_t docId, const std::vector<string_type>& termList, const std::vector<uint32_t>& idList, const std::vector<pos_type>& posList, const std::vector<uint32_t>& positionList, uint32_t& iBegin)
    {
        uint32_t len = termList.size();
        std::vector<Term> terms(len);
        for( uint32_t i = 0; i<len; i++)
        {
          terms[i] = Term(termList[i], idList[i], posList[i], positionList[i] );
        }
        return addTerms_(docId, terms, iBegin);
    }
    
    bool addTerms_(uint32_t docId, const std::vector<Term>& termList, uint32_t& iBegin)
    {
        
        if(iBegin >= termList.size()) return false;
        std::vector<std::pair<bool,uint32_t> > splitVec;
        uint32_t i=iBegin;
        uint32_t _begin = iBegin;
        uint32_t insertTermId;
        for ( ;i<termList.size();i++ )
        {
            bool bSplit = scorer_->isSplitTerm(termList[i].text_, termList[i].tag_, termList[i].id_, insertTermId);
            splitVec.push_back(std::make_pair(bSplit, insertTermId));
            if( i == iBegin ) //first term
            {
                continue;
            }
            else
            {
                if(bSplit)
                {
                    break;
                }
                if( termList[i].position_ != termList[i-1].position_+1 )
                {
                    splitVec.erase( splitVec.end()-1 );
                    break;
                }
            }
            
        }
        iBegin = i;
        if( splitVec.size() == 0 ) return false;
        bool bFirstTerm = true;
        bool bLastTerm = true;
        if( splitVec.size() == 1 ) 
        {
            if( splitVec[0].first == true )
            {
                return true;
            }
        }
        else
        {
            bFirstTerm = !splitVec.front().first;
            bLastTerm = !splitVec.back().first;
            
        }
        std::vector<uint32_t> terms( splitVec.size() );
        for(uint32_t p=_begin;p<_begin+splitVec.size();p++)
        {
            uint32_t _index = p-_begin;
            terms[_index] = splitVec[_index].second;
            if( !idManager_->isAutoInsert() )
            {
                if( !splitVec[_index].first )
                {
                    idManager_->put(terms[_index], termList[p].text_);
                }
            }
        }
        addTerms_(docId, terms, bFirstTerm, bLastTerm);
        return true;


    }
    
    
    void addTerms_(uint32_t docId, const std::vector<uint32_t>& termList, bool bFirstTerm = true, bool bLastTerm = true)
    {
//         std::cout<<"### "<<termList.size()<<" "<<(int)bFirstTerm<<" "<<(int)bLastTerm<<std::endl;
        if( termList.size() == 0 ) return;
        if( termList.size() == 1 ) 
        {
            if( bFirstTerm && bLastTerm )
            {
                std::vector<uint32_t> inputItem(termList);
                inputItem.push_back(0);
                inputItem.push_back(docId);
                if( scorer_->prefixTest(inputItem) != KPStatus::RETURN)
                {
                    pTermListWriter_->append(inputItem);
                }
                appendHashItem_(hash_(inputItem));
                incTermCount_(1);
            }
            return;
        }
        if( termList.size() == 2 && !bFirstTerm && !bLastTerm ) 
        {
            return;
        }
        uint32_t start = 0;
        uint32_t end = termList.size();
        uint32_t increase = termList.size();
        if(!bFirstTerm)
        {
            increase--;
            start = 1;
        }
        if(!bLastTerm)
        {
            increase--;
        }
        
        for(uint32_t i=start; i<end; i++)
        {
            uint32_t len = std::min(end-i,(uint32_t)(maxLen_+1));
            std::vector<uint32_t> frag( termList.begin()+i, termList.begin()+i+len );
            frag.push_back(0);
            frag.push_back(docId);
            if( i!= 0 )
            {
                frag.push_back(termList[i-1]);
            }
            if( scorer_->prefixTest(frag) != KPStatus::RETURN)
            {
                pTermListWriter_->append(frag);
            }
            for(uint32_t j= i+1; j<= end; j++)
            {
                if( j-i >= maxLen_ ) continue;
                std::vector<uint32_t> ifrag( termList.begin()+i, termList.begin()+j );
                appendHashItem_(hash_(ifrag));
            }
            
        }

        
        incTermCount_(increase);
        if( docId != lastDocId_ )
        {
            setDocCount_( getDocCount() + 1 );
        }

        lastDocId_ = docId;


    }
    
    void appendHashItem_(hash_t hash_value)
    {
      if( cache_size_ > 0 )
      {
        if( cache_vec_.size() >= cache_size_ )
        {
          std::cout<<"[FULL]"<<std::endl;
          //output
          for(uint32_t i=0;i<cache_vec_.size();i++)
          {
            pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
          }
          //clean
          cache_vec_.resize(0);
          cache_map_.clear();
        }
        uint32_t* index=  cache_map_.find(hash_value);
        if( index == NULL)
        {
          cache_vec_.push_back( std::make_pair(hash_value, 1) );
          cache_map_.insert( hash_value, (uint32_t)(cache_vec_.size())-1 );
        }
        else
        {
          cache_vec_[*index].second += 1;
        }
      }
      else
      {
        pHashWriter_->append(hash_value);
      }
      
    }
    
    void releaseCachedHashItem_()
    {
      if( cache_size_ > 0 )
      {
        for(uint32_t i=0;i<cache_vec_.size();i++)
        {
          pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
        }
        //clean
        cache_vec_.resize(0);
        cache_map_.clear();
      }
    }
    
    void setDocCount_(uint32_t count)
    {
        docCount_ = count;
    }
    
    
    
    void incTermCount_(uint32_t inc = 1)
    {
        allTermCount_ += inc;
    }
    
    uint32_t getTermCount_()
    {
        return allTermCount_;
    }
    
    inline hash_t hash_(const std::vector<uint32_t>& termIdList)
    {
        return izenelib::util::HashFunction<std::vector<uint32_t> >::generateHash64BySer(termIdList);
    }
    
    inline const bool AstartWithB(const std::vector<uint32_t>& termIdList1, const std::vector<uint32_t>& termIdList2)
    {
        if(termIdList1.size()<termIdList2.size()) return false;
        for(uint32_t i=0;i<termIdList2.size();i++)
        {
            if( termIdList1[i] != termIdList2[i] ) return false;
        }
        return true;
    }
    
    bool makeKPStr_(const std::vector<uint32_t>& termIdList,
        std::vector<izenelib::util::UString>& strList,
        izenelib::util::UString& result)
    {
        bool b = makeKPStr_(termIdList, strList);
        if(!b) return false;
        b = makeKPStr_(strList, result);
        if(!b) return false;
        return true;
    }
    
    bool makeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<izenelib::util::UString>& strList)
    {
        if ( termIdList.empty() ) return false;//if empty
        strList.resize(termIdList.size());
        izenelib::util::UString ustr;
        bool bb = true;
        bb = idManager_->getTermStringByTermId(termIdList[0], strList[0]);
        if(!bb)
        {
            std::cout<<"!!!!can not find term id "<<termIdList[0]<<std::endl;
            return false;
        }
        for(std::size_t i=1;i<termIdList.size();i++)
        {
            bb = idManager_->getTermStringByTermId(termIdList[i], strList[i]);
            if(!bb)
            {
                return false;
            }
        }
        return true;
    }
        
    bool makeKPStr_(const std::vector<izenelib::util::UString>& strList, izenelib::util::UString& result)
    {
        result.clear();
        if(strList.size()>0)
        {
            if( strList[0].length() == 0 ) return false;
            result.append(strList[0]);
            for(std::size_t i=1;i<strList.size();i++)
            {
                if( strList[i].length() == 0 ) return false;
                if( result.isChineseChar( result.length()-1 ) 
                    && strList[i].isChineseChar(0) )
                {
                }
                else
                {
                    result.append(izenelib::util::UString(" ", izenelib::util::UString::UTF_8));
                }
                result.append(strList[i]);
            }
        }
        return true;
    }
    
    uint8_t getScore(uint32_t f, uint32_t termCount, double logL)
    {
        double score=sqrt(f> 200 ? 100 : f/2)*(termCount > 4 ? 4: termCount);
        if(logL>=10.0)
        {
            score *= sqrt(logL> 100 ? 100 : logL);
        }
        else
        {
            score *= 5;
        }
        return (uint8_t)(ceil(score)+1);
    }

    void insertKP_(const std::vector<uint32_t>& terms, const std::vector<uint32_t>& docIdList, 
                        const std::vector<uint32_t>& tfInDocList,  uint8_t score 
                        ,const idvec_t& leftTermIdList,const idvec_t& leftTermCountList
                        ,const idvec_t& rightTermIdList, const idvec_t& rightTermCountList)
    {
        assert(docIdList.size()==tfInDocList.size());
        std::vector<izenelib::util::UString> strList;
        izenelib::util::UString KPStr;
        bool validKP = makeKPStr_(terms, strList, KPStr);
        if(validKP)
        {
            std::vector<id2count_t> toInsert(docIdList.size());
            for(uint32_t i=0;i<toInsert.size();i++)
            {
                toInsert[i].first = docIdList[i];
                toInsert[i].second = tfInDocList[i];
            }
            output_.output(KPStr, toInsert, toInsert.size(), score , leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList);
        }
    }
    
    void getCandidateLabel_(const std::vector<data_t>& data, CandidateSSFType::WriterType* htlWriter, typename Hash2HashSSFType::WriterType* h2hWriter)
    {
        if( data.size()==0 ) return;
        
        std::vector<uint32_t> termIdList;
        std::vector<uint32_t> cpTermIdList;
        for( uint32_t m=0;m<data.size();m++)
        {
            uint32_t inc = boost::get<0>(data[m]);
            std::vector<uint32_t> tmpTermIdList = data[m].get<1>();
            std::vector<id2count_t> tmpDocItemList = boost::get<2>(data[m]);
            uint32_t tmpFreq = boost::get<3>(data[m]);
            std::vector<std::pair<uint32_t, uint32_t> > prefixTermList = boost::get<4>(data[m]);
            cpTermIdList.resize(inc);
            cpTermIdList.insert(cpTermIdList.end(), tmpTermIdList.begin(), tmpTermIdList.end());
            for( uint32_t i=1; i<=tmpTermIdList.size(); i++)
            {
                std::vector<id2count_t> docItemList(tmpDocItemList);
                uint32_t f = tmpFreq;
                termIdList.resize(inc);
                termIdList.insert(termIdList.end(), tmpTermIdList.begin(), tmpTermIdList.begin()+i);
                if( termIdList.size() > maxLen_ ) continue;
                std::vector<uint32_t> rTermIdList(cpTermIdList);
                std::vector<std::pair<uint32_t, uint32_t> > leftTermList(prefixTermList);
                
                std::vector<std::pair<uint32_t, uint32_t> > rightTermList;
                uint32_t last_right_termid = 0;
                if( i!= tmpTermIdList.size() )
                {
                    rightTermList.push_back(std::make_pair(tmpTermIdList[i], tmpFreq));
                    last_right_termid = tmpTermIdList[i];
                }
                
                
                for( uint32_t n=m+1; n<data.size(); n++ )
                {
                    uint32_t inc2 = boost::get<0>(data[n]);
                    
                    if( inc2 <= inc ) break;
                    else 
                    {
                      std::vector<uint32_t> tmpTermIdList2 = boost::get<1>(data[n]);
                      std::vector<id2count_t> tmpDocItemList2 = boost::get<2>(data[n]);
                      uint32_t tmpFreq2 = boost::get<3>(data[n]);
                      std::vector<std::pair<uint32_t, uint32_t> > prefixTermList2 = boost::get<4>(data[n]);
                      if( inc2 = inc+1 )
                      {
                        last_right_termid = tmpTermIdList2[0];
                      }
                      docItemList.insert(docItemList.end(), tmpDocItemList2.begin(), tmpDocItemList2.end());
                      f+=tmpFreq2;
                      leftTermList.insert(leftTermList.end(), prefixTermList2.begin(),prefixTermList2.end() );
                      if( rightTermList.empty() )
                      {
                        rightTermList.push_back(std::make_pair(last_right_termid, tmpFreq2));
                      }
                      else
                      {
                        if(rightTermList.back().first == last_right_termid )
                        {
                          rightTermList.back().second += tmpFreq2;
                        }
                        else
                        {
                          rightTermList.push_back(std::make_pair(last_right_termid, tmpFreq2));
                        }
                      }
                    }
                    
//                     std::vector<uint32_t> tmpTermIdList2 = boost::get<1>(data[n]);
//                     std::vector<id2count_t> tmpDocItemList2 = boost::get<2>(data[n]);
//                     uint32_t tmpFreq2 = boost::get<3>(data[n]);
//                     std::vector<std::pair<uint32_t, uint32_t> > prefixTermList2 = boost::get<4>(data[n]);
//                     rTermIdList.resize(inc2);
//                     rTermIdList.insert(rTermIdList.end(), tmpTermIdList2.begin(), tmpTermIdList2.end());
//                     if( AstartWithB( rTermIdList, termIdList ) )
//                     {
//                         docItemList.insert(docItemList.end(), tmpDocItemList2.begin(), tmpDocItemList2.end());
//                         f+=tmpFreq2;
//                         //or not?
//                         leftTermList.insert(leftTermList.end(), prefixTermList2.begin(),prefixTermList2.end() );
//                         
//                         if( termIdList.size() < rTermIdList.size() )
//                         {
//                             rightTermList.push_back(std::make_pair(rTermIdList[termIdList.size()], tmpFreq2));
//                         }
//                     }
//                     else
//                     {
//                         break;
//                     }
                }
                
                std::vector<uint32_t> docIdList;
                std::vector<uint32_t> tfInDocList;
                idmlib::util::getIdAndCountList(docItemList, docIdList, tfInDocList);
                                    
                std::vector<uint32_t> leftTermIdList;
                std::vector<uint32_t> leftTermCountList;
                idmlib::util::getIdAndCountList(leftTermList, leftTermIdList, leftTermCountList);
                
                std::vector<uint32_t> rightTermIdList;
                std::vector<uint32_t> rightTermCountList;
                idmlib::util::getIdAndCountList(rightTermList, rightTermIdList, rightTermCountList);
                
                SI lri(termIdList, f);
                SCI leftLC( leftTermIdList, leftTermCountList, f, true);
                SCI rightLC( rightTermIdList, rightTermCountList, f, false);
                int status = KPStatus::CANDIDATE;
                if( f<minFreq_ ) status = KPStatus::NON_KP;
                else if( docIdList.size() < minDocFreq_ ) status = KPStatus::NON_KP;
                else
                {
                    status = scorer_->prefixTest(termIdList);
                }
                
                if( status == KPStatus::NON_KP || status == KPStatus::RETURN )
                {
                    continue;
                }
                if( status == KPStatus::KP )
                {
                    insertKP_( termIdList, docIdList, tfInDocList, f, leftTermIdList, leftTermCountList, rightTermIdList, rightTermCountList );
                    continue;
                }
                std::pair<bool, double> lcdResult = scorer_->test(lri, leftLC);
                if( !lcdResult.first ) continue;
                std::pair<bool, double> rcdResult = scorer_->test(lri, rightLC);
                if( !rcdResult.first ) continue;
                
//                     bool isTest = false;
//                     if( termIdList.size() == testTermIdList_.size() )
//                     {
//                         isTest = true;
//                         for(uint32_t u=0;u<termIdList.size();u++)
//                         {
//                             if( termIdList[u] != testTermIdList_[u] )
//                             {
//                                 isTest = false;
//                                 break;
//                             }
//                         }
//                     }
//                     if(isTest)
//                     {
//                         std::cout<<"{TEST-LCD} "<<leftLC.getString(idManager_)<<" ("<<lcdResult.second<<")"<<std::endl;
//                         std::cout<<"{TEST-RCD} "<<rightLC.getString(idManager_)<<" ("<<rcdResult.second<<")"<<std::endl;
//                     }
                
                CandidateItem htList(termIdList, docIdList, tfInDocList, f, leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList );
                hash_t hashId = hash_(termIdList);
                htlWriter->append(hashId , htList );
                if( termIdList.size() >= 2 )
                {
                    {
                        std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                        Hash2HashItem item(hashId, 1);
                        h2hWriter->append( hash_(vec), item);
                    }
                    {
                        std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                        Hash2HashItem item(hashId, 2);
                        h2hWriter->append( hash_(vec), item);
                    }
                }
                if( termIdList.size() >= 3 )
                {
                    {
                        std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                        Hash2HashItem item(hashId, 3);
                        h2hWriter->append( hash_(vec), item);
                    }
                    {
                        std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                        Hash2HashItem item(hashId, 4);
                        h2hWriter->append( hash_(vec), item);
                    }
                }
            }
//             break;
        }
            
            
        
    }
    
    //with complete termidlist version.
    void getCandidateLabel2_(const std::vector<data_t>& data, CandidateSSFType::WriterType* htlWriter, typename Hash2HashSSFType::WriterType* h2hWriter)
    {
        if( data.size()==0 ) return;
        
        //sorting with postorder
        std::vector<uint32_t> post_order(data.size());
        {
          std::stack<uint32_t> index_stack;
          std::stack<uint32_t> depth_stack;
          uint32_t i=0;
          for( uint32_t m=0;m<data.size();m++)
          {
            uint32_t depth = boost::get<0>(data[m]);
            if(m>0)
            {
              while( !depth_stack.empty() && depth_stack.top()<= depth )
              {
                post_order[i] = index_stack.top();
                ++i;
                
                index_stack.pop();
                depth_stack.pop();
              }
            }
            index_stack.push(m);
            depth_stack.push(depth);
          }
          while( !index_stack.empty() )
          {
            post_order[i] = index_stack.top();
            ++i;
            
            index_stack.pop();
            depth_stack.pop();
          }
        }
        
        std::stack<uint32_t> depth_stack;
        std::stack<uint32_t> freq_stack;
        std::stack<std::vector<id2count_t> > doc_item_stack;
        std::stack<std::vector<id2count_t> > prefix_term_stack;
        std::stack<id2count_t > suffix_term_stack;
        
        for( uint32_t m=0;m<data.size();m++)
        {
          uint32_t depth = boost::get<0>(data[post_order[m]]);
          std::vector<uint32_t> termIdList = data[post_order[m]].get<1>();
          std::vector<id2count_t> docItemList = boost::get<2>(data[post_order[m]]);
          uint32_t freq = boost::get<3>(data[post_order[m]]);
          std::vector<id2count_t > prefixTermList = boost::get<4>(data[post_order[m]]);
          std::vector<id2count_t > suffixTermList;
          while( !depth_stack.empty() && depth < depth_stack.top() )
          {
            freq+=freq_stack.top();
            docItemList.insert( docItemList.end(), doc_item_stack.top().begin(), doc_item_stack.top().end() );
            prefixTermList.insert( prefixTermList.end(), prefix_term_stack.top().begin(), prefix_term_stack.top().end() );
            //suffix howto?
            //suffixTermList, suffix_term_stack
//             suffixTermList.insert( suffixTermList.end(), suffix_term_stack.top().begin(), suffix_term_stack.top().end() );
            suffixTermList.push_back(suffix_term_stack.top() );
            //pop stack
            depth_stack.pop();
            freq_stack.pop();
            doc_item_stack.pop();
            prefix_term_stack.pop();
            suffix_term_stack.pop();
          }
          depth_stack.push( depth);
          freq_stack.push(freq);
          doc_item_stack.push( docItemList);
          prefix_term_stack.push(prefixTermList);
          //get the suffix term
          uint32_t suffix_termid = termIdList[depth];
          suffix_term_stack.push(std::make_pair(suffix_termid, freq) );
          
          if( termIdList.size() > maxLen_ ) continue;
          std::vector<uint32_t> docIdList;
          std::vector<uint32_t> tfInDocList;
          idmlib::util::getIdAndCountList(docItemList, docIdList, tfInDocList);
                              
          std::vector<uint32_t> leftTermIdList;
          std::vector<uint32_t> leftTermCountList;
          idmlib::util::getIdAndCountList(prefixTermList, leftTermIdList, leftTermCountList);
          
          std::vector<uint32_t> rightTermIdList;
          std::vector<uint32_t> rightTermCountList;
          idmlib::util::getIdAndCountList(suffixTermList, rightTermIdList, rightTermCountList);
          
          SI lri(termIdList, freq);
          SCI leftLC( leftTermIdList, leftTermCountList, freq, true);
          SCI rightLC( rightTermIdList, rightTermCountList, freq, false);
          int status = KPStatus::CANDIDATE;
          if( freq<minFreq_ ) status = KPStatus::NON_KP;
          else if( docIdList.size() < minDocFreq_ ) status = KPStatus::NON_KP;
          else
          {
              status = scorer_->prefixTest(termIdList);
          }
          
          if( status == KPStatus::NON_KP || status == KPStatus::RETURN )
          {
              continue;
          }
          if( status == KPStatus::KP )
          {
              insertKP_( termIdList, docIdList, tfInDocList, freq, leftTermIdList, leftTermCountList, rightTermIdList, rightTermCountList );
              continue;
          }
          std::pair<bool, double> lcdResult = scorer_->test(lri, leftLC);
          if( !lcdResult.first ) continue;
          std::pair<bool, double> rcdResult = scorer_->test(lri, rightLC);
          if( !rcdResult.first ) continue;
          
          CandidateItem htList(termIdList, docIdList, tfInDocList, freq, leftTermIdList, leftTermCountList,rightTermIdList, rightTermCountList );
          hash_t hashId = hash_(termIdList);
          htlWriter->append(hashId , htList );
          if( termIdList.size() >= 2 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                  Hash2HashItem item(hashId, 1);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                  Hash2HashItem item(hashId, 2);
                  h2hWriter->append( hash_(vec), item);
              }
          }
          if( termIdList.size() >= 3 )
          {
              {
                  std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                  Hash2HashItem item(hashId, 3);
                  h2hWriter->append( hash_(vec), item);
              }
              {
                  std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                  Hash2HashItem item(hashId, 4);
                  h2hWriter->append( hash_(vec), item);
              }
          }
            
        }
            
            
        
    }
    
    
private:
    IDManagerType* idManager_;
    Output output_;
    const std::string dir_;
    uint8_t maxLen_;
    
    TermListWriter* pTermListWriter_;
    HashWriter* pHashWriter_;
    uint32_t cache_size_;
    izenelib::am::rde_hash<hash_t, uint32_t> cache_map_;
    std::vector< std::pair<hash_t, uint32_t> > cache_vec_;
    uint32_t lastDocId_;
    
    uint32_t allTermCount_;
    uint32_t docCount_;
    uint32_t minFreq_;
    uint32_t minDocFreq_;
    
    ScorerType* scorer_;
    bool outside_scorer_;
    
    
};

NS_IDMLIB_KPE_END

#endif 
