///
/// @file   tdt_scorer_types.h
/// @brief  Some type definitions for scorer class in TDT
/// @author Jia Guo
/// @date   2011-04-25
/// @date   2011-04-25
///
#ifndef IDM_TDTSCORERTYPES_H_
#define IDM_TDTSCORERTYPES_H_
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
#include <boost/function.hpp>
#include "../util/StopWordContainer.hpp"
#include "../util/Util.hpp"
#include "../util/idm_id_manager.h"
#include "../idm_types.h"
#include "tdt_types.h"

NS_IDMLIB_TDT_BEGIN

class ScorerContextItem
{
    public:
        ScorerContextItem():term_list()
        {
        }
        
        ScorerContextItem(const std::vector<std::pair<TermInNgram, uint32_t> >& iterm_list)
        :term_list(iterm_list)
        {
        }
        
        ScorerContextItem(const std::vector<std::pair<TermInNgram, uint32_t> >& iterm_list, 
        const std::vector<double>& weightList):
        term_list(iterm_list), weight_list(weightList)
        {
        }

        std::string ToString(idmlib::util::IDMIdManager* id_manager) const
        {
          std::string result = "{SCI}";
          for(uint32_t i=0;i<term_list.size();i++)
          {
              izenelib::util::UString ustr;
              bool b = id_manager->GetStringById(term_list[i].first.id, ustr);
              if(b)
              {
                  std::string str;
                  ustr.convertString(str, izenelib::util::UString::UTF_8);
                  result += str+":"+boost::lexical_cast<std::string>(term_list[i].second)+":"+term_list[i].first.tag+"|";
              }
              else
              {
                  result += boost::lexical_cast<std::string>(term_list[i].first.id)+":"+boost::lexical_cast<std::string>(term_list[i].second)+":"+term_list[i].first.tag+"|";
              }
          }
          return result;
        }
        

        
        
    public:
        std::vector<std::pair<TermInNgram, uint32_t> > term_list;
        std::vector<double> weight_list;

};
typedef ScorerContextItem SCI;
class ScorerItem
{
    public:
        ScorerItem():term_list(), freq(0)
        {
        }
        
        ScorerItem(const std::vector<TermInNgram>& termList, 
        uint32_t f):
        term_list(termList), freq(f)
        {
        }
        
        std::string ToString(idmlib::util::IDMIdManager* id_manager) const
        {
          std::string result = "{SI}["+boost::lexical_cast<std::string>(freq)+"]|";
          for(uint32_t i=0;i<term_list.size();i++)
          {
              izenelib::util::UString ustr;
              bool b = id_manager->GetStringById(term_list[i].id, ustr);
              if(b)
              {
                  std::string str;
                  ustr.convertString(str, izenelib::util::UString::UTF_8);
                  result += str+",";
              }
              else
              {
                  result += boost::lexical_cast<std::string>(term_list[i].id)+",";
              }
          }
          return result;
        }
        
//         template <class IDManager>
//         std::string getString(IDManager* idManager, const std::string& split=" ") const
//         {
//             std::string result = "";
//             if( termIdList_.size() > 0 )
//             {
//                 for(uint32_t i=0;i<termIdList_.size()-1;i++)
//                 {
//                     izenelib::util::UString ustr;
//                     bool b = idManager->getTermStringByTermId(termIdList_[i], ustr);
//                     if(b)
//                     {
//                         std::string str;
//                         ustr.convertString(str, izenelib::util::UString::UTF_8);
//                         result += str+split;
//                     }
//                     else
//                     {
//                         result += boost::lexical_cast<std::string>(termIdList_[i])+split;
//                     }
//                 }
//                 izenelib::util::UString ustr;
//                 bool b = idManager->getTermStringByTermId(termIdList_.back(), ustr);
//                 if(b)
//                 {
//                     std::string str;
//                     ustr.convertString(str, izenelib::util::UString::UTF_8);
//                     result += str;
//                 }
//                 else
//                 {
//                     result += boost::lexical_cast<std::string>(termIdList_.back());
//                 }
//             }
//             return result;
//         }
        
    public:
        std::vector<TermInNgram> term_list;
        uint32_t freq;
        
};
typedef ScorerItem SI;

class KPStatus
{
    public:
        static const int RETURN = 1;
        static const int NON_KP = 2;
        static const int CANDIDATE = 3;
        static const int KP = 4;
};



NS_IDMLIB_TDT_END
#endif
