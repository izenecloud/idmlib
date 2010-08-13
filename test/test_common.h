#include <string>
#include <boost/filesystem.hpp>
#include <util/functional.h>
class ResourceHelper 
{
public:
    
    
    static bool getKpePath(std::string& path)
    {
        path = "../resource/kpe";
        return boost::filesystem::exists(path);
    }
    
    static bool getNecPath(std::string& path)
    {
        path = "../resource/nec";
        return boost::filesystem::exists(path);
    }


};
