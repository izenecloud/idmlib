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
#include <algorithm>

#include <wiselib/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

using namespace idmlib;


const int MAX_READ_BUF_SIZE=1024;
const int MAX_TIME_SERIRES=31;


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

void parseDirectory(const std::string& path, std::list<std::pair<wiselib::UString,int> >& logItems)
{
    boost::filesystem::directory_iterator item_begin(path);
    boost::filesystem::directory_iterator item_end;
    boost::unordered_map<std::string, int> queryFreqMap;
    int count=0;
    for (; item_begin != item_end&&count<MAX_TIME_SERIRES; item_begin++, count++)
    {
      	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
        if (boost::filesystem::is_directory(*item_begin))
            continue;
        std::list<std::pair<wiselib::UString,int> > tempItems;
        parseFile(item_begin -> path().file_string(), tempItems);
        std::list<std::pair<wiselib::UString,int> >::iterator it=tempItems.begin();
        for(;it!=tempItems.end();it++)
        {
        	std::string strItem;
        	it->first.convertString(strItem,wiselib::UString::GB2312);
        	queryFreqMap[strItem]+=it->second;
        }
    }

	boost::unordered_map<std::string, int>::iterator iter=queryFreqMap.begin();
	for(;iter!=queryFreqMap.end();iter++)
	{
		wiselib::UString ustrQuery(iter->first, wiselib::UString::GB2312);
		std::pair<wiselib::UString, int> item(ustrQuery, iter->second);
		logItems.push_back(item);
	}
}


bool testRealTimeQuery()
{
	std::cout<<"Test for query suggestion!"<<std::endl;
    std::string path("/home/jinglei/sohuq");
    std::string basicPath(".");
    Recommender recommender(basicPath);

	//Control the experiment log slice number.
	int count=0;
    try
    {

//    	std::cout<<"entering inside"<<std::endl;
        boost::filesystem::directory_iterator item_begin(path);
        boost::filesystem::directory_iterator item_end;
        uint32_t timeId=1;
        std::vector<std::string> fileNames;
        for (;item_begin != item_end&& count<MAX_TIME_SERIRES;item_begin++, count++)
        {
//        	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
            if (boost::filesystem::is_directory(*item_begin))
                continue;
            else
            {
            	std::string tempFileName((item_begin -> path().file_string()));
            	fileNames.push_back(tempFileName);
            }
        }
        std::sort(fileNames.begin(), fileNames.end());
        for (size_t i=0;i<fileNames.size();i++, count++)
        {
        	std::cout<<fileNames[i]<<std::endl;
            std::list<std::pair<wiselib::UString,int> > logItems;
            parseFile(fileNames[i], logItems);
            recommender.indexLog(timeId, logItems);
            recommender.indexRealTimeQuery(logItems);
            std::vector<wiselib::UString> realItems;
            recommender.getRealTimeQuery(realItems);
            std::cout<<"realItems.size()"<<realItems.size()<<std::endl;
            for(uint32_t i=0;i<realItems.size();i++)
            {
            	realItems[i].displayStringValue(wiselib::UString::UTF_8);
            	std::cout<<std::endl;
            }
            timeId++;
        }
    } catch (...)
    {
        std::cout << "Error : log built fail.\n";
    } // end - try-catch


	return 0;

}

bool testPopularQuery()
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
        uint32_t timeId=1;
        for (;item_begin != item_end&& count<MAX_TIME_SERIRES;item_begin++, count++)
        {
        	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
            if (boost::filesystem::is_directory(*item_begin))
                continue;
            std::list<std::pair<wiselib::UString,int> > logItems;
            parseFile(item_begin -> path().file_string(), logItems);
            recommender.indexLog(timeId, logItems);
            timeId++;
        }
        std::list<std::pair<wiselib::UString,int> > allLogItems;
        parseDirectory(path, allLogItems);
        recommender.indexPopularQuery(allLogItems);
    } catch (...)
    {
        std::cout << "Error : log built fail.\n";
    } // end - try-catch

    std::vector<wiselib::UString> popularItems;
    recommender.getPopularQuery(popularItems);
    for(uint32_t i=0;i<popularItems.size();i++)
    {
    	popularItems[i].displayStringValue(wiselib::UString::UTF_8);
    	std::cout<<std::endl;
    }


	return 0;

}

int main()
{
	testRealTimeQuery();
//	testPopularQuery();
	return 0;

}
