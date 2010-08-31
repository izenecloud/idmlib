///
/// @file   kpe_scorer.h
/// @brief  The scorer class for KPE
/// @author Jia Guo
/// @date   2010-04-09
/// @date   2010-04-09
///
#ifndef IDM_KPESCORER_H_
#define IDM_KPESCORER_H_
#include <string>

#include <ctime>
#include <boost/filesystem.hpp>
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <boost/bind.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/thread.hpp> 
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <cache/IzeneCache.h>
#include <util/izene_serialization.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include "../util/StopWordContainer.hpp"
#include "../util/Util.hpp"
#include "../util/idm_analyzer.h"
#include "kpe_term_group.h"
#include "../idm_types.h"
NS_IDMLIB_KPE_BEGIN

class ScorerContextItem
{
    public:
        ScorerContextItem():termIdList_(),contextCountList_(),weightList_(), f_(0), isPrefix_(true)
        {
        }
        
        ScorerContextItem(const std::vector<uint32_t>& termIdList, 
        const std::vector<uint32_t>& contextCountList, 
        uint32_t f, bool isPrefix = true):
        termIdList_(termIdList),contextCountList_(contextCountList),weightList_(contextCountList.size(), 1.0)
        , f_(f),isPrefix_(isPrefix)
        {
        }
        
        ScorerContextItem(const std::vector<uint32_t>& termIdList, const std::vector<uint32_t>& contextCountList, 
        const std::vector<double>& weightList, 
        uint32_t f, bool isPrefix = true):
        termIdList_(termIdList),contextCountList_(contextCountList),weightList_(weightList), f_(f),isPrefix_(isPrefix)
        {
        }

        template <class IDManager>
        std::string getString(IDManager* idManager) const
        {
            std::string result = "";
            if( isPrefix_ ) result += "L";
            else result+="R";
            result +=":";
            result += boost::lexical_cast<std::string>(f_);
            if( termIdList_.size() > 0 )
            {
                result += "|";
                for(uint32_t i=0;i<termIdList_.size()-1;i++)
                {
                    izenelib::util::UString ustr;
                    bool b = idManager->getTermStringByTermId(termIdList_[i], ustr);
                    if(b)
                    {
                        std::string str;
                        ustr.convertString(str, izenelib::util::UString::UTF_8);
                        result += str+":"+boost::lexical_cast<std::string>(contextCountList_[i])+"|";
                    }
                    else
                    {
                        result += boost::lexical_cast<std::string>(termIdList_[i])+":"+boost::lexical_cast<std::string>(contextCountList_[i])+"|";
                    }
                }
                izenelib::util::UString ustr;
                bool b = idManager->getTermStringByTermId(termIdList_.back(), ustr);
                if(b)
                {
                    std::string str;
                    ustr.convertString(str, izenelib::util::UString::UTF_8);
                    result += str+":"+boost::lexical_cast<std::string>(contextCountList_.back());
                }
                else
                {
                    result += boost::lexical_cast<std::string>(termIdList_.back())+":"+boost::lexical_cast<std::string>(contextCountList_.back());
                }
            }
            return result;
        }
        
        template <class IDManager>
        std::string getString2(IDManager* idManager) const
        {
            std::string direct = "R";
            if( isPrefix_ ) direct = "L";
            std::string result = "";
            if( termIdList_.size() > 0 )
            {
                for(uint32_t i=0;i<termIdList_.size();i++)
                {
                    izenelib::util::UString ustr;
                    bool b = idManager->getTermStringByTermId(termIdList_[i], ustr);
                    if(b)
                    {
                        std::string str;
                        ustr.convertString(str, izenelib::util::UString::UTF_8);
                        result += direct+"_"+str+"_"+boost::lexical_cast<std::string>(contextCountList_[i])+" ";
                    }
                }
                
            }
            return result;
        }
        
        
    public:
        std::vector<uint32_t> termIdList_;
        std::vector<uint32_t> contextCountList_;
        std::vector<double> weightList_;
        uint32_t f_;
        bool isPrefix_;

};
typedef ScorerContextItem SCI;
class ScorerItem
{
    public:
        ScorerItem():termIdList_(), f_(0)
        {
        }
        
        ScorerItem(const std::vector<uint32_t>& termIdList, 
        uint32_t f):
        termIdList_(termIdList), f_(f)
        {
        }
        
        template <class IDManager>
        std::string getString(IDManager* idManager, const std::string& split=" ") const
        {
            std::string result = "";
            if( termIdList_.size() > 0 )
            {
                for(uint32_t i=0;i<termIdList_.size()-1;i++)
                {
                    izenelib::util::UString ustr;
                    bool b = idManager->getTermStringByTermId(termIdList_[i], ustr);
                    if(b)
                    {
                        std::string str;
                        ustr.convertString(str, izenelib::util::UString::UTF_8);
                        result += str+split;
                    }
                    else
                    {
                        result += boost::lexical_cast<std::string>(termIdList_[i])+split;
                    }
                }
                izenelib::util::UString ustr;
                bool b = idManager->getTermStringByTermId(termIdList_.back(), ustr);
                if(b)
                {
                    std::string str;
                    ustr.convertString(str, izenelib::util::UString::UTF_8);
                    result += str;
                }
                else
                {
                    result += boost::lexical_cast<std::string>(termIdList_.back());
                }
            }
            return result;
        }
        
    public:
        std::vector<uint32_t> termIdList_;
        uint32_t f_;
        
};
typedef ScorerItem SI;

class StatisticalScorer : public boost::noncopyable
{
public:    
        //if term_group == NULL, means no group info
        StatisticalScorer(KPETermGroup* term_group):term_group_(term_group)
        {
            
        }
        double cd(const SCI& item)
        {
            uint32_t s = 0;
            double r = 0;
            double sum = item.f_;
            std::vector<uint32_t> countList;
            term_group_->filter(item.termIdList_, item.contextCountList_, countList);
            if( countList.size() == 0 )
            {
                return 100.0;
            }
            for (std::size_t i=0; i<countList.size();++i)
                s += countList[i];

            for (std::size_t i=0; i<countList.size();++i)
                r += countList[i]/sum*log(countList[i]/sum);
            double ind = sum - s;
        //     r += 30.0 * ind/sum*log(ind/sum);
            double d = (r+2*ind/sum*log(1/sum))*-1.;
            return d;
        }
        double cd2(const SCI& item)
        {
            uint32_t s = 0;
            double r = 0;
            double sum = item.f_;
            std::vector<uint32_t> countList;
            term_group_->filter(item.termIdList_, item.contextCountList_, countList);
            if( countList.size() == 0 )
            {
                return 100.0;
            }
            uint32_t maxCount = 0;
            for (std::size_t i=0; i<countList.size();++i)
            {
                s += countList[i];
                if( countList[i]> maxCount)
                {
                    maxCount = countList[i];
                }
            }

            for (std::size_t i=0; i<countList.size();++i)
                r += countList[i]/sum*log(countList[i]/sum);
            double ind = sum - s;
            double d = (r+2*ind/sum*log(1/sum))*-1.;
            if( ind == 0.0 )
            {
                double minus = sum*maxCount;
                minus = std::pow(minus, 0.66);
                minus /= 50;
                d -= minus;
            }
        //     if( d<0.0 ) d=0.0;
            return d;
        }
        inline static double mi(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            return ( double ) f* n / f1/f2;
        }
        inline static double logL(uint32_t f, uint32_t f1, uint32_t f2, uint32_t n)
        {
            uint32_t k1 = f;
            uint32_t k2 = 0;
            if ( f2>f ) k2 = f2-f;
            uint32_t n1 = f1;
            uint32_t n2 = n-n1;
            return ll_(k1, n1, k2, n2);
        }
    private:
        
        static double ll_ ( uint32_t k1, uint32_t n1, uint32_t k2, uint32_t n2 )
        {
            return ll_ ( ( double ) k1/n1,k1,n1 ) +
            ll_ ( ( double ) k2/n2,k2,n2 )-ll_ ( ( double ) ( k1+k2 ) / ( n1+n2 ),k1,n1 )-
            ll_ ( ( double ) ( k1+k2 ) / ( n1+n2 ),k2,n2 );
        }
        static double ll_ ( double p,uint32_t k,uint32_t n )
        {
            double v1 = 0.0;
            double v2 = 0.0;
            if( p==0 || k==0) v1 = 0.0;
            else
            {
                v1 = std::log ( p ) *k;
            }
            if( p==1 || k==n) v2 = 0.0;
            else
            {
                v2 = std::log ( 1-p ) * ( n-k );
            }
            return v1+v2;
        }
        
        KPETermGroup* term_group_;

        
};

class KPStatus
{
    public:
        static const int RETURN = 1;
        static const int NON_KP = 2;
        static const int CANDIDATE = 3;
        static const int KP = 4;
};


class LanguageScorer : public boost::noncopyable
{
    typedef KPETermGroup TermGroupType;
    public:
        LanguageScorer(const std::string& resPath, idmlib::util::IDMAnalyzer* analyzer)
        :analyzer_(analyzer), term_group_(new TermGroupType()), stat_scorer_(term_group_)
        {
            init_(resPath);
            
        }
        
        ~LanguageScorer()
        {
            delete term_group_;
        }
        
        uint32_t getArabicNumber() const
        {
            return arabicNumber_;
        }
        
        int prefixTest(const std::vector<uint32_t>& termIdList) 
        {
            uint32_t termId = termIdList.back();
            int result = KPStatus::CANDIDATE;
            if( termId == arabicNumber_ || termId == singleEnglishChar_ )
            {
                result = KPStatus::RETURN;
            }
            else if( termIdList.size() == 1 )
            {
                result = KPStatus::NON_KP;
            }
            else
            {
                uint32_t b1 = termIdList[0];
                uint32_t b2 = termIdList[1];
                uint64_t b1_b2 = idmlib::util::make64UInt(b1, b2);
                boost::tuple<float, float, float, float>* v = bigramStat_.find(b1_b2);
                if( v!= NULL )
                {
                    if( boost::get<1>(*v) < 0.1 )
                    {
                        result = KPStatus::RETURN;
                    }
                }
                else
                {
                    boost::tuple<float, float, float, float>* v1 = unigramStat_.find(b1);
                    boost::tuple<float, float, float, float>* v2 = unigramStat_.find(b2);
                    if( v1== NULL || v2 == NULL)
                    {
                        result = KPStatus::CANDIDATE;
                    }
                    else
                    {
                        double u_score = boost::get<0>(*v1) * boost::get<1>(*v2);
                        if( u_score < 0.01 )
                        {
                            result = KPStatus::RETURN;
                        }
                    }
                }
                
            }

            return result;
        }

        std::pair<bool, double> test(const SI& item,const SCI& citem) 
        {
            double LOGL_MIN = 1.099;
            double d = stat_scorer_.cd(citem);
            double d2 = stat_scorer_.cd2(citem);
            std::pair<bool, double> result(true, d2);
            if( d2 < LOGL_MIN )
            {
                result.first = false;
            }

            uint32_t maxCountId = 0;
            uint32_t maxCount = 0;
            for(uint32_t i=0;i<citem.termIdList_.size();i++)
            {
                if( citem.contextCountList_[i]>maxCount)
                {
                    maxCount = citem.contextCountList_[i];
                    maxCountId = citem.termIdList_[i];
                }
                
            }
            if(item.termIdList_.size()>1)
            {
                if( citem.isPrefix_ ) //LCD
                {
                    uint64_t b1_b2 = idmlib::util::make64UInt(item.termIdList_[0], item.termIdList_[1]);
                    boost::tuple<float, float, float, float>* v_item = bigramStat_.find(b1_b2);
                    if( v_item != NULL )
                    {
                        if( d >= LOGL_MIN && boost::get<1>(*v_item) >= 0.4 )
                        {
                            result.first = true;
                        }
                        else if( boost::get<1>(*v_item) < 0.1)
                        {
                            result.first = false;
                        }
                        else
                        {
                            uint64_t p_b1 = idmlib::util::make64UInt(maxCountId, item.termIdList_[0]);
                            v_item = bigramStat_.find(p_b1);
                            if( v_item != NULL )
                            {
                                if( boost::get<0>(*v_item) < 0.1 )
                                {
                                    result.first = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        boost::tuple<float, float, float, float>* b1_item = unigramStat_.find(item.termIdList_[0]);
                        boost::tuple<float, float, float, float>* b2_item = unigramStat_.find(item.termIdList_[1]);
                        if( result.first && b1_item != NULL)
                        {
                            if(boost::get<0>(*b1_item)<0.18 )
                            {
                                result.first = false;
                            }
                        }
                        
                        if( result.first && b2_item != NULL)
                        {
                            if(boost::get<1>(*b2_item)<0.18 )
                            {
                                result.first = false;
                            }
                        }
                    }
                }
                else
                {
                    uint64_t e2_e1 = idmlib::util::make64UInt(item.termIdList_[item.termIdList_.size()-2], item.termIdList_[item.termIdList_.size()-1]);
                    boost::tuple<float, float, float, float>* v_item = bigramStat_.find(e2_e1);
                    if( v_item != NULL )
                    {
                        if( d >= LOGL_MIN && boost::get<2>(*v_item) >= 0.4 )
                        {
                            result.first = true;
                        }
                        else if( boost::get<2>(*v_item) < 0.1)
                        {
                            result.first = false;
                        }
                        else
                        {
                            uint64_t e1_s = idmlib::util::make64UInt(item.termIdList_.back(), maxCountId);
                            v_item = bigramStat_.find(e1_s);
                            if( v_item != NULL )
                            {
                                if( boost::get<3>(*v_item) < 0.1 )
                                {
                                    result.first = false;
                                }
                            }
                        }
                    }
                    else
                    {
                        boost::tuple<float, float, float, float>* e2_item = unigramStat_.find(item.termIdList_[item.termIdList_.size()-2]);
                        boost::tuple<float, float, float, float>* e1_item = unigramStat_.find(item.termIdList_[item.termIdList_.size()-1]);
                        if( result.first && e2_item != NULL)
                        {
                            if(boost::get<2>(*e2_item)<0.18 )
                            {
                                result.first = false;
                            }
                        }
                        if( result.first && e1_item != NULL)
                        {
                            if(boost::get<3>(*e1_item)<0.18 )
                            {
                                result.first = false;
                            }
                        }
                    }
                }
            }
            else//one term
            {

            }
        //     if( result.first )
        //     {
        //         std::cout<<"[0] {"<<item.getString(idManager_,"")<<"} {"<<citem.getString(idManager_)<<"}. ["<<d<<","<<d2<<"]"<<std::endl;
        //     }
        //     else
        //     {
        //         std::string LR = "[L]";
        //         if( !citem.isPrefix_ )
        //         {
        //             LR = "[R]";
        //         }
        //         std::cout<<LR<<" {"<<item.getString(idManager_,"")<<"} {"<<citem.getString(idManager_)<<"}. ["<<d<<","<<d2<<"]"<<std::endl;
        //     }
                
            
            return result;
        }
       
    private:
        void init_(const std::string& resPath)
        {
          arabicNumber_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("ARABICNUMBER",izenelib::util::UString::UTF_8) );
          singleEnglishChar_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("SINGLEENGLISHCHAR",izenelib::util::UString::UTF_8) );
            
          {
//                 std::ifstream ifs ( (resPath + "/ubtrain").c_str() );
              std::istream* ifs = idmlib::util::getResourceStream(resPath+"/ubtrain");
              std::string word;
              uint32_t type;
              uint64_t id;
              float a;
              float b;
              float c;
              float d;
              while ( getline ( *ifs,word ) )
              {
                  std::vector<std::string> items;
                  boost::algorithm::split( items, word, boost::algorithm::is_any_of(",") );
                  if( items.size() != 7 ) continue;
                  type = boost::lexical_cast<uint32_t>(items[0]);
                  id = boost::lexical_cast<uint64_t>(items[2]);
                  a = boost::lexical_cast<float>(items[3]);
                  b = boost::lexical_cast<float>(items[4]);
                  c = boost::lexical_cast<float>(items[5]);
                  d = boost::lexical_cast<float>(items[6]);
                  if( type == 1 )
                  {
                      bigramStat_.insert( id, boost::make_tuple(a,b,c,d) );
                  }
                  else if( type == 2 )
                  {
                      uint32_t termId = (uint32_t) id;
                      unigramStat_.insert( termId, boost::make_tuple(a,b,c,d) );
                  }
              }
              delete ifs;
          }
        }
    private:
        idmlib::util::IDMAnalyzer* analyzer_;
        uint32_t arabicNumber_;
        uint32_t singleEnglishChar_;
        izenelib::am::rde_hash<uint64_t, boost::tuple<float, float, float, float> > bigramStat_;
        izenelib::am::rde_hash<uint32_t, boost::tuple<float, float, float, float> > unigramStat_;
        TermGroupType* term_group_;
        StatisticalScorer stat_scorer_;
};




class KPEScorer : public boost::noncopyable
{
    public:
        KPEScorer(idmlib::util::IDMAnalyzer* analyzer)
        :analyzer_(analyzer), langScorer_(NULL), swContainer_(NULL),
        invalidChnBigram_(new izenelib::am::rde_hash<uint64_t, bool>()), 
        nonAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>()), 
        midAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>())
        {
            
        }
        ~KPEScorer()
        {
            if( langScorer_ != NULL ) delete langScorer_;
            if( swContainer_ != NULL ) delete swContainer_;
            if( invalidChnBigram_ != NULL ) delete invalidChnBigram_;
            if( nonAppearTerms_ != NULL ) delete nonAppearTerms_;
            if( midAppearTerms_ != NULL ) delete midAppearTerms_;
        }
        void load(const std::string& resPath)
        {
            namespace bfs = boost::filesystem;
            langScorer_ = new LanguageScorer(resPath, analyzer_);
            swContainer_ = new idmlib::util::StopWordContainer();
            {
                std::istream* ifs = idmlib::util::getResourceStream(resPath+"/stop_words");
                std::string word;
                while ( getline ( *ifs,word ) )
                {
                    boost::to_lower(word);
                    if ( word.length() >0 )
                    {
//                       std::cout<<"add stop "<<word<<std::endl;
                        std::vector<uint32_t> termIdList;
                        analyzer_->GetIdList(izenelib::util::UString(word,izenelib::util::UString::UTF_8), termIdList);

                        if( termIdList.size() > 0 )
                        {
                            swContainer_->insert(termIdList);
                        }
                        
                    }
                }
                delete ifs;
            }
            
//             std::string chnBigramPath = resPath+"/invalid_chn_bigram.dic";
            std::istream* ifs = idmlib::util::getResourceStream(resPath+"/invalid_chn_bigram");
//             if ( !boost::filesystem::exists ( dicPath ) )
//             {
//                 throw ResourceNotFoundException(chnBigramPath, "LabelRecognizer");
//             }
//             std::ifstream ifs ( chnBigramPath.c_str() );
            std::string word;
            while ( getline ( *ifs,word ) )
            {
                if ( word.length() >0 )
                {
//                     std::cout<<"[KPE] "<<word<<std::endl;
                    if( word[0] == '@' ) break;
                    izenelib::util::UString ustr(word, izenelib::util::UString::UTF_8);
                    if( ustr.length()!= 2 ) continue;
                    if( !ustr.isChineseChar(0) ) continue;
                    if( !ustr.isChineseChar(1) ) continue;
                    izenelib::util::UString ustr1;
                    izenelib::util::UString ustr2;
                    ustr.substr(ustr1, 0,1);
                    ustr.substr(ustr2, 1,1);
                    uint32_t id1 = idmlib::util::IDMIdConverter::GetId(ustr1, idmlib::util::IDMTermTag::CHN);
                    uint32_t id2 = idmlib::util::IDMIdConverter::GetId(ustr2, idmlib::util::IDMTermTag::CHN);
                    uint64_t key = idmlib::util::make64UInt(id1, id2);
                    invalidChnBigram_->insert(key, 0);
                }
            }
//             ifs.close();
            delete ifs;
        }
        
        int prefixTest(const std::vector<uint32_t>& termIdList)
        {
            uint32_t termCount = termIdList.size();
            if( termCount == 0 ) return KPStatus::NON_KP;
            if(termCount == 1)
            {
                uint32_t termId = termIdList[0];
                
                //check if single korean noun
                if( idmlib::util::IDMIdConverter::IsKP(termId) )
                {
                    return KPStatus::KP;
                }
                else if( isNonAppearTerm(termId) )
                {
                    return KPStatus::RETURN;
                }
            }
            if( termCount == 2 )
            {
                uint64_t key = idmlib::util::make64UInt(termIdList[0], termIdList[1]);
                if( invalidChnBigram_->find(key) != NULL )
                {
                    return KPStatus::NON_KP;
                }
            }
            if ( swContainer_->endWithStopWord( termIdList ) )
            {
                return KPStatus::RETURN;
            }
            int result = langScorer_->prefixTest(termIdList);
            if( result != KPStatus::CANDIDATE )
            {
                return result;
            }
            else
            {
                if( isNonAppearTerm(termIdList.back()) )
                {
                    return KPStatus::RETURN;
                }
                if( isMidAppearTerm(termIdList.front()) || isMidAppearTerm(termIdList.back())  )
                {
                    return KPStatus::NON_KP;
                }
            }
            return KPStatus::CANDIDATE;
        }
       
        void insertNonAppearTerm( uint32_t termId )
        {
            if( nonAppearTerms_ == NULL )
            {
                nonAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
            }
            nonAppearTerms_->insert(termId, 0);
        }
        
        bool isNonAppearTerm( uint32_t termId)
        {
            if( nonAppearTerms_ == NULL ) return false;
            return (nonAppearTerms_->find(termId)!=NULL);
        }
        
        void insertMidAppearTerm( uint32_t termId )
        {
            if( midAppearTerms_ == NULL )
            {
                midAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
            }
            midAppearTerms_->insert(termId, 0);
        }
        
        bool isMidAppearTerm( uint32_t termId)
        {
            if( midAppearTerms_ == NULL ) return false;
            return (midAppearTerms_->find(termId)!=NULL);
        }
        
        bool isSplitTerm(const izenelib::util::UString& ustr, char tag, uint32_t termId, uint32_t& insertTermId)
        {
            uint32_t indicateId = termId;
            insertTermId = termId;
            bool result = false;
//             std::cout<<"aaa"<<std::endl;
            if( tag == idmlib::util::IDMTermTag::NUM)
            {
                insertTermId = langScorer_->getArabicNumber();
                return true;
            }
            else if( tag == idmlib::util::IDMTermTag::ENG)
            {
                
                izenelib::util::UString lower = ustr;
                lower.toLowerString();
                indicateId = idmlib::util::IDMIdConverter::GetId(lower, tag);
                
                if( isNonAppearTerm(indicateId) )
                {
                    insertTermId = indicateId;
                    return true;
                }
                if( lower.length() == 1 )
                {
                    insertNonAppearTerm(indicateId);
                    insertTermId = indicateId;
                    return true;
                }
            }
            else if( tag == idmlib::util::IDMTermTag::CHN)
            {
                if( isNonAppearTerm(indicateId) )
                {
                    return true;
                }
            }
//             std::cout<<"bbb"<<std::endl;
            if( swContainer_->isStopWord( indicateId ) )
            {
                if( tag == idmlib::util::IDMTermTag::ENG)
                {
                    insertTermId = indicateId;
                }
                return true;
            }
//             std::cout<<"ccc"<<std::endl;
            if ( tag == idmlib::util::IDMTermTag::KOR && ustr.isDigitChar(0) )
            {
                insertNonAppearTerm(termId);
                result = true;
            }
            else if( (tag == idmlib::util::IDMTermTag::KOR||tag == idmlib::util::IDMTermTag::KOR_NOUN) && ustr.length()==1 )
            {
                insertNonAppearTerm(termId);
                result = true;
            }
            else if( tag == idmlib::util::IDMTermTag::KOR )
            {
                insertMidAppearTerm(termId);
            }
//             std::cout<<"ddd"<<std::endl;
            
            return result;
        }
        
        std::pair<bool, double> test(const SI& item,const SCI& citem)
        {
            return langScorer_->test(item, citem);
        }
        
        void flush()
        {
            if( nonAppearTerms_ != NULL )
            {
                delete nonAppearTerms_;
                nonAppearTerms_ = NULL;
            }
            if( midAppearTerms_ != NULL )
            {
                delete midAppearTerms_;
                midAppearTerms_ = NULL;
            }
        }
    
    private:
      idmlib::util::IDMAnalyzer* analyzer_;
      LanguageScorer* langScorer_;
      idmlib::util::StopWordContainer* swContainer_;
      izenelib::am::rde_hash<uint64_t, bool>* invalidChnBigram_;
      izenelib::am::rde_hash<uint32_t, bool>* nonAppearTerms_;//never occur in label
      izenelib::am::rde_hash<uint32_t, bool>* midAppearTerms_;//can occur in the middle of labels
        
};

NS_IDMLIB_KPE_END
#endif
