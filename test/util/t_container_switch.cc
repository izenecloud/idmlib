
#include <boost/test/unit_test.hpp>

#include <idmlib/util/container_switch.h>
#include "../TestResources.h"

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


class SimpleContainer
{
public:
    SimpleContainer(const std::string& dir):dir_(dir), value("")
    {
    }
    
    bool Open()
    {
        boost::filesystem::create_directories(dir_);
        std::string file = dir_+"/file";
        izenelib::am::ssf::Util<>::Load(file, value);
        return true;
    }
    
    
    void Flush()
    {
        std::string file = dir_+"/file";
        izenelib::am::ssf::Util<>::Save(file, value);
    }
    
   
private:
    std::string dir_;
public:
    std::string value; 
    
};

typedef ContainerSwitch<SimpleContainer> SwitchType;

BOOST_AUTO_TEST_SUITE(t_container_switch)

BOOST_AUTO_TEST_CASE(normal_test)
{
    std::string test_dir = "./switch_test";
    boost::filesystem::remove_all(test_dir);
    SwitchType sw(test_dir);
    bool b = sw.Open();
    BOOST_CHECK(b);
    SimpleContainer* c = sw.Current();
    BOOST_CHECK(c->value=="");
    SimpleContainer* next = sw.Next();
    BOOST_CHECK(next->value=="");
    next->value = "aaa";
    BOOST_CHECK(sw.Current()->value=="");
    next->Flush();
    BOOST_CHECK(sw.EnsureSwitch());
    BOOST_CHECK(sw.Current()->value=="aaa");
    next = sw.Next();
    next->value = "bbb";
    BOOST_CHECK(sw.Current()->value=="aaa");
    next->Flush();
    BOOST_CHECK(sw.EnsureSwitch());
    BOOST_CHECK(sw.Current()->value=="bbb");
}

BOOST_AUTO_TEST_CASE(excep_usage_test)
{
    std::string test_dir = "./switch_test";
    boost::filesystem::remove_all(test_dir);
    SwitchType sw(test_dir);
    bool b = sw.Open();
    BOOST_CHECK(b);
    SimpleContainer* c = sw.Current();
    BOOST_CHECK(c->value=="");
    SimpleContainer* next = sw.Next();
    BOOST_CHECK(next->value=="");
    next->value = "aaa";
    BOOST_CHECK(sw.Current()->value=="");
    next->Flush();
    next = sw.Next();
    BOOST_CHECK(sw.Current()->value=="aaa");
    
    next->value = "bbb";
    BOOST_CHECK(sw.Current()->value=="aaa");
    next->Flush();
    BOOST_CHECK(sw.EnsureSwitch());
    BOOST_CHECK(sw.Current()->value=="bbb");
}


BOOST_AUTO_TEST_SUITE_END() 
