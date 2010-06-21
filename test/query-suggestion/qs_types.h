#include <ir/id_manager/IDManager.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include <LA.h>
#include <util/functional.h>
class TestIDManager : public boost::noncopyable
{
public:
	TestIDManager(const std::string& path):path_(path)
	/*:strStorage_(path+"/id_ustring_map")*/
	{
	    boost::filesystem::create_directories(path_);
	    strStorage_=new izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t>(path+"/id_ustring_map");
	    initializeLa();
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

	void getAnalysisTermIdList(const izenelib::util::UString& str, std::vector<uint32_t>& termIdList)
	{
	    std::vector<izenelib::util::UString> termList;
	    la::TermList laTermList;
	    la_.process_search( str, laTermList);
	    for (la::TermList::iterator p = laTermList.begin(); p != laTermList.end(); p++)
	    {
	        termList.push_back(p->text_);
	    }
	    for(size_t i=0;i<termList.size();i++)
	    {
	    	uint32_t termId;
	    	getTermIdByTermString( termList[i], termId );
	    	termIdList.push_back(termId);
	    }
	}
    
    void getAnalysisTermList(const izenelib::util::UString& str, std::vector<izenelib::util::UString>& strList, std::vector<uint32_t>& termIdList)
    {
        la::TermList laTermList;
        la_.process_search( str, laTermList);
        for (la::TermList::iterator p = laTermList.begin(); p != laTermList.end(); p++)
        {
            strList.push_back(p->text_);
        }
        for(size_t i=0;i<strList.size();i++)
        {
            uint32_t termId;
            getTermIdByTermString( strList[i], termId );
            termIdList.push_back(termId);
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

	bool initializeLa()
	{
		const int min=2;
		const int max=2;
		la::TokenizeConfig config;
		la_.setTokenizerConfig( config );
		boost::shared_ptr<la::Analyzer> analyzer;
		analyzer.reset( new la::NGramAnalyzer( min, max, 16 ) );
		static_cast<la::NGramAnalyzer*>(analyzer.get())->setApartFlag( la::NGramAnalyzer::NGRAM_APART_ALL_ );
		la_.setAnalyzer( analyzer );
		return true;
	}

private:        
	izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t>* strStorage_;
	boost::mutex mutex_;
	std::string path_;
	la::LA la_;


};
