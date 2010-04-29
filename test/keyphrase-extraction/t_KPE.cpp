#include <idmlib/keyphrase-extraction/KPESimpleAPI.hpp>
#include "kpe_types.h"

#include <string>
#include <iostream>
#include <vector>

using namespace idmlib::kpe;

void test_func(const wiselib::UString& ustr, const std::vector<std::pair<uint32_t, uint32_t> >& id2countList, uint8_t score)
{
    std::string str;
    ustr.convertString(str, wiselib::UString::UTF_8);
    std::cout<<"Find KP: "<<str<<std::endl;
}

int maxCount = 30;

class OneStringData : public KPEDataInterator
{
public:
    OneStringData( string& data )
      : data_( data ), begin_( true ), count_( 0 )
    {
    }

    virtual bool next( string& outStr, uint32_t& docId )
    {
        if( begin_  == false )
        {
            return false;
        }

        outStr = data_;
        docId = ++count_;

        if( count_ >= maxCount )
            begin_ = false;

        return true;
    }

private:
    string data_;
    bool begin_;
    int count_;
};


int main()
{
    std::string input;
    while( true )
    {
        cout<<"Document ('x' or 'X' to exit): ";
        getline( cin, input );

        //cout<<"Input: "<<input<<endl;

        if( input == "x" || input == "X" )
            break;

        vector<string> kpeVec;
        OneStringData osd(input);

        perform_kpe( "../resource/kpe", osd, kpeVec,
                "./id", "./tmp");

        cout << "##############\n Number of results: " << kpeVec.size() << endl;
        for( vector<string>::iterator itr = kpeVec.begin(); itr != kpeVec.end(); ++itr )
        {
            cout<< *itr << endl;
        }

        /*
        TestIDManager idManager("./id");
        KPE_ALL<TestIDManager>::function_type func = &test_func;
        KPE_ALL<TestIDManager> kpe( &idManager, func, "./tmp");
        kpe.load("../resource/kpe");
        wiselib::UString article( input.c_str(), wiselib::UString::UTF_8);
        kpe.insert(article, 1);
        kpe.close();
        */
    }
}
