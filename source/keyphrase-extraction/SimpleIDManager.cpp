#include <idmlib/keyphrase-extraction/SimpleIDManager.hpp>

NS_IDMLIB_KPE_BEGIN

SimpleIDManager::SimpleIDManager(const std::string& path):path_(path)
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

void SimpleIDManager::getAnalysisTermIdList(const wiselib::UString& str, std::vector<wiselib::UString>& termList, std::vector<uint32_t>& idList, std::vector<char>& posInfoList, std::vector<uint32_t>& positionList)
{
    size_t len = str.length();
    char pos = 'C';
    for( size_t i = 0; i < len; ++i )
    {
        wiselib::UString term = str.substr( i, 1 );
        termList.push_back( term );
        uint32_t termId;
        getTermIdByTermString( term, termId );
        idList.push_back( termId );
        posInfoList.push_back( pos );
        positionList.push_back( static_cast<uint32_t>( i ) );
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

