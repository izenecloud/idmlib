/*
 * t_qs.cpp
 *
 *  Created on: 2010-4-26
 *      Author: jinglei
 */

#include <idmlib/query-suggestion/Reminder.h>
#include "qs_types.h"

#include <sdb/SequentialDB.h>

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/unordered_map.hpp>

using namespace idmlib;


const uint32_t MAX_READ_BUF_SIZE=1024;
const uint32_t MAX_TIME_SERIRES=7;
const uint32_t REAL_NUM=10;
const uint32_t POP_NUM=20;


void parseFile(const std::string& fileName, std::list<std::pair<izenelib::util::UString,uint32_t> >& logItems)
{
    fstream fileIn(fileName.c_str(), ios::in | ios::binary);
    if (!fileIn.good())
    {
        return;
    }
    boost::unordered_map<std::string, uint32_t> queryFreqMap;
	while(!fileIn.eof())
	{
		string strLine;
		getline(fileIn,strLine);
		int posBegin=strLine.find("[");
		int posEnd=strLine.find("]");
		std::string strQuery=strLine.substr(posBegin+1, posEnd-posBegin-1);
		queryFreqMap[strQuery]+=1;
	}
	boost::unordered_map<std::string, uint32_t>::iterator iter=queryFreqMap.begin();
	for(;iter!=queryFreqMap.end();iter++)
	{
		izenelib::util::UString ustrQuery(iter->first, izenelib::util::UString::GB2312);
//		izenelib::util::UString ustrQuery(iter->first, izenelib::util::UString::UTF_8);
		std::pair<izenelib::util::UString, uint32_t> item(ustrQuery, iter->second);
		logItems.push_back(item);
	}
}

void parseTosf1v5LogFile(const std::string& fileName, const std::string& logFileName)
{
    std::ifstream fileIn(fileName.c_str());
    if (!fileIn.good())
    {
        return;
    }
    std::ofstream fileOut(logFileName.c_str());
    string strLine;
    while( getline(fileIn,strLine) )
    {
//         std::cout<<"Have "<<strLine<<std::endl;
        
        std::vector<std::string> items;
        boost::algorithm::split( items, strLine, boost::algorithm::is_any_of("\t") );
//         std::cout<<"size "<<items.size()<<" "<<items[2]<<std::endl;
        if(items.size()!=4) continue;
        std::vector<std::string> subItems;
        boost::algorithm::split( subItems, items[2], boost::algorithm::is_any_of(" ") );
        if( subItems.size()!=2 ) continue;
        if( subItems[1] != "1" ) continue;
        std::string outputLine = "Query\t";
        outputLine += items[0] + "\t20100517T151422\t00:00:01.027035\t";
        std::string query = items[1];
        query = query.substr(1, query.length()-2);
        izenelib::util::UString uquery(query, izenelib::util::UString::GB2312);
        std::string tmp;
        uquery.convertString(tmp, izenelib::util::UString::UTF_8);
//         std::cout<<"append "<<tmp<<std::endl;
        
        
        outputLine += tmp + "\t100\t0\t10";
        fileOut<<outputLine<<std::endl;
    }
    fileIn.close();
    fileOut.close();

}

void convertLog(const std::string& sogouPath, const std::string& objPath)
{
//    boost::filesystem::create_directories(objPath);
//    boost::posix_time::ptime time = boost::posix_time::second_clock::local_time();
//    boost::gregorian::days oneday(1);
//    for(uint32_t i=20060831;i>=20060801;i--)
//    {
//        std::string fileName = sogouPath+"/"+"access_log."+boost::lexical_cast<std::string>(i)+".decode.filter";
//        if( !boost::filesystem::exists(fileName) )
//        {
//            continue;
//        }
//        std::string dateTimeStr = boost::posix_time::to_iso_string(time).substr(0,8);
//        std::string outputFileName = objPath+"/"+dateTimeStr+".log";
//        std::cout<<"Converting "<<fileName<<" to "<<outputFileName<<std::endl;
//        parseTosf1v5LogFile( fileName, outputFileName);
//        time -= oneday;
//    }
}

void convertLog2(const std::string& testFile, const std::string& objPath)
{
//    boost::filesystem::create_directories(objPath);
//    std::ifstream fileIn(testFile.c_str());
//    boost::posix_time::ptime time = boost::posix_time::second_clock::local_time();
//    boost::gregorian::days oneday(1);
//    string strLine;
//    std::ofstream* pOfs = NULL;
//    while( getline(fileIn,strLine) )
//    {
//        if( strLine.length()==0 ) continue;
//        std::string dateTimeStr = boost::posix_time::to_iso_string(time).substr(0,8);
//        if( strLine[0] == '#' )
//        {
//            if( pOfs!= NULL )
//            {
//                pOfs->close();
//                delete pOfs;
//            }
//
//            std::string outputFileName = objPath+"/"+dateTimeStr+".log";
//            pOfs = new std::ofstream(outputFileName.c_str() );
//            time -= oneday;
//        }
//        else
//        {
//            std::string outputLine = "Query\t";
//            outputLine += "session\t"+dateTimeStr+"T151422\t00:00:01.027035\t";
//            outputLine += strLine + "\t100\t0\t10";
//            (*pOfs)<<outputLine<<std::endl;
//        }
//
//    }
//    if( pOfs!= NULL )
//    {
//        pOfs->close();
//        delete pOfs;
//    }
//    fileIn.close();
}

//void parseDirectory(const std::string& path, std::list<std::pair<izenelib::util::UString,int> >& logItems)
//{
//    boost::filesystem::directory_iterator item_begin(path);
//    boost::filesystem::directory_iterator item_end;
//    boost::unordered_map<std::string, int> queryFreqMap;
//    int count=0;
//    for (; item_begin != item_end&&count<MAX_TIME_SERIRES; item_begin++, count++)
//    {
//      	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
//        if (boost::filesystem::is_directory(*item_begin))
//            continue;
//        std::list<std::pair<izenelib::util::UString,int> > tempItems;
//        parseFile(item_begin -> path().file_string(), tempItems);
//        std::list<std::pair<izenelib::util::UString,int> >::iterator it=tempItems.begin();
//        for(;it!=tempItems.end();it++)
//        {
//        	std::string strItem;
//        	it->first.convertString(strItem,izenelib::util::UString::GB2312);
//        	queryFreqMap[strItem]+=it->second;
//        }
//    }
//
//	boost::unordered_map<std::string, int>::iterator iter=queryFreqMap.begin();
//	for(;iter!=queryFreqMap.end();iter++)
//	{
//		izenelib::util::UString ustrQuery(iter->first, izenelib::util::UString::GB2312);
//		std::pair<izenelib::util::UString, int> item(ustrQuery, iter->second);
//		logItems.push_back(item);
//	}
//}


bool test()
{
	std::cout<<"Test for query suggestion!"<<std::endl;
    std::string path("/home/jinglei/sohuq");
    std::string basicPath(".");
	TestIDManager* idManager= new TestIDManager(".");
    Reminder<TestIDManager> reminder(idManager, basicPath, REAL_NUM, POP_NUM);

	//Control the experiment log slice number.
	uint32_t count=0;
    try
    {
        boost::filesystem::directory_iterator item_begin(path);
        boost::filesystem::directory_iterator item_end;
        uint32_t timeId=1;
        std::vector<std::string> fileNames;
        for (;item_begin != item_end&& count<MAX_TIME_SERIRES;item_begin++, count++)
        {
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
            std::list<std::pair<izenelib::util::UString,uint32_t> > logItems;
            parseFile(fileNames[i], logItems);
            if(i==fileNames.size()-1)
            	reminder.indexQueryLog(timeId, logItems, true);
            else
            	reminder.indexQueryLog(timeId, logItems);
            timeId++;
        }
    } catch (...)
    {
        std::cout << "Error : log built fail.\n";
    }

    reminder.indexQuery();
    std::vector<izenelib::util::UString> realItems;
    reminder.getRealTimeQuery(realItems);
    std::cout<<"realItems.size()"<<realItems.size()<<std::endl;
    for(uint32_t i=0;i<realItems.size();i++)
    {
    	realItems[i].displayStringValue(izenelib::util::UString::UTF_8);
    	std::cout<<std::endl;
    }

    std::vector<izenelib::util::UString> popularItems;
    reminder.getPopularQuery(popularItems);
    std::cout<<"popularItems.size()"<<popularItems.size()<<std::endl;
    for(uint32_t i=0;i<popularItems.size();i++)
    {
    	popularItems[i].displayStringValue(izenelib::util::UString::UTF_8);
    	std::cout<<std::endl;
    }

    if(idManager)
    	delete idManager;

	return 0;

}

bool testPopularQuery()
{
//	std::cout<<"Test for query suggestion!"<<std::endl;
//    std::string path("/home/jinglei/sohuq");
//    std::string basicPath(".");
//    Recommender recommender(basicPath);
//
//	//Control the experiment log slice number.
//	int count=0;
//    try
//    {
//
//    	std::cout<<"entering inside"<<std::endl;
//        boost::filesystem::directory_iterator item_begin(path);
//        boost::filesystem::directory_iterator item_end;
//        uint32_t timeId=1;
//        for (;item_begin != item_end&& count<MAX_TIME_SERIRES;item_begin++, count++)
//        {
//        	std::cout<<"parsing file"<<item_begin -> path().file_string()<<std::endl;
//            if (boost::filesystem::is_directory(*item_begin))
//                continue;
//            std::list<std::pair<izenelib::util::UString,int> > logItems;
//            parseFile(item_begin -> path().file_string(), logItems);
//            recommender.indexLog(timeId, logItems);
//            timeId++;
//        }
//        std::list<std::pair<izenelib::util::UString,int> > allLogItems;
//        parseDirectory(path, allLogItems);
//        recommender.indexPopularQuery(allLogItems);
//    } catch (...)
//    {
//        std::cout << "Error : log built fail.\n";
//    } // end - try-catch
//
//    std::vector<izenelib::util::UString> popularItems;
//    recommender.getPopularQuery(popularItems);
//    for(uint32_t i=0;i<popularItems.size();i++)
//    {
//    	popularItems[i].displayStringValue(izenelib::util::UString::UTF_8);
//    	std::cout<<std::endl;
//    }


	return 0;

}

int main()
{
    convertLog2("/home/jarvis/projects/sf1-revolution/test-logs","/home/jarvis/projects/sf1-revolution/test-logs-dir");
//     convertLog("/home/jarvis/data/SogouQ", "/home/jarvis/data/SogouQ/sf1v5");
//	testRealTimeQuery();
//	testPopularQuery();
// 	test();
	return 0;

}
