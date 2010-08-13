// by Jia, 2010-08-12
#include <boost/test/unit_test.hpp>
#include <idmlib/nec/NameEntity.h>
#include <idmlib/nec/NameEntityUtil.h>
#include <ml/Evaluator.h>
#include <idmlib/nec/NameEntityDict.h>
#include <idmlib/nec/NameEntityManager.h>
#include <fstream>
#include <string>
#include <set>
#include <idmlib/util/StringUtil.hpp>
#include "../test_common.h"
using namespace idmlib;



BOOST_AUTO_TEST_SUITE(NEC_test)


BOOST_AUTO_TEST_CASE(normal_test)
{
    std::string res_path;
    BOOST_CHECK( ResourceHelper::getNecPath(res_path) == true );
    NameEntityManager& neMgr = NameEntityManager::getInstance(res_path);
    std::vector<izenelib::util::UString> prefixList;
    std::vector<izenelib::util::UString> suffixList;
    double correct = 0.0;
    uint32_t total = 0;
    {
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("张国荣", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            izenelib::util::UString word = inputList[w];
            idmlib::NameEntity ne(word, prefixList, suffixList);
            neMgr.predict(ne);
            std::vector<ml::Label>& mllabels= ne.predictLabels;
            for(uint32_t i=0;i<mllabels.size();i++)
            {
                if(mllabels[i] == "PEOP" )
                {
                    correct += 1;
                }
                
                break;
            }
            total += 1;
        }
    }
    
    {
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("中国", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            izenelib::util::UString word = inputList[w];
            idmlib::NameEntity ne(word, prefixList, suffixList);
            neMgr.predict(ne);
            std::vector<ml::Label>& mllabels= ne.predictLabels;
            for(uint32_t i=0;i<mllabels.size();i++)
            {
                if(mllabels[i] == "LOC" )
                {
                    correct += 1;
                }
                
                break;
            }
            total += 1;
        }
    }
    
    {
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("人民银行", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            izenelib::util::UString word = inputList[w];
            idmlib::NameEntity ne(word, prefixList, suffixList);
            neMgr.predict(ne);
            std::vector<ml::Label>& mllabels= ne.predictLabels;
            for(uint32_t i=0;i<mllabels.size();i++)
            {
                if(mllabels[i] == "ORG" )
                {
                    correct += 1;
                }
                
                break;
            }
            total += 1;
        }
    }
    
    double rate = correct/total;
    BOOST_CHECK( rate >= 0.5 );
}




BOOST_AUTO_TEST_SUITE_END() 
