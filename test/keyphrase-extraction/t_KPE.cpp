#include <boost/test/unit_test.hpp>

#include <idmlib/keyphrase-extraction/fwd.h>
#include "kpe_types.h"
#include "../test_common.h"
#include <string>
#include <iostream>
#include <vector>

using namespace idmlib::kpe;


BOOST_AUTO_TEST_SUITE(KPE_test)


void test_func(const izenelib::util::UString& ustr
, const std::vector<id2count_t>& id2countList
, uint8_t score
, const std::vector<uint32_t>& leftTermIdList
, const std::vector<uint32_t>& leftTermCountList
, const std::vector<uint32_t>& rightTermIdList
, const std::vector<uint32_t>& rightTermCountList)
{
    std::string str;
    ustr.convertString(str, izenelib::util::UString::UTF_8);
    std::cout<<"Find KP: "<<str<<std::endl;
}

BOOST_AUTO_TEST_CASE(normal_test)
{
//     boost::filesystem::remove_all("./kpetestid");
//     boost::filesystem::remove_all("./kmp");
//     std::string res_path;
//     BOOST_CHECK( ResourceHelper::getKpePath(res_path) == true );
//     typedef TestIDManager IDManagerType;
//     IDManagerType idManager("./kpetestid");
//     KPE_ALL<IDManagerType>::function_type func = &test_func;
//     KPE_ALL<IDManagerType> kpe( &idManager, func, "./kmp");
//     kpe.load(res_path);
//     izenelib::util::UString article( "中国阿，我在中国玩，你呢?我觉得中国不错。我下次也去中国", izenelib::util::UString::UTF_8);
//     kpe.insert(article, 1);
//     kpe.close();
//     boost::filesystem::remove_all("./kpetestid");
//     boost::filesystem::remove_all("./kmp");
}




BOOST_AUTO_TEST_SUITE_END() 
