#ifndef SIMPLEIDMANAGER_HPP_
#define SIMPLEIDMANAGER_HPP_

#include <ir/id_manager/IDManager.h>
#include <idmlib/idm_types.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>

NS_IDMLIB_KPE_BEGIN

class KPEAnalyzer;

class SimpleIDManager : public boost::noncopyable
{
public:

SimpleIDManager(const std::string& path, KPEAnalyzer* analyzer = NULL );

~SimpleIDManager();

bool getTermIdByTermString(const izenelib::util::UString& ustr, uint32_t& termId);

bool getTermIdByTermString(const izenelib::util::UString& ustr, char pos, uint32_t& termId);

bool getTermStringByTermId(uint32_t termId, izenelib::util::UString& ustr);

void put(uint32_t termId, const izenelib::util::UString& ustr);

bool isKP(uint32_t termId);

void getAnalysisTermIdList(const izenelib::util::UString& str, std::vector<uint32_t>& termIdList);

void getAnalysisTermIdList(
        const izenelib::util::UString& str,
        std::vector<izenelib::util::UString>& termList,
        std::vector<uint32_t>& idList,
        std::vector<char>& posInfoList,
        std::vector<uint32_t>& positionList
        );

void flush();

void close();

private:        
    izenelib::ir::idmanager::HDBIDStorage< izenelib::util::UString, uint32_t>* strStorage_;
    boost::mutex mutex_;
    std::string path_;
    KPEAnalyzer* analyzer_;
        
};

NS_IDMLIB_KPE_END

#endif

