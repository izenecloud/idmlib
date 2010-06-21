/*
 * \file KPESimpleAPI.cpp
 * \brief 
 * \date Apr 27, 2010
 * \author Vernkin Chen
 */

#include <idmlib/keyphrase-extraction/KPESimpleAPI.hpp>
#include <idmlib/keyphrase-extraction/SimpleIDManager.hpp>
#include <idmlib/keyphrase-extraction/fwd.h>

#include <boost/bind.hpp>

NS_IDMLIB_KPE_BEGIN

/**
 * Boost::bind is applied here to hold return value
 */
class RetHolder
{
public:
    RetHolder( std::vector<std::string>* ret )
        : ret_ ( ret )
    {
    }

    void process_ret(const izenelib::util::UString& ustr,
            const std::vector<std::pair<uint32_t, uint32_t> >& id2countList, uint8_t score)
    {
        std::string str;
        ustr.convertString(str, izenelib::util::UString::UTF_8);
        ret_->push_back( str );
    }

public:
    std::vector<std::string>* ret_;
};

void test_func(const izenelib::util::UString& ustr,
        const std::vector<std::pair<uint32_t, uint32_t> >& id2countList, uint8_t score)
{
    std::string str;
    ustr.convertString(str, izenelib::util::UString::UTF_8);
    std::cout<<"Find KP: "<<str<<std::endl;
}

void perform_kpe(
        const string& resPath,
        KPEDataInterator& inputItr,
        std::vector<std::string>& kpeVec,
        KPEAnalyzer* analyzer,
        const string idDataPath,
        const string kpeDataPath
        )
{

    SimpleIDManager idManager( idDataPath, analyzer );

    RetHolder rh( &kpeVec );

    KPE_ALL<SimpleIDManager>::function_type func = boost::bind( &RetHolder::process_ret,
            boost::ref(rh), _1, _2, _3 );
    KPE_ALL<SimpleIDManager> kpe( &idManager, func, kpeDataPath );
    kpe.load(resPath);

    string input;
    uint32_t docId;

    while( inputItr.next( input, docId ) == true )
    {
        izenelib::util::UString article( input.c_str(), izenelib::util::UString::UTF_8);
        kpe.insert( article, docId );
    }

    kpe.close();

}


NS_IDMLIB_KPE_END
