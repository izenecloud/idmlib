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
  
  void GetTgTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
  {
    typedef idmlib::util::IDMTerm TgTerm;
    std::vector<idmlib::util::IDMTerm> raw_term_list;
    GetTermList(text, raw_term_list);
    uint32_t size = raw_term_list.size();
    uint32_t p=0;
    std::vector<idmlib::util::IDMTerm> tmp_term_list;
    uint32_t last_position = 0;

    while (true)
    {
      bool bCheckTmpTermList = false;

      uint32_t position = 0;
      if(p == size) bCheckTmpTermList = true;
      else
      {
        position = raw_term_list[p].position;
        if(last_position != position) bCheckTmpTermList = true;
      }
      if( bCheckTmpTermList )
      {
        if( tmp_term_list.size() > 0 )
        {
          //TODO
          if(tmp_term_list.size()==1)//
          {
            uint32_t termId = 0;
            if(tmp_term_list[0].tag== idmlib::util::IDMTermTag::KOR_NOUN && tmp_term_list[0].text.length()>=4)
            {
              tmp_term_list[0].tag = idmlib::util::IDMTermTag::KOR_COMP_NOUN;
            }
            else
            {
              if( tmp_term_list[0].text.isKoreanChar(0) && tmp_term_list[0].tag!=idmlib::util::IDMTermTag::KOR_NOUN
                  && tmp_term_list[0].tag!=idmlib::util::IDMTermTag::KOR)
              {
                  tmp_term_list[0].tag = idmlib::util::IDMTermTag::KOR;
              }
            }
            
            termId = IDMIdConverter::GetId( tmp_term_list[0].text, tmp_term_list[0].tag );
            TgTerm term(tmp_term_list[0].text, termId, tmp_term_list[0].tag, tmp_term_list[0].position);
            term_list.push_back(term);

          }
          else
          {
            if(tmp_term_list[0].tag == idmlib::util::IDMTermTag::KOR)
            {
              izenelib::util::UString combination;
              for(uint32_t p=1;p<tmp_term_list.size();p++)
              {
                if( tmp_term_list[p].tag == idmlib::util::IDMTermTag::KOR_NOUN )
                {
                  combination.append(tmp_term_list[p].text);
                }
                else
                {
                  break;
                }
              }
              if( combination.length() >= tmp_term_list[0].text.length()-1 && combination.length() >= 3 )
              {

                char tag = idmlib::util::IDMTermTag::KOR_NOUN;
                if(combination.length()>=4)
                {
                  tag = idmlib::util::IDMTermTag::KOR_COMP_NOUN;
                }
                
                uint32_t termId = IDMIdConverter::GetId( combination, tag );
                TgTerm term(combination, termId, tag, tmp_term_list[0].position);
                term_list.push_back(term);
              }
              else
              {
                uint32_t termId = IDMIdConverter::GetId( tmp_term_list[0].text, tmp_term_list[0].tag );
                TgTerm term(tmp_term_list[0].text, termId, tmp_term_list[0].tag, tmp_term_list[0].position);
                term_list.push_back(term);

              }
            }
          }
          tmp_term_list.clear();
        }
      }
      if(p == size) break;

      if (raw_term_list[p].text.length() == 0)
      {
          p++;
          last_position = position;
          continue;
      }
      char tag = raw_term_list[p].tag;
      if (tag == idmlib::util::IDMTermTag::ENG || tag == idmlib::util::IDMTermTag::CHN)//english or chinese
      {
        uint32_t termId = IDMIdConverter::GetId( raw_term_list[p].text, tag );
        TgTerm term(raw_term_list[p].text, termId, tag, position);
        term_list.push_back(term);
      }
      else
      {
        TgTerm term(raw_term_list[p].text, 0, tag, position);
        tmp_term_list.push_back(term);
      }
      last_position = position;
      ++p;
    }
  }
   
 private:
  la::LA* la_;
        
        
};

NS_IDMLIB_UTIL_END

#endif
