///
/// @file idm_analyzer_config.h
/// @brief Config for idm_analyzer
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-07-01
/// @date
///

#ifndef IDM_UTIL_IDMANALYZER_CONFIG_H_
#define IDM_UTIL_IDMANALYZER_H_


#include <la/LA.h>
#include <la/stem/Stemmer.h>
#include <la/util/UStringUtil.h>
#include <idmlib/idm_types.h>
#include <idmlib/util/resource_util.h>
#include <ir/index_manager/index/LAInput.h>
#include <boost/algorithm/string/trim.hpp>
#include "idm_id_converter.h"
#include "idm_term.h"
#include "Util.hpp"
NS_IDMLIB_UTIL_BEGIN

class EMAConfig
{
    public:
        bool enable;
        bool case_sensitive;
};


class KMAConfig
{
    public:
        std::string path;
};

class CMAConfig
{
    public:
        std::string path;
        bool use_char;
        la::ChineseAnalyzer::ChineseAnalysisType type;
        bool remove_stopwords;
        bool noun_only;
        bool merge_alpha_digit;
};

class JMAConfig
{
    public:
        std::string path;
        bool noun_only;
};

class IDMAnalyzerConfig
{
 public:
  enum LANGUAGE{
      ENGLISH = 1,
      KOREAN = 2,
      CHINESE = 3,
      JAPANESE = 4
  } ;

  IDMAnalyzerConfig():
  default_language(CHINESE)
  {
  }

  static IDMAnalyzerConfig GetCommonConfig(const std::string& kma_path, const std::string& cma_path, const std::string& jma_path)
  {
      IDMAnalyzerConfig config;
      config.symbol = false;
      config.ema_config.enable = true;
      config.ema_config.case_sensitive = true;
      config.kma_config.path = kma_path;
      config.cma_config.path = cma_path;
      config.cma_config.use_char = false;
      config.cma_config.type = la::ChineseAnalyzer::maximum_match;
      config.cma_config.remove_stopwords = false;
      config.cma_config.noun_only = false;
      config.cma_config.merge_alpha_digit = false;
      if( config.cma_config.path=="" )
      {
          config.cma_config.use_char = true;
      }
      config.jma_config.path = jma_path;
      config.jma_config.noun_only = true;
      if( config.kma_config.path!="" )
      {
          config.default_language = KOREAN;
      }
      else
      {
          if(config.cma_config.path!="" )
          {
              config.default_language = CHINESE;
          }
          else if(config.jma_config.path!="" )
          {
              config.default_language = JAPANESE;
          }
          else
          {
              //char analyzer
              config.default_language = CHINESE;
          }
      }
      return config;
  }

  static IDMAnalyzerConfig GetCommonTgConfig(const std::string& kma_path, const std::string& cma_path, const std::string& jma_path)
  {
      IDMAnalyzerConfig config;
      config.symbol = true;
      config.ema_config.enable = true;
      config.ema_config.case_sensitive = true;
      config.kma_config.path = kma_path;
      config.cma_config.path = cma_path;
      config.cma_config.use_char = false;
      config.cma_config.type = la::ChineseAnalyzer::maximum_match;
      config.cma_config.remove_stopwords = false;
      config.cma_config.noun_only = false;
      config.cma_config.merge_alpha_digit = false;
      if( config.cma_config.path=="" )
      {
          config.cma_config.use_char = true;
      }
      config.jma_config.path = jma_path;
      config.jma_config.noun_only = true;
      if( config.kma_config.path!="" )
      {
          config.default_language = KOREAN;
      }
      else
      {
          if(config.cma_config.path!="" )
          {
              config.default_language = CHINESE;
          }
          else if(config.jma_config.path!="" )
          {
              config.default_language = JAPANESE;
          }
          else
          {
              //char analyzer
              config.default_language = CHINESE;
          }
      }
      return config;
  }

  EMAConfig ema_config;
  KMAConfig kma_config;
  CMAConfig cma_config;
  JMAConfig jma_config;
  LANGUAGE default_language;
  bool symbol;
};

NS_IDMLIB_UTIL_END

#endif
