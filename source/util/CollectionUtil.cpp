#include <idmlib/util/CollectionUtil.h>

using namespace idmlib::util;
using namespace izenelib;


bool CollectionProcessor::processSCD()
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
         std::vector<std::string> list;
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
             if (maxDoc_ != 0 && curDocNum >= maxDoc_)
             {
                 DLOG(INFO) << "processed: " << curDocNum << ")"<<endl;
                 break;
             }
         }
     }

     DLOG(INFO) << "post processing..."<<endl;
     postProcess(); // xxx

     DLOG(INFO) << "End Collection (SCD) processing. "<< endl;
     return true;
}


bool CollectionProcessor::getScdFileList(const std::string& scdDir, std::vector<std::string>& fileList)
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
            std::string file_name = iter->path().filename().string();
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
