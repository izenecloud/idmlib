///
/// @file Util.hpp
/// @brief Utility for iDMlib
/// @date Created 2010-05-07
/// @date 
///

#ifndef IDM_UTIL_HPP_
#define IDM_UTIL_HPP_

#include <string>
#include <vector>

namespace idmlib {


inline static void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	// Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos=str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos)
	{
		std::string strToken=str.substr(lastPos, pos-lastPos);
		if(strToken!="") tokens.push_back(strToken);
        // Skip delimiters.
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
	}
}

inline static void trimLeft(std::string& str, const char* chars2remove)
{
	if (!str.empty())
	{
		std::string::size_type pos = str.find_first_not_of(chars2remove);
		if (pos != std::string::npos)
		    str.erase(0,pos);
		else
		    str.erase( str.begin() , str.end() ); // make empty
	}
}

inline static void trimRight(std::string& str, const char* chars2remove)
{
	if (!str.empty())
	{
		std::string::size_type pos = str.find_last_not_of(chars2remove);
		if (pos != std::string::npos)
		    str.erase(pos+1);
		else
		    str.erase( str.begin() , str.end() ); // make empty
	}
}

}

#endif
