/**
 * @file CollectionUtil.h
 * @author Zhongxia Li
 * @date Jun 1, 2011
 * @brief 
 */
#ifndef COLLECTION_UTIL_H_
#define COLLECTION_UTIL_H_

#include <idmlib/idm_types.h>
#include <idmlib/util/idm_analyzer.h>
#include <idmlib/util/FSUtil.hpp>

#include <la/LA.h>
#include <la/util/UStringUtil.h>

#include <ir/index_manager/utility/system.h>
#include <ir/id_manager/IDManager.h>
#include <ir/index_manager/index/LAInput.h>
// prevent conflicting with ScdParser of sf1-revolution
#ifndef __SCD__PARSER__H__
#define __SCD__PARSER__H__
#include <util/scd_parser.h>
#endif

using namespace boost::filesystem;
using namespace izenelib::ir::idmanager;
using namespace izenelib::ir::indexmanager;
using namespace izenelib::util;
using namespace idmlib::util;


NS_IDMLIB_UTIL_BEGIN

struct CollectionPath
{
    CollectionPath(const std::string& basePath)
    : basePath_(basePath)
    {
        idmlib::util::FSUtil::normalizeFilePath(basePath_);

        scdPath_ = basePath_ + "/scd/index";
        dataPath_ = basePath_ + "/collection-data/default-collection-dir";
        dataIdPath_ = dataPath_ + "/id";
    }

    std::string basePath_;
    std::string scdPath_;
    std::string dataPath_;
    std::string dataIdPath_;
};


/**
 * Base
 */
class CollectionProcessor
{
public:
    typedef std::vector<std::pair<izenelib::util::UString, izenelib::util::UString> >::iterator doc_properties_iterator;

public:
    /**
     *
     * @param colBasePath base directory of collection
     * @param laResPath LA(CMA) resource path
     * @param maxDoc max documents to be processed, not limited if it's 0.
     * @param encoding encoding of SCD
     */
    CollectionProcessor(
        const std::string& colBasePath,
        const std::string& laResPath,
        size_t maxDoc = 0,
        izenelib::util::UString::EncodingType encoding = izenelib::util::UString::UTF_8)
    : colPath_(colBasePath)
    , laResPath_(laResPath)
    , maxDoc_(maxDoc)
    , encoding_(encoding)
    , curDocId_(0)
    , content_("", encoding)
    , pTermIdList_(new TermIdList())
    {
        createIdManager();
        createAnalyzer();
    }

public:
    bool processSCD()
    {
         std::vector<std::string> scdFileList;
         if (!getScdFileList(colPath_.scdPath_, scdFileList)) {
             return false;
         }

         DLOG(INFO) << "Start Collection (SCD) processing." << endl;

         // parsing all SCD files
         for (size_t i = 1; i <= scdFileList.size(); i++)
         {
             std::string scdFile = scdFileList[i-1];
             ScdParser scdParser(encoding_);
             if(!scdParser.load(scdFile) )
             {
               DLOG(WARNING) << "Load scd file failed: " << scdFile << std::endl;
               return false;
             }

             // check total count
             std::vector<izenelib::util::UString> list;
             scdParser.getDocIdList(list);
             size_t totalDocNum = list.size();

             DLOG(INFO) << "Start to process SCD file (" << i <<" / " << scdFileList.size() <<"), total documents: " << totalDocNum << endl;

             // parse documents
             size_t curDocNum = 0;
             for (ScdParser::iterator iter = scdParser.begin(); iter != scdParser.end(); iter ++)
             {
                 curSCDDoc_ = *iter;
                 processDocument(); // xxx

                 curDocNum ++;
                 if (curDocNum % 2000 == 0) {
                     DLOG(INFO) << "["<<scdFile<<"] processed: " << curDocNum<<" / total: "<<totalDocNum
                                << " - "<< curDocNum*100.0f / totalDocNum << "%" << endl;
                 }

                 // stop
                 if (maxDoc_ != 0 && curDocNum > maxDoc_)
                     break;
             }
         }

         postProcess(); // xxx

         DLOG(INFO) << "End Collection (SCD) processing." << endl;
         return true;
    }

protected:
    /**
     * Sub class can override this function to process other properties if needed.
     * @param pDoc SCD document
     */
    virtual void processDocument()
    {
        content_.clear();

        doc_properties_iterator proIter;
        for (proIter = curSCDDoc_->begin(); proIter != curSCDDoc_->end(); proIter ++)
        {
            izenelib::util::UString propertyName = proIter->first;
            const izenelib::util::UString & propertyValue = proIter->second;
            propertyName.toLowerString();

            if ( propertyName == izenelib::util::UString("docid", encoding_) ) {
                bool ret = pIdManager_->getDocIdByDocName(propertyValue, curDocId_, false);
                if (ret) ;
            }
            if ( propertyName == izenelib::util::UString("title", encoding_) ) {
                // increase the weight of title?
                content_ += propertyValue;
                content_ += propertyValue;
            }
            else if ( propertyName == izenelib::util::UString("content", encoding_)) {
                content_ += propertyValue;
            }
            else {
                continue;
            }
        }

        processDocumentContent(); // xxx
    }


    virtual void processDocumentContent()
    {
        //cout << la::to_utf8(content_) << endl;

        pTermIdList_->clear();
        pIdmAnalyzer_->GetTermIdList(pIdManager_.get(), content_, *pTermIdList_);

        processDocumentAnalyzedContent(); // xxx
    }

    /**
     * Sub class can override this function for concrete processing.
     */
    virtual void processDocumentAnalyzedContent()
    {

    }

    /**
     * Sub class can override this function for concrete processing.
     */
    virtual void postProcess()
    {

    }

protected:
    void createIdManager()
    {
        if (!exists(colPath_.dataIdPath_)) {
            DLOG(ERROR) <<"Not existed: " <<colPath_.dataIdPath_ <<std::endl;
        }
        pIdManager_.reset(new IDManager(colPath_.dataIdPath_));
        BOOST_ASSERT(pIdManager_);
    }

    void createAnalyzer()
    {
        pIdmAnalyzer_.reset(
                new idmlib::util::IDMAnalyzer(laResPath_, la::ChineseAnalyzer::maximum_match) );
        BOOST_ASSERT(pIdmAnalyzer_);
    }

    bool getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList)
    {
        if ( exists(scdDir) )
        {
            if ( !is_directory(scdDir) ) {
                std::cout << "It's not a directory: " << scdDir << std::endl;
                return false;
            }

            directory_iterator iterEnd;
            for (directory_iterator iter(scdDir); iter != iterEnd; iter ++)
            {
                std::string file_name = iter->path().filename();
                //std::cout << file_name << endl;

                if (ScdParser::checkSCDFormat(file_name) )
                {
                    SCD_TYPE scd_type = ScdParser::checkSCDType(file_name);
                    if( scd_type == INSERT_SCD ||scd_type == UPDATE_SCD )
                    {
                        cout << "scd file: "<<iter->path().string() << endl;
                        fileList.push_back( iter->path().string() );
                    }
                }
            }

            if (fileList.size() > 0) {
                return true;
            }
            else {
                std::cout << "There is no scd file in: " << scdDir << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "File path dose not existed: " << scdDir << std::endl;
            return false;
        }
    }

protected:
    CollectionPath colPath_;
    std::string laResPath_;
    size_t maxDoc_;
    izenelib::util::UString::EncodingType encoding_;

    // Language Analyzer
    boost::shared_ptr<idmlib::util::IDMAnalyzer> pIdmAnalyzer_;
    // ID Manager
    boost::shared_ptr<IDManager> pIdManager_;

    // temporary: current document information
    docid_t curDocId_;
    SCDDocPtr curSCDDoc_;
    izenelib::util::UString content_;
    boost::shared_ptr<TermIdList> pTermIdList_;
};


NS_IDMLIB_UTIL_END

#endif /* COLLECTION_UTIL_H_ */
