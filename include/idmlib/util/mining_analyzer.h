///
/// @file mining_analyzer.h
/// @brief To provide the LA functions for mining tasks.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-08-26
/// @date 
///

#ifndef IDM_UTIL_MININGANALYZER_HPP_
#define IDM_UTIL_MININGANALYZER_HPP_


#include <la/LA.h>
#include <idmlib/idm_types.h>

NS_IDMLIB_UTIL_BEGIN



class MiningAnalyzer
{
 public:
  MiningAnalyzer(const std::string& kma_resource_path)
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
  
  ~MiningAnalyzer()
  {
    delete la_;
  }
  
  void get_term_list(const izenelib::util::UString& text, la::TermList& term_list)
  {
    la_->process( text, term_list );
  }
   
 private:
  la::LA* la_;
        
        
};

NS_IDMLIB_UTIL_END

#endif
