#include <idmlib/duplicate-detection/group_table.h>
#include <boost/test/unit_test.hpp>


using namespace idmlib::dd;

BOOST_AUTO_TEST_SUITE(group_table_test)

BOOST_AUTO_TEST_CASE(no_trans_test)
{
    std::string file = "./test_gt_file";
    boost::filesystem::remove_all(file);
    GroupTable<std::string, uint32_t> table(file);
    table.Load();
    table.AddDoc("1", "2");
    table.AddDoc("3", "4");
    table.AddDoc("6", "5");
    table.AddDoc("10", "11");
    table.Flush();
    BOOST_CHECK( table.IsSameGroup("1","2")==true);
    BOOST_CHECK( table.IsSameGroup("5","6")==true);
    BOOST_CHECK( table.IsSameGroup("11","10")==true);
    BOOST_CHECK( table.IsSameGroup("1","5")==false);
    BOOST_CHECK( table.IsSameGroup("1","11")==false);
    BOOST_CHECK( table.IsSameGroup("3","6")==false);
}

BOOST_AUTO_TEST_CASE(trans_test)
{
    std::string file = "./test_gt_file";
    boost::filesystem::remove_all(file);
    GroupTable<std::string, uint32_t> table(file);
    table.Load();
    table.AddDoc("1", "2");
    table.AddDoc("3", "4");
    table.AddDoc("6", "5");
    table.AddDoc("10", "11");
    table.AddDoc("1", "11");
    table.Flush();
    BOOST_CHECK( table.IsSameGroup("1","2")==true);
    BOOST_CHECK( table.IsSameGroup("5","6")==true);
    BOOST_CHECK( table.IsSameGroup("11","10")==true);
    BOOST_CHECK( table.IsSameGroup("1","5")==false);
    BOOST_CHECK( table.IsSameGroup("1","10")==true);
    BOOST_CHECK( table.IsSameGroup("3","6")==false);
}

BOOST_AUTO_TEST_CASE(inc_trans_test)
{
    std::string file = "./test_gt_file";
    boost::filesystem::remove_all(file);
    GroupTable<std::string, uint32_t> table(file);
    table.Load();
    table.AddDoc("1", "2");
    table.AddDoc("3", "4");
    table.AddDoc("6", "5");
    table.AddDoc("10", "11");
    table.AddDoc("1", "11");
    table.Flush();
    table.AddDoc("23", "1");
    table.AddDoc("24", "4");
    table.AddDoc("23", "24");
    table.Flush();
    BOOST_CHECK( table.IsSameGroup("11","23")==true);
    BOOST_CHECK( table.IsSameGroup("3","24")==true);
    BOOST_CHECK( table.IsSameGroup("23","24")==true);
    BOOST_CHECK( table.IsSameGroup("11","4")==true);
}

BOOST_AUTO_TEST_SUITE_END()

