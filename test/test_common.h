#include <string>
#include <boost/filesystem.hpp>
#include <util/functional.h>
class ResourceHelper 
{
public:
    
    
    static bool getKpePath(std::string& path)
    {
        path = "../package/resource/kpe";
        return boost::filesystem::exists(path);
    }
    
    static bool getNecPath(std::string& path)
    {
        path = "../package/resource/nec";
        return boost::filesystem::exists(path);
    }

    static bool getNewNecPath(std::string& path)
    {
        path = "../package/resource/nec/chinese_svm";
        return boost::filesystem::exists(path);
    }

};
