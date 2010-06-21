/**
* Name        : UtilTool.cpp
* Author      : Jinglei Zhao
* Version     : v1.0
* Copyright   : iZENESoft
* Description : Set the parameters for all the components in the RankingMangaer.
*/

#include <idmlib/duplicate-detection/DupDetector.h>
#include <idmlib/duplicate-detection/UtilTool.h>
#include <idmlib/duplicate-detection/UtilProperty.h>

#include <iostream>
#include <fstream>
#include <map>
#include <string>

using namespace std;

UtilTool::UtilTool() {
	// TODO Auto-generated constructor stub

}

UtilTool::~UtilTool() {

}

void UtilTool::readDupProperty()
{
	ifstream fileIn("./config/dup.property");
	try{
		if(fileIn.fail())
		throw "txt";
	}
	catch(char* s)
	{
		cout<<"NOTICE: no dup.property"<<endl;
		return;
    }
    map<string,string> mapProperties;
	UtilProperty::read(fileIn, mapProperties);
	map<string,string>::iterator iterMap;
	char* stopstring;
	if((iterMap=mapProperties.find("dup_alg"))!=mapProperties.end()&&iterMap->second!="")
	{
		int nType=strtol(iterMap->second.c_str(),&stopstring,10);
		if(!(nType==0||nType==1||nType==2))
		{
			cout<<"Error in setting parameter dup_alg. It could not be "<<nType<<endl;
			exit(1);
		}
		else
          sf1v5::DupDetector::_alg=(sf1v5::DUP_ALG)nType;
	}
}
