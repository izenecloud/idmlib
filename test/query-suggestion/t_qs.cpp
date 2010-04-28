/*
 * t_qs.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/LogProcessor.h>
#include <idmlib/query-suggestion/Recommender.h>

#include <sdb/SequentialDB.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <wiselib/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

using namespace idmlib;


const int MAX_READ_BUF_SIZE=1024;


void parseFile(const std::string& fileName, std::list<std::pair<wiselib::UString,int> >& logItems)
{
    fstream fileIn(fileName.c_str(), ios::in | ios::binary);
    if (!fileIn.good())
    {
        return;
    }
    boost::unordered_map<std::string, int> queryFreqMap;
	while(!fileIn.eof())
	{
		string strLine;
		getline(fileIn,strLine);
		int posBegin=strLine.find("[");
		int posEnd=strLine.find("]");
		std::string strQuery=strLine.substr(posBegin+1, posEnd-posBegin-1);
		queryFreqMap[strQuery]+=1;
	}
	boost::unordered_map<std::string, int>::iterator iter=queryFreqMap.begin();
	for(;iter!=queryFreqMap.end();iter++)
	{
		wiselib::UString ustrQuery(iter->first, wiselib::UString::GB2312);
		std::pair<wiselib::UString, int> item(ustrQuery, iter->second);
		logItems.push_back(item);
	}
}

int main()
{
	std::cout<<"Test for query suggestion!"<<std::endl;
    std::string path("/home/jinglei/sohuq");
    std::string basicPath(".");
    Recommender recommender(basicPath);

	//Control the experiment log slice number.
	int count=0;
    try
    {

    	std::cout<<"entering inside"<<std::endl;
        boost::filesystem::directory_iterator item_begin(path);
        boost::filesystem::directory_iterator item_end;
        for (;item_begin != item_end&& count<10;item_begin++, count++)
        {
        	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
            if (boost::filesystem::is_directory(*item_begin))
                continue;
            std::list<std::pair<wiselib::UString,int> > logItems;
            parseFile(item_begin -> path().file_string(), logItems);
            recommender.indexLog(logItems);
            std::vector<wiselib::UString> realItems;
            recommender.getRealTimeQuery(realItems);
            for(uint32_t i=0;i<realItems.size();i++)
            {
            	realItems[i].displayStringValue(wiselib::UString::UTF_8);
            	std::cout<<std::endl;
            }
        }
    } catch (...)
    {
        std::cout << "Error : log built fail.\n";
    } // end - try-catch


	return 0;
}
