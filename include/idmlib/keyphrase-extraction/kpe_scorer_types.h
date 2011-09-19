///
/// @file   kpe_scorer_types.h
/// @brief  Some type definitions for scorer class in KPE
/// @author Jia Guo
/// @date   2010-11-04
/// @date   2010-11-04
///
#ifndef IDM_KPESCORERTYPES_H_
#define IDM_KPESCORERTYPES_H_
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

NS_IDMLIB_KPE_BEGIN

class ScorerContextItem
{
    public:
        ScorerContextItem():term_list()
        {
        }
        
        ScorerContextItem(const std::vector<std::pair<uint32_t, uint32_t> >& iterm_list)
        :term_list(iterm_list)
        {
        }
        
        ScorerContextItem(const std::vector<std::pair<uint32_t, uint32_t> >& iterm_list, 
        const std::vector<double>& weightList):
        term_list(iterm_list), weight_list(weightList)
        {
        }

//         std::string ToString(idmlib::util::IDMIdManager* id_manager) const
//         {
//           std::string result = "{SCI}";
//           for(uint32_t i=0;i<termid_list.size();i++)
//           {
//               izenelib::util::UString ustr;
//               bool b = id_manager->GetStringById(termid_list[i], ustr);
//               if(b)
//               {
//                   std::string str;
//                   ustr.convertString(str, izenelib::util::UString::UTF_8);
//                   result += str+":"+boost::lexical_cast<std::string>(count_list[i])+"|";
//               }
//               else
//               {
//                   result += boost::lexical_cast<std::string>(termid_list[i])+":"+boost::lexical_cast<std::string>(count_list[i])+"|";
//               }
//           }
//           return result;
//         }
        

        
        
    public:
        std::vector<std::pair<uint32_t, uint32_t> > term_list;
        std::vector<double> weight_list;

};
typedef ScorerContextItem SCI;
class ScorerItem
{
    public:
        ScorerItem():termid_list(), freq(0)
        {
        }
        
        ScorerItem(const std::vector<uint32_t>& termIdList, 
        uint32_t f):
        termid_list(termIdList), freq(f)
        {
        }
        
        std::string ToString(idmlib::util::IDMIdManager* id_manager) const
        {
          std::string result = "{SI}["+boost::lexical_cast<std::string>(freq)+"]|";
          for(uint32_t i=0;i<termid_list.size();i++)
          {
              izenelib::util::UString ustr;
              bool b = id_manager->GetStringById(termid_list[i], ustr);
              if(b)
              {
                  std::string str;
                  ustr.convertString(str, izenelib::util::UString::UTF_8);
                  result += str+",";
              }
              else
              {
                  result += boost::lexical_cast<std::string>(termid_list[i])+",";
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
        std::vector<uint32_t> termid_list;
        uint32_t freq;
        
};
typedef ScorerItem SI;





NS_IDMLIB_KPE_END
#endif
