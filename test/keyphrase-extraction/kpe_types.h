#include <ir/id_manager/IDManager.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
class TestIDManager : public boost::noncopyable
{
public:
TestIDManager(const std::string& path):path_(path)
/*:strStorage_(path+"/id_ustring_map")*/
{
    boost::filesystem::create_directories(path_);
    strStorage_=new izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t>(path+"/id_ustring_map");
}

~TestIDManager()
{
    if(strStorage_)
        delete strStorage_;
}

bool getTermIdByTermString(const izenelib::util::UString& ustr, uint32_t& termId)
{
    termId = izenelib::util::HashFunction<izenelib::util::UString>::generateHash32(ustr);
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    return true;
}

bool getTermIdByTermString(const izenelib::util::UString& ustr, char pos, uint32_t& termId)
{
    termId = izenelib::util::HashFunction<izenelib::util::UString>::generateHash32(ustr);
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    return true;
}

bool getTermStringByTermId(uint32_t termId, izenelib::util::UString& ustr)
{
    boost::mutex::scoped_lock lock(mutex_);
    return strStorage_->get(termId, ustr);
}

void put(uint32_t termId, const izenelib::util::UString& ustr)
{
    boost::mutex::scoped_lock lock(mutex_);
    strStorage_->put(termId, ustr);
    
}

bool isKP(uint32_t termId)
{
    return false;
}

void getAnalysisTermIdList(const izenelib::util::UString& str, std::vector<uint32_t>& termIdList)
{
    std::vector<izenelib::util::UString> termList;
    std::vector<char> posInfoList;
    std::vector<uint32_t> positionList;

    getAnalysisTermIdList( str, termList, termIdList, posInfoList, positionList );
}

void getAnalysisTermIdList(const izenelib::util::UString& str, std::vector<izenelib::util::UString>& termList, std::vector<uint32_t>& idList, std::vector<char>& posInfoList, std::vector<uint32_t>& positionList)
{
    size_t len = str.length();
    char pos = 'C';
    for( size_t i = 0; i < len; ++i )
    {
        izenelib::util::UString term = str.substr( i, 1 );
        termList.push_back( term );
        uint32_t termId;
        getTermIdByTermString( term, termId );
        idList.push_back( termId );
        posInfoList.push_back( pos );
        positionList.push_back( static_cast<uint32_t>( i ) );
    }
}

void flush()
{
    strStorage_->flush();
}

void close()
{
    strStorage_->close();
}

private:        
izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t>* strStorage_;
boost::mutex mutex_;
std::string path_;
        
};
