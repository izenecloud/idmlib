
#include <boost/test/unit_test.hpp>
#include <boost/date_time.hpp>

#include <idmlib/util/idm_analyzer.h>
#include "../TestResources.h"

#include <sdb/SequentialDB.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

using namespace idmlib::util;





BOOST_AUTO_TEST_SUITE(analyzer_test)

BOOST_AUTO_TEST_CASE(display_test)
{
  IDMAnalyzer analyzer;
  analyzer.ExtractSpecialChar(true, false);
//   izenelib::util::UString text("国家经济 Sites year, 呵呵! United 사회부조리 애프터 전교조도", izenelib::util::UString::UTF_8);
  izenelib::util::UString text("| [[数理逻辑|數學邏輯]] || [[集合论|集合論]] || [[範疇論, 你呢, 呵呵,", izenelib::util::UString::UTF_8);
    
  la::TermList term_list;
  analyzer.GetTermList(text, term_list);
  
  for( uint32_t i=0;i<term_list.size();++i )
  {
    std::cout<<"["<<term_list[i].textString()<<","<<term_list[i].wordOffset_<<","<<term_list[i].pos_<<"]"<<std::endl;
  }
  
}

BOOST_AUTO_TEST_CASE(chineseandenglish_test)
{
  IDMAnalyzer analyzer;
//   izenelib::util::UString text("国家经济 Sites year, 呵呵! United 사회부조리 애프터 전교조도", izenelib::util::UString::UTF_8);
  izenelib::util::UString text("国家经 济 Sites year, 呵呵! United configuration", izenelib::util::UString::UTF_8);
  uint32_t term_count = 10;
  
  std::vector<izenelib::util::UString> str_vec(term_count);
  str_vec[0] = izenelib::util::UString("国", izenelib::util::UString::UTF_8);
  str_vec[1] = izenelib::util::UString("家", izenelib::util::UString::UTF_8);
  str_vec[2] = izenelib::util::UString("经", izenelib::util::UString::UTF_8);
  str_vec[3] = izenelib::util::UString("济", izenelib::util::UString::UTF_8);
  str_vec[4] = izenelib::util::UString("Sites", izenelib::util::UString::UTF_8);
  str_vec[5] = izenelib::util::UString("year", izenelib::util::UString::UTF_8);
  str_vec[6] = izenelib::util::UString("呵", izenelib::util::UString::UTF_8);
  str_vec[7] = izenelib::util::UString("呵", izenelib::util::UString::UTF_8);
  str_vec[8] = izenelib::util::UString("United", izenelib::util::UString::UTF_8);
  str_vec[9] = izenelib::util::UString("configuration", izenelib::util::UString::UTF_8);
  
  std::vector<uint32_t> position_vec(term_count);
  position_vec[0] = 0;
  position_vec[1] = 1;
  position_vec[2] = 2;
  position_vec[3] = 3;
  position_vec[4] = 4;
  position_vec[5] = 5;
  position_vec[6] = 7;
  position_vec[7] = 8;
  position_vec[8] = 10;
  position_vec[9] = 11;
  
  std::vector<std::string> tag_vec(term_count);
  tag_vec[0] = "C";
  tag_vec[1] = "C";
  tag_vec[2] = "C";
  tag_vec[3] = "C";
  tag_vec[4] = "F";
  tag_vec[5] = "F";
  tag_vec[6] = "C";
  tag_vec[7] = "C";
  tag_vec[8] = "F";
  tag_vec[9] = "F";
  
  la::TermList term_list;
  analyzer.GetTermList(text, term_list);
  
  for( uint32_t i=0;i<term_list.size();++i )
  {
    std::cout<<"["<<term_list[i].textString()<<","<<term_list[i].wordOffset_<<","<<term_list[i].pos_<<"]"<<std::endl;
  }
  
  BOOST_CHECK( term_list.size() == term_count );
  
  for( uint32_t i=0;i<term_list.size();++i )
  {
    BOOST_CHECK( term_list[i].text_ == str_vec[i] );
    BOOST_CHECK( term_list[i].wordOffset_ == position_vec[i] );
    BOOST_CHECK( term_list[i].pos_ == tag_vec[i] );
  }

}

BOOST_AUTO_TEST_CASE(korean_test)
{
  IDMAnalyzer analyzer(WISEKMA_KNOWLEDGE);
//   izenelib::util::UString text("国家经济 Sites year, 呵呵! United 사회부조리 애프터 전교조도", izenelib::util::UString::UTF_8);
  izenelib::util::UString text("정형석기자", izenelib::util::UString::UTF_8);
  uint32_t term_count = 3;
  
  std::vector<izenelib::util::UString> str_vec(term_count);
  str_vec[0] = izenelib::util::UString("정형석기자", izenelib::util::UString::UTF_8);
  str_vec[1] = izenelib::util::UString("정형석", izenelib::util::UString::UTF_8);
  str_vec[2] = izenelib::util::UString("기자", izenelib::util::UString::UTF_8);
//   str_vec[3] = izenelib::util::UString("济", izenelib::util::UString::UTF_8);
//   str_vec[4] = izenelib::util::UString("Sites", izenelib::util::UString::UTF_8);
//   str_vec[5] = izenelib::util::UString("year", izenelib::util::UString::UTF_8);
//   str_vec[6] = izenelib::util::UString("呵", izenelib::util::UString::UTF_8);
//   str_vec[7] = izenelib::util::UString("呵", izenelib::util::UString::UTF_8);
//   str_vec[8] = izenelib::util::UString("United", izenelib::util::UString::UTF_8);
  
  std::vector<uint32_t> position_vec(term_count);
  position_vec[0] = 0;
  position_vec[1] = 0;
  position_vec[2] = 0;
//   position_vec[3] = 3;
//   position_vec[4] = 4;
//   position_vec[5] = 5;
//   position_vec[6] = 7;
//   position_vec[7] = 8;
//   position_vec[8] = 10;
  
  std::vector<std::string> tag_vec(term_count);
  tag_vec[0] = "?";
  tag_vec[1] = "NNI";
  tag_vec[2] = "NNG";
//   tag_vec[3] = "C";
//   tag_vec[4] = "F";
//   tag_vec[5] = "F";
//   tag_vec[6] = "C";
//   tag_vec[7] = "C";
//   tag_vec[8] = "F";
  
  la::TermList term_list;
  analyzer.GetTermList(text, term_list);
  
  for( uint32_t i=0;i<term_list.size();++i )
  {
    std::cout<<"["<<term_list[i].textString()<<","<<term_list[i].wordOffset_<<","<<term_list[i].pos_<<"]"<<std::endl;
  }
  
  BOOST_CHECK( term_list.size() == term_count );
  
  for( uint32_t i=0;i<term_list.size();++i )
  {
    BOOST_CHECK( term_list[i].text_ == str_vec[i] );
    BOOST_CHECK( term_list[i].wordOffset_ == position_vec[i] );
    BOOST_CHECK( term_list[i].pos_ == tag_vec[i] );
  }

}




BOOST_AUTO_TEST_SUITE_END() 
