#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/query-correction/fuzzy_pinyin_segmentor.h>

BOOST_AUTO_TEST_SUITE( t_py_seg )

using namespace idmlib::qc;

void display_raw( FuzzyPinyinSegmentor* seg, const std::string& input)
{
    std::vector<std::string> vec;
    seg->FuzzySegmentRaw(input, vec);
    std::cout<<"input "<<input<<std::endl;
    for(uint32_t i=0;i<vec.size();i++)
    {
        std::cout<<"result: "<<vec[i]<<std::endl;
    }
}

void display_convert( FuzzyPinyinSegmentor* seg, const std::string& input)
{
    std::vector<std::string> vec;
    izenelib::util::UString ustr(input, izenelib::util::UString::UTF_8);
    seg->GetPinyin(ustr, vec);
    std::cout<<"input "<<input<<std::endl;
    for(uint32_t i=0;i<vec.size();i++)
    {
        std::cout<<"result: "<<vec[i]<<std::endl;
    }
}

void display_convert_py( FuzzyPinyinSegmentor* seg, const std::string& input)
{
    std::vector<izenelib::util::UCS2Char> result_list;
    seg->GetChar(input, result_list);
    
    std::cout<<"input "<<input<<std::endl;
    for(uint32_t i=0;i<result_list.size();i++)
    {
        izenelib::util::UString ustr;
        ustr+=result_list[i];
        std::string str;
        ustr.convertString(str, izenelib::util::UString::UTF_8);
        std::cout<<"result: "<<str<<std::endl;
    }
}

BOOST_AUTO_TEST_CASE(seg_test )
{
    FuzzyPinyinSegmentor seg;
    seg.LoadPinyinFile("./cn_qc_res/pinyin.txt");
    display_raw(&seg, "nanaodeye");
    display_raw(&seg, "mangzuoni");
    display_raw(&seg, "woyaochitang");
    display_raw(&seg, "woyaochitan");
    display_raw(&seg, "congxin");
    
    
}

BOOST_AUTO_TEST_CASE(cn_conv_py_test )
{
    FuzzyPinyinSegmentor seg;
    seg.LoadPinyinFile("./cn_qc_res/pinyin.txt");
    display_convert(&seg, "我们的爱");
    display_convert(&seg, "你再哪儿");
    display_convert(&seg, "中过");
    display_convert(&seg, "你哦");
    display_convert(&seg, "哈哈");
    
    
}

BOOST_AUTO_TEST_CASE(py_conv_cn_test )
{
    FuzzyPinyinSegmentor seg;
    seg.LoadPinyinFile("./cn_qc_res/pinyin.txt");
    display_convert_py(&seg, "zhen");
    display_convert_py(&seg, "zi");
    
    
    
}



BOOST_AUTO_TEST_SUITE_END()
