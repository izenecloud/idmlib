#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/query-correction/cn_query_correction.h>
#include <util/ClockTimer.h>

BOOST_AUTO_TEST_SUITE( t_cn_correction )

using namespace idmlib::qc;

void display( CnQueryCorrection* cqc, const std::string& input)
{
    std::cout << "[input]" << input << std::endl;
    izenelib::util::UString text(input, izenelib::util::UString::UTF_8);
    std::vector<izenelib::util::UString> output;
    izenelib::util::ClockTimer clock;
    cqc->GetResult(text, output);
    std::cout << "cost " << clock.elapsed() << " seconds" << std::endl;
    for (uint32_t i = 0; i < output.size(); i++)
    {
        std::string str;
        output[i].convertString(str, izenelib::util::UString::UTF_8);
        std::cout << "[Correct Result] " << str << std::endl;
    }
}

BOOST_AUTO_TEST_CASE(cqc_test )
{
    CnQueryCorrection cqc("./cn_qc_res");
    cqc.Load();
    display(&cqc, "woyaochitang");
    display(&cqc, "nuojiya");
    display(&cqc, "hetangyuese");
    display(&cqc, "zhonghuarenmingongheguo");
    display(&cqc, "shanghai");
    display(&cqc, "yiyao");
    display(&cqc, "chenshan");
    display(&cqc, "yifu");
    display(&cqc, "shouji");
    display(&cqc, "lianyiqun");
    display(&cqc, "niuzaiku");
    display(&cqc, "pingbandianshi");
    display(&cqc, "dianshi");
    display(&cqc, "paoxie");
    display(&cqc, "huawei");
    display(&cqc, "nikang");
    display(&cqc, "xiangji");
    display(&cqc, "wuxian");
    display(&cqc, "shangwang");
    display(&cqc, "wuxianshangwang");
    display(&cqc, "maozi");
    display(&cqc, "lifu");
    display(&cqc, "weiyi");
    display(&cqc, "duanxiu");
    display(&cqc, "changxiu");
    display(&cqc, "wenxiong");
    display(&cqc, "wazi");
    display(&cqc, "tuoxie");
    display(&cqc, "nanzhuang");

    display(&cqc, "chinese");
    display(&cqc, "chinesa");
    display(&cqc, "be");
    display(&cqc, "a");
    display(&cqc, "z");
    display(&cqc, "quality");
    display(&cqc, "you");
    display(&cqc, "youtube");

    display(&cqc, "中过");
    display(&cqc, "周结论");
    display(&cqc, "蔡依琳");
    display(&cqc, "阵子单");

    display(&cqc, "项链");
    display(&cqc, "响亮");
    display(&cqc, "哈哈");
    display(&cqc, "我们的爱");
    display(&cqc, "耐克");
    display(&cqc, "鞋子");
    display(&cqc, "蝎子");
    display(&cqc, "写字");
}

BOOST_AUTO_TEST_CASE(update_test )
{
    CnQueryCorrection cqc("./cn_qc_res");
    cqc.Load();
    std::cout << "before update" << std::endl;
    display(&cqc, "shanghai");
    typedef boost::tuple<uint32_t, uint32_t, izenelib::util::UString> QueryLogType;
    std::list<QueryLogType> items;
    items.push_back(boost::make_tuple(100,100,izenelib::util::UString("伤害", izenelib::util::UString::UTF_8)));
    cqc.Update(items);
    std::cout << "after update" << std::endl;
    display(&cqc, "shanghai");
}

BOOST_AUTO_TEST_SUITE_END()
