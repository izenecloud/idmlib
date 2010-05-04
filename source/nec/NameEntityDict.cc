/*
 * NameEntityDict.cc
 *
 *  Created on: Mar 31, 2010
 *      Author: eric
 */

#include <iostream>
#include <fstream>
#include <string>
#include <idmlib/nec/NameEntityDict.h>

namespace idmlib
{

hash_set<std::string> NameEntityDict::locSuffix;
hash_set<std::string> NameEntityDict::orgSuffix;
hash_set<std::string> NameEntityDict::peopSuffix;
hash_set<std::string> NameEntityDict::namePrefix;
hash_set<std::string> NameEntityDict::locList_;
hash_set<std::string> NameEntityDict::orgList_;
hash_set<std::string> NameEntityDict::peopList_;

void NameEntityDict::loadSuffix(std::string& path, hash_set<std::string>& suffixSet)
{
	if (suffixSet.size() == 0)
		{
			std::ifstream inStream(path.c_str());
			if (!inStream)
			{
				return;
			}
			std::string line;
			while(std::getline(inStream, line))
			{
				if (line.length() > 0)
				{
					suffixSet.insert(line);
				}
			}
			inStream.close();
		}
}

void NameEntityDict::loadLocSuffix(std::string& path)
{
	loadSuffix(path, locSuffix);
}


void NameEntityDict::loadPeopSuffix(std::string& path)
{
	loadSuffix(path, peopSuffix);
}


void NameEntityDict::loadOrgSuffix(std::string& path)
{
	loadSuffix(path, orgSuffix);
}

void NameEntityDict::loadNamePrefix(std::string& path)
{
	loadSuffix(path, namePrefix);
}

void NameEntityDict::loadLocList(std::string& path)
{
	loadSuffix(path, locList_);
}

void NameEntityDict::loadPeopList( std::string& path)
{
	loadSuffix(path, peopList_);
}

void NameEntityDict::loadOrgList( std::string& path)
{
	loadSuffix(path, orgList_);
}

bool NameEntityDict::isLocSuffix(std::string& str)
{
	return locSuffix.find(str) != locSuffix.end();
}

bool NameEntityDict::isOrgSuffix(std::string& str)
{
	return orgSuffix.find(str) != orgSuffix.end();
}

bool NameEntityDict::isPeopSuffix(std::string& str)
{
	return peopSuffix.find(str) != peopSuffix.end();
}

bool NameEntityDict::isNamePrefix(std::string& str)
{
	return namePrefix.find(str) != namePrefix.end();
}

bool NameEntityDict::isKownLoc(std::string& str)
{
	return locList_.find(str) != locList_.end();
}

bool NameEntityDict::isKownPeop(std::string& str)
{
	return peopList_.find(str) != peopList_.end();
}

bool NameEntityDict::isKownOrg( std::string& str)
{
	return orgList_.find(str) != orgList_.end();
}

}
