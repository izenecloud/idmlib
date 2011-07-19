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


BOOST_AUTO_TEST_CASE(seg_test )
{
    FuzzyPinyinSegmentor seg;
    seg.LoadPinyinFile("./pinyin.txt");
    display_raw(&seg, "nanaodeye");
    display_raw(&seg, "mangzuoni");
    display_raw(&seg, "woyaochitang");
    display_raw(&seg, "woyaochitan");
    display_raw(&seg, "congxin");
    
    
}



BOOST_AUTO_TEST_SUITE_END()
