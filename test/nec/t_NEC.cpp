// by Jia, 2010-08-12
#include <boost/test/unit_test.hpp>
#include <idmlib/nec/NameEntity.h>
#include <idmlib/nec/NameEntityUtil.h>
#include <ml/Evaluator.h>
#include <idmlib/nec/NameEntityDict.h>
#include <idmlib/nec/NameEntityManager.h>
#include <idmlib/nec/nec.h>
#include <fstream>
#include <string>
#include <set>
#include <idmlib/util/StringUtil.hpp>
#include "../test_common.h"
using namespace idmlib;
using namespace idmlib::nec;



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

BOOST_AUTO_TEST_CASE(newnec_test)
{
    std::string res_path;
    BOOST_CHECK( ResourceHelper::getNewNecPath(res_path) == true );
    idmlib::nec::NEC nec;
    nec.Load(res_path);
    typedef std::pair<uint32_t, uint32_t> id2count_t;
    typedef std::pair<izenelib::util::UString, uint32_t> str2count_t;
    std::vector<id2count_t> b(1, std::make_pair(1,1));
    std::vector<str2count_t> c;
    std::vector<str2count_t> d;
    double correct = 0.0;
    uint32_t total = 0;
    {
        
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("张国荣", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            NECItem item( inputList[w], b,c,d );
            int result = nec.Predict(item);
            if( result==1 )
            {
                correct += 1;
            }
            total += 1;
        }
    }
    
    {
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("中国", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            NECItem item( inputList[w], b,c,d );
            int result = nec.Predict(item);
            if( result==2 )
            {
                correct += 1;
            }
            total += 1;
        }
    }
    
    {
        std::vector<izenelib::util::UString> inputList;
        inputList.push_back( izenelib::util::UString("人民银行", izenelib::util::UString::UTF_8) );
        
        for(uint32_t w=0;w<inputList.size();w++)
        {
            NECItem item( inputList[w], b,c,d );
            int result = nec.Predict(item);
            if( result==3 )
            {
                correct += 1;
            }
            total += 1;
        }
    }
    
    double rate = correct/total;
    std::cout<<"new nec rate : "<<rate<<std::endl;
    BOOST_CHECK( rate >= 0.5 );
}




BOOST_AUTO_TEST_SUITE_END() 
