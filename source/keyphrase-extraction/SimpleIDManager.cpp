#include <idmlib/keyphrase-extraction/SimpleIDManager.hpp>

#include <idmlib/keyphrase-extraction/KPESimpleAPI.hpp>

NS_IDMLIB_KPE_BEGIN

using std::vector;

SimpleIDManager::SimpleIDManager( const std::string& path, KPEAnalyzer* analyzer )
    : path_(path), analyzer_( analyzer )
{
    boost::filesystem::create_directories(path_);
    strStorage_=new izenelib::ir::idmanager::HDBIDStorage< wiselib::UString, uint32_t>(path+"/id_ustring_map");
}

SimpleIDManager::~SimpleIDManager()
{
    if(strStorage_)
        delete strStorage_;
}

bool SimpleIDManager::getTermIdByTermString(const wiselib::UString& ustr, uint32_t& termId)
{
    termId = izenelib::util::HashFunction<wiselib::UString>::generateHash32(ustr);
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    return true;
}

bool SimpleIDManager::getTermIdByTermString(const wiselib::UString& ustr, char pos, uint32_t& termId)
{
    termId = izenelib::util::HashFunction<wiselib::UString>::generateHash32(ustr);
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    return true;
}

bool SimpleIDManager::getTermStringByTermId(uint32_t termId, wiselib::UString& ustr)
{
    boost::mutex::scoped_lock lock(mutex_);
    return strStorage_->get(termId, ustr);
}

void SimpleIDManager::put(uint32_t termId, const wiselib::UString& ustr)
{
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    
}

bool SimpleIDManager::isKP(uint32_t termId)
{
    return false;
}

void SimpleIDManager::getAnalysisTermIdList(const wiselib::UString& str, std::vector<uint32_t>& termIdList)
{
    std::vector<wiselib::UString> termList;
    std::vector<char> posInfoList;
    std::vector<uint32_t> positionList;

    getAnalysisTermIdList( str, termList, termIdList, posInfoList, positionList );
}

void SimpleIDManager::getAnalysisTermIdList(
        const wiselib::UString& str,
        std::vector<wiselib::UString>& termList,
        std::vector<uint32_t>& idList,
        std::vector<char>& posInfoList,
        std::vector<uint32_t>& positionList
        )
{
    if( analyzer_ == NULL )
    {
        size_t len = str.length();
        char pos = 'C';
        for( size_t i = 0; i < len; ++i )
        {
            wiselib::UString term = str.substr( i, 1 );
            // Only handle Chinese Characters
            if( term.isChineseChar( 0 ) == false )
                continue;

            posInfoList.push_back( pos );
            termList.push_back( term );
            uint32_t termId;
            getTermIdByTermString( term, termId );
            idList.push_back( termId );

            positionList.push_back( static_cast<uint32_t>( i ) );
        }
    }
    else
    {
        string strForm;
        str.convertString( strForm, wiselib::UString::UTF_8 );
        vector< string > strTerms;
        analyzer_->analyze( strForm.c_str(), strTerms, posInfoList, positionList );

        // convert terms from string to UString and update termId
        for( vector< string >::iterator itr = strTerms.begin(); itr != strTerms.end(); ++itr )
        {
            wiselib::UString term;
            term.assign( itr->c_str(), wiselib::UString::UTF_8 );
            termList.push_back( term );
            uint32_t termId;
            getTermIdByTermString( term, termId );
            idList.push_back( termId );
        }
    }
}

void SimpleIDManager::flush()
{
    strStorage_->flush();
}

void SimpleIDManager::close()
{
    strStorage_->close();
}


NS_IDMLIB_KPE_END

