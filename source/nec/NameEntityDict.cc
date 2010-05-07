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
hash_set<std::string> NameEntityDict::noiseList_;
hash_set<std::string> NameEntityDict::otherList_;
hash_set<std::string> NameEntityDict::nounList_;
hash_set<std::string> NameEntityDict::locLeft_;
hash_set<std::string> NameEntityDict::locRight_;
hash_set<std::string> NameEntityDict::orgLeft_;
hash_set<std::string> NameEntityDict::orgRight_;
hash_set<std::string> NameEntityDict::peopLeft_;
hash_set<std::string> NameEntityDict::peopRight_;

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

void NameEntityDict::loadNoiseList( std::string& path)
{
	loadSuffix(path, noiseList_);
}

void NameEntityDict::loadOtherList( std::string& path)
{
	loadSuffix(path, otherList_);
}

void NameEntityDict::loadNounList( std::string& path)
{
	loadSuffix(path, nounList_);
}


void NameEntityDict::loadLocLeft(std::string& path)
{
	loadSuffix(path, locLeft_);
}

void NameEntityDict::loadLocRight( std::string& path)
{
	loadSuffix(path, locRight_);
}

void NameEntityDict::loadOrgLeft( std::string& path)
{
	loadSuffix(path, orgLeft_);
}

void NameEntityDict::loadOrgRight( std::string& path)
{
	loadSuffix(path, orgRight_);
}

void NameEntityDict::loadPeopLeft( std::string& path)
{
	loadSuffix(path, peopLeft_);
}

void NameEntityDict::loadPeopRight( std::string& path)
{
	loadSuffix(path, peopRight_);
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

bool NameEntityDict::isKownNoise( std::string& str)
{
	return noiseList_.find(str) != noiseList_.end();
}

bool NameEntityDict::isKownOther( std::string& str)
{
	return otherList_.find(str) != otherList_.end();
}

bool NameEntityDict::isNoun( std::string& str)
{
	return nounList_.find(str) != nounList_.end();
}

bool NameEntityDict::isLocLeft(std::string& str)
{
	return locLeft_.find(str) != locLeft_.end();
}

bool NameEntityDict::isLocRight(std::string& str)
{
	return locRight_.find(str) != locRight_.end();
}

bool NameEntityDict::isOrgLeft(std::string& str)
{
	return orgLeft_.find(str) != orgLeft_.end();
}

bool NameEntityDict::isOrgRight( std::string& str)
{
	return orgRight_.find(str) != orgRight_.end();
}

bool NameEntityDict::isPeopLeft( std::string& str)
{
	return peopLeft_.find(str) != peopLeft_.end();
}

bool NameEntityDict::isPeopRight( std::string& str)
{
	return peopRight_.find(str) != peopRight_.end();
}

}
