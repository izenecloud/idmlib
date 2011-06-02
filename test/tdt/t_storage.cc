#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/tdt/storage.h>


BOOST_AUTO_TEST_SUITE( t_tdt_storage )

using namespace idmlib::tdt;

BOOST_AUTO_TEST_CASE(init_test )
{
    std::string dir = "./tdt_storage_test";
    boost::filesystem::remove_all(dir);
    Storage* storage = new Storage(dir);
    bool bo = storage->Open();
    BOOST_CHECK( bo == true );
    delete storage;
    storage = new Storage(dir);
    bo = storage->Open();
    BOOST_CHECK( bo == true );
    delete storage;
}

BOOST_AUTO_TEST_CASE(empty_test )
{
    std::string dir = "./tdt_storage_test";
    boost::filesystem::remove_all(dir);
    Storage* storage = new Storage(dir);
    bool bo = storage->Open();
    BOOST_CHECK( bo );
    TimeIdType start = boost::gregorian::from_string("2011-01-01");
    TimeIdType end = boost::gregorian::from_string("2011-12-31");
    std::vector<izenelib::util::UString> vec;
    bool b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( b );
    BOOST_CHECK( vec.size()==0 );
    vec.clear();
    end = boost::gregorian::from_string("2011-01-01");
    b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( b );
    BOOST_CHECK( vec.size()==0 );
    vec.clear();
    end = boost::gregorian::from_string("2010-01-01");
    b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( !b );
    BOOST_CHECK( vec.size()==0 );
    delete storage;
}

BOOST_AUTO_TEST_CASE(normal_test )
{
    std::string dir = "./tdt_storage_test";
    boost::filesystem::remove_all(dir);
    Storage* storage = new Storage(dir);
    bool bo = storage->Open();
    BOOST_CHECK( bo );
    TrackResult tr;
    tr.text = izenelib::util::UString("earthquake", izenelib::util::UString::UTF_8);
    tr.start_time = boost::gregorian::from_string("2011-01-01");
    tr.burst.resize(2);
    tr.burst[0].start_time = boost::gregorian::from_string("2011-01-10");
    tr.burst[0].period = 2;
    tr.burst[1].start_time = boost::gregorian::from_string("2011-01-20");
    tr.burst[1].period = 3;
    tr.ts.resize(31);
    for(uint32_t i=0;i<tr.ts.size();i++)
    {
        tr.ts[i] = 20;
    }
    std::vector<izenelib::util::UString> similar_topics;
    similar_topics.push_back(izenelib::util::UString("sa", izenelib::util::UString::UTF_8));
    similar_topics.push_back(izenelib::util::UString("sb", izenelib::util::UString::UTF_8));
    bool b = storage->Add(tr, similar_topics);
    BOOST_CHECK( b );
    storage->Flush();
    
    TimeIdType start = boost::gregorian::from_string("2011-01-01");
    TimeIdType end = boost::gregorian::from_string("2011-12-31");
    std::vector<izenelib::util::UString> vec;
    b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( b );
    BOOST_CHECK( vec.size()==1 );
    BOOST_CHECK( vec[0] == izenelib::util::UString("earthquake", izenelib::util::UString::UTF_8) );
    vec.clear();
    
    start = boost::gregorian::from_string("2011-01-22");
    b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( b );
    BOOST_CHECK( vec.size()==1 );
    BOOST_CHECK( vec[0] == izenelib::util::UString("earthquake", izenelib::util::UString::UTF_8) );
    vec.clear();
    
    start = boost::gregorian::from_string("2011-01-23");
    b = storage->GetTopicsInTimeRange(start, end, vec);
    BOOST_CHECK( b );
    BOOST_CHECK( vec.size()==0 );
    vec.clear();
    
    TopicInfoType info;
    b = storage->GetTopicInfo(tr.text, info);
    BOOST_CHECK( b );
    BOOST_CHECK( info.first.text == tr.text );
    BOOST_CHECK( info.second.size() == 2 );
    
    
    delete storage;
}

BOOST_AUTO_TEST_SUITE_END()
