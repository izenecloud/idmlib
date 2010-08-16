/*
 * t_qs.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */
#include <boost/test/unit_test.hpp>
#include <boost/date_time.hpp>

#include <idmlib/query-suggestion/Reminder.h>
#include "qs_types.h"

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

using namespace idmlib;





BOOST_AUTO_TEST_SUITE(QS_test)

const uint32_t MAX_READ_BUF_SIZE=1024;
const uint32_t MAX_TIME_SERIRES=7;
const uint32_t REAL_NUM=10;
const uint32_t POP_NUM=20;


BOOST_AUTO_TEST_CASE(normal_test)
{
    boost::filesystem::remove_all("./qstestid");
    boost::filesystem::remove_all("./qstest");
    boost::filesystem::create_directories("./qstest");
    TestIDManager* idManager = new TestIDManager("./qstestid");
    Reminder<TestIDManager>* reminder = new Reminder<TestIDManager>(idManager, "./qstest", REAL_NUM, POP_NUM);

    //Control the experiment log slice number.
    std::list<std::pair<izenelib::util::UString,uint32_t> > popularitem;
    popularitem.push_back( std::make_pair( izenelib::util::UString("游戏", izenelib::util::UString::UTF_8) , 4 ) );
    
    uint32_t count=0;
    std::list<std::pair<izenelib::util::UString,uint32_t> > logitem;
    logitem.push_back( std::make_pair( izenelib::util::UString("中国", izenelib::util::UString::UTF_8) , 4 ) );
    logitem.push_back( std::make_pair( izenelib::util::UString("游戏", izenelib::util::UString::UTF_8) , 2 ) );
    logitem.push_back( std::make_pair( izenelib::util::UString("音乐", izenelib::util::UString::UTF_8) , 3 ) );
    
    
    reminder->indexQueryLog(7, popularitem, false);
    reminder->indexQueryLog(8, popularitem, false);
    reminder->indexQueryLog(9, popularitem, false);
    reminder->indexQueryLog(10, logitem, true);
//     for( uint32_t time= 20100107; time<=20100107; ++time )
//     {
//         
//     }

    reminder->indexQuery();
    std::vector<izenelib::util::UString> realItems;
    reminder->getRealTimeQuery(realItems);
    std::cout<<"realtime count : "<<realItems.size()<<std::endl;
    BOOST_CHECK( realItems.size() > 0 );

    std::vector<izenelib::util::UString> popularItems;
    reminder->getPopularQuery(popularItems);
    std::cout<<"popular count : "<<popularItems.size()<<std::endl;
    BOOST_CHECK( popularItems.size() == 1 );
    delete idManager;
    delete reminder;
//     boost::filesystem::remove_all("./qstestid");
//     boost::filesystem::remove_all("./qstest");
}




BOOST_AUTO_TEST_SUITE_END() 
