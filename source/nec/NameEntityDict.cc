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

hash_set<std::string> NameEntityDict::U;
hash_set<std::string> NameEntityDict::Q;
hash_set<std::string> NameEntityDict::P;
hash_set<std::string> NameEntityDict::X;
hash_set<std::string> NameEntityDict::C;
hash_set<std::string> NameEntityDict::D;


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

void NameEntityDict::loadU(std::string& path)
{
	loadSuffix(path, U);
}

void NameEntityDict::loadQ(std::string& path)
{
	loadSuffix(path, Q);
}

void NameEntityDict::loadP(std::string& path)
{
	loadSuffix(path, P);
}


void NameEntityDict::loadX(std::string& path)
{
	loadSuffix(path, X);
}

void NameEntityDict::loadC(std::string& path)
{
	loadSuffix(path, C);

}


void NameEntityDict::loadD(std::string& path)
{

	loadSuffix(path, D);

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

bool NameEntityDict::isU(std::string& str)
{
	return U.find(str) != U.end();
}

bool NameEntityDict::isQ(std::string& str)
{
	return Q.find(str) != Q.end();
}

bool NameEntityDict::isP(std::string& str)
{
	return P.find(str) != P.end();
}


bool NameEntityDict::isX(std::string& str)
{
	return X.find(str) != X.end();
}

bool NameEntityDict::isC(std::string& str)
{
	return C.find(str) != C.end();
}

bool NameEntityDict::isD(std::string& str)
{
	return D.find(str) != D.end();
}





}
