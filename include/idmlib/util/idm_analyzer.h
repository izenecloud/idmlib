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
#include <idmlib/idm_types.h>
#include "idm_id_converter.h"
#include "idm_term.h"
NS_IDMLIB_UTIL_BEGIN



class IDMAnalyzer
{
 public:
  IDMAnalyzer(const std::string& kma_resource_path)
  :la_(new la::LA() )
  {
    boost::shared_ptr<la::MultiLanguageAnalyzer> ml_analyzer(new la::MultiLanguageAnalyzer() );
    ml_analyzer->setExtractSpecialChar(false, false);
    boost::shared_ptr<la::Analyzer> korean_analyzer(new la::KoreanAnalyzer( kma_resource_path ) );
    la::KoreanAnalyzer* p_korean_analyzer = static_cast<la::KoreanAnalyzer*>(korean_analyzer.get());
    p_korean_analyzer->setLabelMode();
//     p_korean_analyzer->setNBest(1);
    p_korean_analyzer->setExtractEngStem( false );
    p_korean_analyzer->setExtractSynonym(false);
    p_korean_analyzer->setCaseSensitive(true, false);
    ml_analyzer->setDefaultAnalyzer( korean_analyzer );
    boost::shared_ptr<la::Analyzer> char_analyzer(new la::CharAnalyzer() );
    ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::CHINESE, char_analyzer );
    la_->setAnalyzer( ml_analyzer );
  }
  
  ~IDMAnalyzer()
  {
    delete la_;
  }
  
  void GetTermList(const izenelib::util::UString& text, la::TermList& term_list)
  {
    la_->process( text, term_list );
  }
  
  void GetTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
  {
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    term_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
      term_list[i].text = it->text_;
      term_list[i].tag = IDMTermTag::GetTermTag( it->pos_ );
      term_list[i].id = IDMIdConverter::GetId( it->text_, term_list[i].tag );
      term_list[i].position = it->wordOffset_;
      i++;
      it++;
    }
  }
  
  void GetIdList(const izenelib::util::UString& text, std::vector<uint32_t>& id_list)
  {
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    id_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
      id_list[i] = IDMIdConverter::GetId( it->text_, it->pos_ );
      i++;
      it++;
    }
  }
   
 private:
  la::LA* la_;
        
        
};

NS_IDMLIB_UTIL_END

#endif
