
/// @file   t_UString.cpp
/// @brief  A test unit for checking if all interfaces is 
///         available to use.
/// @author Do Hyun Yun 
/// @date   2008-07-11
///
///  
/// @brief Test all the interfaces in UString class.
///
/// @details
/// 
/// ==================================== [ Test Schemes ] ====================================
///
///
/// -# Tested basic part of UString according to the certain scenario with simple usage.\n
/// \n 
///     -# Create three UString variables in different ways : Default Initializing, Initializing with another UString, and initialize with stl string class.\n\n
///     -# Check attributes of some characters in UString using is_____Char() interface. With this interface, it is possible to recognize certain character is alphabet or number or something.\n\n
///     -# Get attribute of certain characters in UString using charType() interface.\n\n
///     -# Change some characters into upper alphabet or lower alphabet using toUpperChar() and toLowerChar(), and toLowerString() which changes all characters in UString into lower one.\n\n
///     -# With given pattern string, Get the index of matched position by using find(). \n\n
///     -# Create the sub-string using subString() with the index number which is the result of find().\n\n
///     -# Assign string data in different ways using assign(), format() interfaces and "=" "+=" operators.\n\n
///     -# Export UString data into stl string class according to the encoding type.\n\n
///     -# Check size, buffer size, and its length. Clear string data and re-check its information including empty().\n\n
/// \n
/// -# Tested all the interfaces by using correct and incorrect test sets.
//#include <util/log.h
#include <idmlib/duplicate-detection/DupDetector.h>
#include <idmlib/duplicate-detection/rand_proj_gen.h>
#include <idmlib/duplicate-detection/rand_proj.h>
#include <string>
#include <time.h>
#include <math.h>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem/path.hpp> 
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp> 
#include <sys/time.h>
#include <fstream>
#include <iostream>
#include <vector>
#include<stdio.h>

BOOST_AUTO_TEST_SUITE( t_duplication_detection_suite )
using namespace std;
using namespace sf1v5;
using namespace boost::unit_test;

#define CHECK(f)\
  {     \
    if (!(f)){ BOOST_CHECK(false); std::cout<<"ERROR: "<<__FILE__<<": "<<__LINE__<<": "<<__FUNCTION__<<endl;} \
  }
#define ERROR_COUNT {if(error_count>0)cout<<endl<<error_count<<" errors ware found!";else{cout<<"\nNo error detected!\n"}}

void rand_str(string& s)
{
  s.clear();
  
  size_t l = rand()%10;
  while (l == 0)
    l = rand()%10;

  for (size_t i=0; i<l; ++i)
    s += 'a' + i;
}

void rand_doc(std::vector<std::string>& strs, size_t max=100)
{
  strs.clear();
  size_t l = rand()%max;
  while (l == 0)
    l = rand()%max;
  
  for (size_t i=0; i<l; ++i)
  {
    string t;
    rand_str(t);
    strs.push_back(t);
  }
  
}

bool match(const std::string& path, const char* prefix)
{
    uint32_t i = path.find_last_of('/');
    i = path.substr(i+1).find(prefix);
    if (i != (uint32_t)-1)
        return true;
    return false;
}

void remove(const char* prefix)
{
    boost::filesystem::path full_path(   "./"   ,boost::filesystem::native);
    if (boost::filesystem::exists(full_path))
         {
       boost::filesystem::directory_iterator item_begin(full_path);
       boost::filesystem::directory_iterator item_end;
       for ( ;item_begin   !=  item_end; item_begin ++ )
                 {
           if (match(item_begin ->path().native_file_string(), prefix))
        		boost::filesystem::remove(item_begin ->path().native_file_string());
           //cout  << item_begin ->path().native_file_string() << " \t[dir] " << endl;
                 }
        }
}

BOOST_AUTO_TEST_CASE(RandProjGen_check )
{
  boost::filesystem::remove_all("./tt");
  const size_t SIZE= 100000;

  vector<std::string> strs;
  vector<RandProj> projs;

  {
    RandProjGen pg("./tt", 384);

    struct timeval tvafter,tvpre;
    struct timezone tz;
    gettimeofday (&tvpre , &tz);
    for (uint32_t i=0; i<SIZE; ++i)
    {
      string str;
      rand_str(str);
      strs.push_back(str);
      projs.push_back(pg.get_random_projection(str));
    }

    gettimeofday (&tvafter , &tz);
    cout<<"\nFP Generation: "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/1000.<<std::endl;
    pg.commit();
  }
  {
    RandProjGen pg("./tt", 384);
    for (uint32_t i=0; i<SIZE; ++i)
    {
      if (!(pg.get_random_projection(strs[i]) == projs[i]))
      {
        std::cout<<pg.get_random_projection(strs[i])<<endl;
        cout<<projs[i]<<endl;
        
        CHECK(false);
        return;
      }
    }
  }

}

BOOST_AUTO_TEST_CASE(DupDetector_check )
{
  boost::filesystem::remove_all("./tt");
  
  const size_t SIZE= 20;//1000000;

  const size_t TYPES_NUM= 4;//40000;

  vector<vector<string> >  v;
  v.reserve(TYPES_NUM);
  for (size_t i = 0; i<TYPES_NUM; i++)
  {
    vector<string> vs;
    rand_doc(vs);
    v.push_back(vs);
  }

  cout<<"Data is ready!\n";
  boost::filesystem::remove_all("./tt");

  struct timeval tvafter,tvpre;
  struct timezone tz;
  {
    DupDetector dupd(1, "./tt");
  
  gettimeofday (&tvpre , &tz);
  
  dupd.ready_for_insert();
  
  for (size_t i=0; i<SIZE; i++)
    dupd.insertDocument(i, v[i%TYPES_NUM]);
  gettimeofday (&tvafter , &tz);
  cout<<"Adding docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  //  getchar();
  
  gettimeofday (&tvpre , &tz);
  dupd.runDuplicateDetectionAnalysis();
  gettimeofday (&tvafter , &tz);
  cout<<"Indexing docs is over! "<<((tvafter.tv_sec-tvpre.tv_sec)*1000+(tvafter.tv_usec-tvpre.tv_usec)/1000)/60000.<<std::endl;
  //getchar();
  }
  
  //test for incremental
  {
    DupDetector dupd(1, "./tt");
    dupd.ready_for_insert();
    
    for (size_t i=SIZE; i<SIZE+2*TYPES_NUM; i++)
      dupd.insertDocument(i, v[i%TYPES_NUM]);

    dupd.runDuplicateDetectionAnalysis();
  }

  //test for updating
  {
    DupDetector dupd(1, "./tt");
    dupd.ready_for_insert();
    dupd.updateDocument(1, v[0%TYPES_NUM]);
    dupd.updateDocument(9, v[0%TYPES_NUM]);
    dupd.runDuplicateDetectionAnalysis();

    dupd.ready_for_insert();
    dupd.updateDocument(1, v[1%TYPES_NUM]);
    dupd.updateDocument(9, v[9%TYPES_NUM]);
    dupd.runDuplicateDetectionAnalysis();

    dupd.ready_for_insert();
    dupd.removeDocument(SIZE+1);
    dupd.removeDocument(SIZE+2);
    dupd.runDuplicateDetectionAnalysis();
    
    dupd.ready_for_insert();
    
    dupd.insertDocument(SIZE+1, v[(SIZE+1)%TYPES_NUM]);
    dupd.insertDocument(SIZE+2, v[(SIZE+2)%TYPES_NUM]);

    dupd.runDuplicateDetectionAnalysis();
  }
  
  {
    DupDetector dupd(1, "./tt");
    vector<unsigned int> ids;
    dupd.getDuplicatedDocIdList(1, ids);

    // for (uint32_t i=0; i<ids.size(); ++i)
//       cout<<ids[i]<<" ";
//     cout<<endl;
    
    for (size_t i=0; i<SIZE+TYPES_NUM; ++i)
    {
      //cout<<i<<" "<<i+TYPES_NUM<<std::endl;
      CHECK(dupd.isDuplicated(i, i+TYPES_NUM));
    }
  }
  
    remove("tt");
    remove("fp_");

}

BOOST_AUTO_TEST_SUITE_END()
