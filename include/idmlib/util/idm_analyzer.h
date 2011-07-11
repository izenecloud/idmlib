///
/// @file idm_analyzer.h
/// @brief To provide the LA functions for mining tasks.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-08-26
/// @date 
///

#ifndef IDM_UTIL_IDMANALYZER_H_
#define IDM_UTIL_IDMANALYZER_H_


#include <la/LA.h>
#include <la/stem/Stemmer.h>
#include <la/util/UStringUtil.h>
#include <idmlib/idm_types.h>
#include <idmlib/util/resource_util.h>
#include <ir/index_manager/index/LAInput.h>
#include <boost/algorithm/string/trim.hpp>
#include "idm_analyzer_config.h"
#include "idm_id_converter.h"
#include "idm_term.h"
#include "Util.hpp"
NS_IDMLIB_UTIL_BEGIN



class IDMAnalyzer
{
 public:
     
  IDMAnalyzer();   
  
  IDMAnalyzer(const IDMAnalyzerConfig& config);
   
   
  ~IDMAnalyzer();
  
  void ExtractSpecialChar(bool extractSpecialChar, bool convertToPlaceHolder);
  
  void ExtractSymbols();
  
  bool LoadSimplerFile(const std::string& file);
  
  bool LoadT2SMapFile(const std::string& file);
  
  template<typename IDManagerType>
  void GetTermIdList(IDManagerType* idm, const izenelib::util::UString& inputString, TermIdList& outList)
  {
	  boost::mutex::scoped_lock lock(la_mtx_);
	  la_->process( idm, inputString, outList );
  }

  void GetTermList(const izenelib::util::UString& utext, la::TermList& term_list, bool convert = true);
  
  void GetTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list);
  
  void GetStemTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list);
  
  void GetTermList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list, std::vector<uint32_t>& id_list);
  
  void GetStringList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list);
  
  void GetFilteredStringList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list);
  
  void GetIdList(const izenelib::util::UString& text, std::vector<uint32_t>& id_list);
  
  void GetIdListForMatch(const izenelib::util::UString& text, std::vector<uint32_t>& id_list);
  
  void GetTgTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list);
  
 private:
     
  void InitWithConfig_(const IDMAnalyzerConfig& config);
  void simple_(izenelib::util::UString& content);
  void JKCompound_(const std::vector<idmlib::util::IDMTerm>& raw_term_list, std::vector<idmlib::util::IDMTerm>& term_list);
  void CompoundInSamePosition_(const std::vector<idmlib::util::IDMTerm>& terms_in_same_position, idmlib::util::IDMTerm& compound_kor);
  void CompoundInContinous_(std::vector<idmlib::util::IDMTerm>& terms_in_continous);
  bool TermIgnore_(la::Term& term);
  bool TermIgnoreInTg_(const idmlib::util::IDMTerm& term);
   
 private:
  IDMAnalyzerConfig config_;
  la::LA* la_;
  la::stem::Stemmer* stemmer_;
  bool simpler_set_;
  izenelib::am::rde_hash<izenelib::util::UCS2Char, izenelib::util::UCS2Char> simpler_map_;
  boost::mutex la_mtx_;
        
        
};

NS_IDMLIB_UTIL_END

#endif
