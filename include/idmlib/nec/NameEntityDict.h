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

	static bool isKownNoise(std::string& path);
	static bool isKownLoc( std::string& str);
	static bool isKownPeop(std::string& str);
	static bool isKownOrg(std::string& str);
	static bool isKownOther(std::string& str);
	static bool isNoun(std::string& str);

	static bool isLocLeft(std::string& path);
	static bool isLocRight( std::string& str);
	static bool isOrgLeft(std::string& str);
	static bool isOrgRight(std::string& str);
	static bool isPeopLeft(std::string& str);
	static bool isPeopRight(std::string& str);


	static void loadLocSuffix(std::string& path);
	static void loadOrgSuffix(std::string& path);
	static void loadPeopSuffix(std::string& path);
	static void loadNamePrefix(std::string& path);

	static void loadLocList(std::string& path);
	static void loadPeopList(std::string& path);
	static void loadOrgList(std::string& path);
	static void loadNoiseList(std::string& path);
	static void loadOtherList(std::string& path);
	static void loadNounList(std::string& path);

	static void loadLocLeft(std::string& path);
	static void loadLocRight(std::string& path);
	static void loadOrgLeft(std::string& path);
	static void loadOrgRight(std::string& path);
	static void loadPeopLeft(std::string& path);
	static void loadPeopRight(std::string& path);


	static hash_set<std::string> locSuffix;
	static hash_set<std::string> orgSuffix;
	static hash_set<std::string> peopSuffix;
	static hash_set<std::string> namePrefix;

	static hash_set<std::string> locList_;
	static hash_set<std::string> peopList_;
	static hash_set<std::string> orgList_;
	static hash_set<std::string> noiseList_;
	static hash_set<std::string> otherList_;
	static hash_set<std::string> nounList_;

	static hash_set<std::string> locLeft_;
	static hash_set<std::string> locRight_;
	static hash_set<std::string> orgLeft_;
	static hash_set<std::string> orgRight_;
	static hash_set<std::string> peopLeft_;
	static hash_set<std::string> peopRight_;


private:
	static void loadSuffix(std::string& path, hash_set<std::string>& suffixSet);
};

}


#endif /* NAMEENTITYDICT_H_ */
