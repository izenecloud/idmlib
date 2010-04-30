/*
 * NameEntityDict.h
 *
 *  Created on: Mar 31, 2010
 *      Author: eric
 */

#ifndef DM_NAMEENTITYDICT_H_
#define DM_NAMEENTITYDICT_H_

#include <ext/hash_set>


#ifndef MS_WIN
using __gnu_cxx::hash_set;
#ifndef GNU_HASH_
#define GNU_HASH_
namespace __gnu_cxx {
	template<>
	struct hash <std::string>
	{
	  size_t operator()( const std::string& x ) const
	  {
		return hash< const char* >()( x.c_str() );
	  }
	};
};
#endif
#endif

namespace idmlib
{


class NameEntityDict
{

public:
	static bool isLocSuffix(std::string& str);
	static bool isOrgSuffix(std::string& str);
	static bool isPeopSuffix(std::string& str);
	static bool isNamePrefix(std::string& str);


	static bool isU(std::string& str);
	static bool isQ(std::string& str);
	static bool isP(std::string& str);
	static bool isX(std::string& str);
	static bool isC(std::string& str);
	static bool isD(std::string& str);

	static void loadLocSuffix(std::string& path);
	static void loadOrgSuffix(std::string& path);
	static void loadPeopSuffix(std::string& path);
	static void loadNamePrefix(std::string& path);

	static void loadU(std::string& path);
	static void loadQ(std::string& path);
	static void loadP(std::string& path);
	static void loadX(std::string& path);
	static void loadC(std::string& path);
	static void loadD(std::string& path);


	static hash_set<std::string> locSuffix;
	static hash_set<std::string> orgSuffix;
	static hash_set<std::string> peopSuffix;
	static hash_set<std::string> namePrefix;
	static hash_set<std::string> U;
	static hash_set<std::string> Q;
	static hash_set<std::string> P;
	static hash_set<std::string> X;
	static hash_set<std::string> C;
	static hash_set<std::string> D;

private:
	static void loadSuffix(std::string& path, hash_set<std::string>& suffixSet);
};

}


#endif /* NAMEENTITYDICT_H_ */
