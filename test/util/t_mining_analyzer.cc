
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


void common_check(IDMAnalyzer* analyzer, const izenelib::util::UString& text, const std::vector<IDMTerm>& term_vec)
{
    std::vector<idmlib::util::IDMTerm> term_list;
    analyzer->GetTermList(text, term_list);
    
    for( uint32_t i=0;i<term_list.size();++i )
    {
        std::cout<<"["<<term_list[i].TextString()<<","<<term_list[i].position<<","<<term_list[i].tag<<"]"<<std::endl;
    }
    
    BOOST_CHECK( term_list.size() == term_vec.size() );
    
    for(uint32_t i=0;i<term_list.size();i++)
    {
        BOOST_CHECK( term_list[i].EqualsWithoutId(term_vec[i]) );
    }
}

void tg_check(IDMAnalyzer* analyzer, const izenelib::util::UString& text, const std::vector<IDMTerm>& term_vec)
{
    std::vector<idmlib::util::IDMTerm> term_list;
    analyzer->GetTgTermList(text, term_list);
    BOOST_CHECK( term_list.size() == term_vec.size() );
    
    for(uint32_t i=0;i<term_list.size();i++)
    {
        BOOST_CHECK( term_list[i].EqualsWithoutId(term_vec[i]) );
    }
}

// void common_check(IDMAnalyzer* analyzer, const izenelib::util::UString& text, const std::vector<izenelib::util::UString>& str_vec, const std::vector<uint32_t>& position_vec, const std::vector<char>& tag_vec)
// {
//     std::vector<idmlib::util::IDMTerm> term_list;
//     analyzer->GetTermList(text, term_list);
//     if(str_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == str_vec.size() );
//     }
//     if(position_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == position_vec.size() );
//     }
//     if(tag_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == tag_vec.size() );
//     }
//     for(uint32_t i=0;i<term_list.size();i++)
//     {
//         if( !str_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].text == str_vec[i] );
//         }
//         if( !position_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].position == position_vec[i] );
//         }
//         if( !tag_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].tag == tag_vec[i] );
//         }
//     }
// }
// 
// void tg_check(IDMAnalyzer* analyzer, const izenelib::util::UString& text, const std::vector<izenelib::util::UString>& str_vec, const std::vector<uint32_t>& position_vec, const std::vector<char>& tag_vec)
// {
//     std::vector<idmlib::util::IDMTerm> term_list;
//     analyzer->GetTgTermList(text, term_list);
//     if(str_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == str_vec.size() );
//     }
//     if(position_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == position_vec.size() );
//     }
//     if(tag_vec.size()>0)
//     {
//         BOOST_CHECK( term_list.size() == tag_vec.size() );
//     }
//     for(uint32_t i=0;i<term_list.size();i++)
//     {
//         if( !str_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].text == str_vec[i] );
//         }
//         if( !position_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].position == position_vec[i] );
//         }
//         if( !tag_vec.empty() )
//         {
//             BOOST_CHECK( term_list[i].tag == tag_vec[i] );
//         }
//     }
// }


BOOST_AUTO_TEST_SUITE(analyzer_test)

BOOST_AUTO_TEST_CASE(display_test)
{
//     {
//         IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",""));
//         izenelib::util::UString text(" Ｙｏｕ ａｒＥ　ａ　ｐｉｇ,， ａｒｅ　ｙｏｕ?２００１年| [[数理逻辑|數學邏輯]] || [[集合论|集合論]] || [[範疇論, 你呢, 呵呵,", izenelib::util::UString::UTF_8);
//             
//         la::TermList term_list;
//         analyzer.GetTermList(text, term_list);
//         
//         for( uint32_t i=0;i<term_list.size();++i )
//         {
//             std::cout<<"["<<term_list[i].textString()<<","<<term_list[i].wordOffset_<<","<<term_list[i].pos_<<"]"<<std::endl;
//         }
//     }
//     {
//         IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig("",IZENECMA_KNOWLEDGE,"")) ;
//         //   izenelib::util::UString text("国家经济 Sites year, 呵呵! United 사회부조리 애프터 전교조도", izenelib::util::UString::UTF_8);
//         izenelib::util::UString text("国家经 济 Sites year, 呵呵! United configuration", izenelib::util::UString::UTF_8);
//         
//         
//             
//         la::TermList term_list;
//         analyzer.GetTermList(text, term_list);
//         std::cout<<"CHINESE-TEST"<<std::endl;
//         for( uint32_t i=0;i<term_list.size();++i )
//         {
//             std::cout<<"["<<term_list[i].textString()<<","<<term_list[i].wordOffset_<<","<<term_list[i].pos_<<"]"<<std::endl;
//         }
//     }
    
    {
        IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonTgConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE)) ;
        izenelib::util::UString text("W ork T ime F un (亦称为 Hell's Part-Time Jobs 2000 、バイトヘル2000、打工地狱2000)是一款由 D3与 SCEI 为 PSP 平台开发的电子游戏。WTF于 2006年 12月22日 在日本发售；美国发售时间为 2006年 10月16日 。 [1] 该游戏包含超过40款小游戏，讲述你接受来自\"工作恶魔\"的兼职，你需要于根据等级而定的时间内完成指定数额的工作。工作包括如计算鸡隻(将初生的鸡隻分类为男性、女性或死亡)、劈木(同时避免劈到一些经常会放到劈木台上的可爱动物)、在工厂内替原子笔盖上盖子与空手道打斗。完成小游戏后你会得到金钱，可用在游戏的 转蛋 机内以随机的方式得到新的小游戏、你画廊内的奖品或一些甚至可用在现实生活的小活意，如一个时钟。该游戏英文名字的首字WTF，亦可解作常用语\"What the fuck?\"的首字母略缩字，一般用作表示惊讶或迷惑，因此与游戏的主题一致。这是巧合或是故意仍为未知数。游戏的美版于2006年9月17日泄漏到互联网上。&lt;ArticleID&gt;", izenelib::util::UString::UTF_8);
        std::vector<idmlib::util::IDMTerm> term_list;
        analyzer.GetTgTermList(text, term_list);
        for( uint32_t i=0;i<term_list.size();++i )
        {
            std::cout<<"["<<term_list[i].TextString()<<","<<term_list[i].position<<","<<term_list[i].tag<<"]"<<std::endl;
        }
    }
  
}

BOOST_AUTO_TEST_CASE(chineseandenglish_test)
{
  IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
  izenelib::util::UString text("国家经 济 Sites year, 呵呵! United configuration. Ｙｏｕ ａｒＥ L4", izenelib::util::UString::UTF_8);
  
  std::vector<IDMTerm> term_vec;
  term_vec.push_back( IDMTerm("国",0, 'C' ));
  term_vec.push_back( IDMTerm("家",1, 'C' ));
  term_vec.push_back( IDMTerm("经",2, 'C' ));
  term_vec.push_back( IDMTerm("济",3, 'C' ));
  term_vec.push_back( IDMTerm("Sites",4, 'F' ));
  term_vec.push_back( IDMTerm("year",5, 'F' ));
  term_vec.push_back( IDMTerm("呵",7, 'C' ));
  term_vec.push_back( IDMTerm("呵",8, 'C' ));
  term_vec.push_back( IDMTerm("United",10, 'F' ));
  term_vec.push_back( IDMTerm("configuration",11, 'F' ));
  term_vec.push_back( IDMTerm("You",13, 'F' ));
  term_vec.push_back( IDMTerm("arE",14, 'F' ));
  term_vec.push_back( IDMTerm("L",15, 'F' ));
  term_vec.push_back( IDMTerm("4",16, 'S' ));
  
  
    
  common_check(&analyzer, text, term_vec);
  
  IDMAnalyzer analyzer2(idmlib::util::IDMAnalyzerConfig::GetCommonTgConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
  
  term_vec.clear();
  term_vec.push_back( IDMTerm("国",0, 'C' ));
  term_vec.push_back( IDMTerm("家",1, 'C' ));
  term_vec.push_back( IDMTerm("经",2, 'C' ));
  term_vec.push_back( IDMTerm("济",3, 'C' ));
  term_vec.push_back( IDMTerm("Sites",4, 'F' ));
  term_vec.push_back( IDMTerm("year",5, 'F' ));
  term_vec.push_back( IDMTerm(",",6, '.' ));
  term_vec.push_back( IDMTerm("呵",7, 'C' ));
  term_vec.push_back( IDMTerm("呵",8, 'C' ));
  term_vec.push_back( IDMTerm("!",9, '.' ));
  term_vec.push_back( IDMTerm("United",10, 'F' ));
  term_vec.push_back( IDMTerm("configuration",11, 'F' ));
  term_vec.push_back( IDMTerm(".",12, '.' ));
  term_vec.push_back( IDMTerm("You",13, 'F' ));
  term_vec.push_back( IDMTerm("arE",14, 'F' ));
  term_vec.push_back( IDMTerm("L",15, 'F' ));
  term_vec.push_back( IDMTerm("4",16, 'S' ));
  
  tg_check(&analyzer2, text, term_vec);

}


BOOST_AUTO_TEST_CASE(cma_test)
{
  IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,IZENECMA_KNOWLEDGE,IZENEJMA_KNOWLEDGE));
  izenelib::util::UString text("新闻集团（英语：News corporation）是目前全球第三大的媒体集团。", izenelib::util::UString::UTF_8);
  
  std::vector<IDMTerm> term_vec;
  term_vec.push_back( IDMTerm("新闻",0, 'N' ));
  term_vec.push_back( IDMTerm("集团",1, 'N' ));
  term_vec.push_back( IDMTerm("英语",3, 'N' ));
  term_vec.push_back( IDMTerm("News",5, 'F' ));
  term_vec.push_back( IDMTerm("corporation",6, 'F' ));
  term_vec.push_back( IDMTerm("全球",10, 'N' ));
  term_vec.push_back( IDMTerm("媒体",14, 'N' ));
  term_vec.push_back( IDMTerm("集团",15, 'N' ));
  
    
  common_check(&analyzer, text, term_vec);
  
  
}


BOOST_AUTO_TEST_CASE(korean_test)
{
  izenelib::util::UString text("정형석기자 Korean", izenelib::util::UString::UTF_8);
  {
    IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
   
    std::vector<IDMTerm> term_vec;
    term_vec.push_back( IDMTerm("정형석기자",0, '?' ));
    term_vec.push_back( IDMTerm("정형석",0, 'N' ));
    term_vec.push_back( IDMTerm("기자",0, 'N' ));
    term_vec.push_back( IDMTerm("Korean",1, 'F' ));
    
    common_check(&analyzer, text, term_vec);
  }
  {
    IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonTgConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
    
    std::vector<IDMTerm> term_vec;
    term_vec.push_back( IDMTerm("정형석기자",0, 'P' ));
    term_vec.push_back( IDMTerm("Korean",1, 'F' ));
    tg_check(&analyzer, text, term_vec);
  }

}


BOOST_AUTO_TEST_CASE(ja_test)
{
  izenelib::util::UString text("イラク共和国（イラクきょうわこく）、通称イラクは、中東・西アジアの連邦共和制国家である。English char L4, 世界で3番目の原油埋蔵国である。", izenelib::util::UString::UTF_8);
  
  IDMAnalyzer analyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
//   izenelib::util::UString text("国家经济 Sites year, 呵呵! United 사회부조리 애프터 전교조도", izenelib::util::UString::UTF_8);
  std::vector<IDMTerm> term_vec;
  term_vec.push_back( IDMTerm("イラク", 0, 'N') );
  term_vec.push_back( IDMTerm("共和国",  1, 'N') );
  term_vec.push_back( IDMTerm("イラク",  3, 'N') );
  term_vec.push_back( IDMTerm("きょう",  4, 'N') );
  term_vec.push_back( IDMTerm("通称",  9, 'N') );
  term_vec.push_back( IDMTerm("イラク",  10, 'N') );
  term_vec.push_back( IDMTerm("中東",  13, 'N') );
  term_vec.push_back( IDMTerm("西",  15, 'N') );
  term_vec.push_back( IDMTerm("アジア",  16, 'N') );
  term_vec.push_back( IDMTerm("の",  17, 'L') );
  term_vec.push_back( IDMTerm("連邦",  18, 'N') );
  term_vec.push_back( IDMTerm("共和制",  19, 'N') );
  term_vec.push_back( IDMTerm("国家",  20, 'N') );
  term_vec.push_back( IDMTerm("English",  24, 'F') );
  term_vec.push_back( IDMTerm("char",  25, 'F') );
  term_vec.push_back( IDMTerm("L",  26, 'F') );
  term_vec.push_back( IDMTerm("4",  27, 'N') );
  term_vec.push_back( IDMTerm("世界",  29, 'N') );
  term_vec.push_back( IDMTerm("3",  31, 'N') );
  term_vec.push_back( IDMTerm("番目",  32, 'N') );
  term_vec.push_back( IDMTerm("の",  33, 'L') );
  term_vec.push_back( IDMTerm("原油",  34, 'N') );
  term_vec.push_back( IDMTerm("埋蔵国",  35, 'N') );
  
  common_check(&analyzer, text, term_vec);
  
  IDMAnalyzer analyzer2(idmlib::util::IDMAnalyzerConfig::GetCommonTgConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
  
  term_vec.clear();
  
  term_vec.push_back( IDMTerm("イラク共和国", 0, 'P') );
  term_vec.push_back( IDMTerm("（", 2, '.') );
  term_vec.push_back( IDMTerm("イラクきょう",  3, 'P') );
  term_vec.push_back( IDMTerm("）",  7, '.') );
  term_vec.push_back( IDMTerm("、",  8, '.') );
  term_vec.push_back( IDMTerm("通称イラク",  9, 'P') );
  term_vec.push_back( IDMTerm("、",  12, '.') );
  term_vec.push_back( IDMTerm("・",  14, '.') );
  
  term_vec.push_back( IDMTerm("西アジアの連邦共和制国家",  15, 'P') );
  term_vec.push_back( IDMTerm("。",  23, '.') );
  term_vec.push_back( IDMTerm("English",  24, 'F') );
  term_vec.push_back( IDMTerm("char",  25, 'F') );
  term_vec.push_back( IDMTerm("L",  26, 'F') );
  term_vec.push_back( IDMTerm("4",  27, 'N') );
  term_vec.push_back( IDMTerm(",",  28, '.') );
  term_vec.push_back( IDMTerm("3番目の原油埋蔵国",  31, 'P') );
  term_vec.push_back( IDMTerm("。",  38, '.') );
  
  tg_check(&analyzer2, text, term_vec);
  
//   std::vector<idmlib::util::IDMTerm> term_list;
//   
//   std::cout<<"[After Compound]"<<std::endl;
//   analyzer2.GetTgTermList(text, term_list);
//   
//   for( uint32_t i=0;i<term_list.size();++i )
//   {
//     std::cout<<"["<<term_list[i].TextString()<<","<<term_list[i].position<<","<<term_list[i].tag<<"]"<<std::endl;
//   }

}

BOOST_AUTO_TEST_SUITE_END() 
