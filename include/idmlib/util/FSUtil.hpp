///
/// @file FSUtil.hpp
/// @brief Utility for file system operation
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-12
/// @date 
///

#ifndef IDM_FSUTIL_HPP_
#define IDM_FSUTIL_HPP_


#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <util/filesystem.h>
#include <idmlib/idm_types.h>

using namespace boost::filesystem;

NS_IDMLIB_UTIL_BEGIN
    
    class FSUtil
    {
        
        public:
            
            static std::string getTmpFileName( const std::string& dir, const std::string& extName = ".tmp" )
            {
                time_t _time = time(NULL);
                struct tm * timeinfo;
                char buffer[14];
                timeinfo = localtime(&_time);
                strftime(buffer, 80, "%Y%m%d%H%M%S", timeinfo);
                std::string time_stamp = buffer;
                
                std::string fileName = "";
                for( uint32_t num = std::numeric_limits<uint32_t>::min(); num < std::numeric_limits<uint32_t>::max(); ++num)
                {
                    fileName = time_stamp+"-"+boost::lexical_cast<std::string>(num)+extName;
                    if( !exists( dir+"/"+fileName) )
                    {
                        break;
                    }
                }
                return fileName;
            }
            
            static std::string getTmpFileFullName( const std::string& dir, const std::string& extName = ".tmp" )
            {
                std::string fileName = getTmpFileName(dir, extName);
                return dir+"/"+fileName;
            }
            
            static void checkPath(const std::string& path)
            {
                if( !exists(path) )
                {
//                     throw MiningConfigException(path+" does not exist");
                }
            }
            
            static void del(const std::string& path)
            {
                boost::filesystem::remove_all(path);
            }
            
            static void rename(const std::string& from, const std::string& to)
            {
                boost::filesystem::rename(from, to);
            }
            
            static void copy(const std::string& from, const std::string& to)
            {
                izenelib::util::recursive_copy_directory(path(from),path(to));
            }
            
            static bool exists(const std::string& path)
            {
                return boost::filesystem::exists(path);
            }
            
            static void createDir(const std::string& path)
            {
                if( exists(path) )
                {
                    if( !boost::filesystem::is_directory(path) )
                    {
//                         throw FileOperationException(path+" error when creating dir");
                    }
                }
                else
                {
                    bool b = boost::filesystem::create_directories(path);
                    if(!b)
                    {
//                         throw FileOperationException(path+" can not be created");
                    }
                    if( !exists(path) )
                    {
//                         throw FileOperationException(path+" can not be created");
                    }
                }
            }
            
            static void normalizeFilePath(std::string& path)
        	{
        		std::string normalPath;

        		std::string::const_iterator iter;
        		for (iter = path.begin(); iter != path.end(); iter ++)
        		{
        			if (*iter == '\\' || *iter == '/') {
        				if ( (iter+1) != path.end()) {
        					normalPath.push_back('/');
        				}
        			}
        			else {
        				normalPath.push_back(*iter);
        			}
        		}

        		path.swap(normalPath);
        	}
    };
NS_IDMLIB_UTIL_END

#endif
