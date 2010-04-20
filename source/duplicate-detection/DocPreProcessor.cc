/*
 * DocPreProcessor.cpp
 *
 *  Created on: 2008-12-12
 *      Author: jinglei
 */

#include <idmlib/duplicate-detection/DocPreProcessor.h>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <dirent.h>
using namespace std;

DocPreProcessor::DocPreProcessor(const string& strDir):_strDir(strDir) {
}

DocPreProcessor::~DocPreProcessor() {
	// TODO Auto-generated destructor stub
}

/**
 * Read files from a directory.
 */
void DocPreProcessor::ReadDir(const string &strDir,list<string>& listOut)
{
    DIR* dir;
    dir=opendir(strDir.c_str());
    if(dir==NULL){cout<<"Open Directory Error!"<<endl; exit(1);}
    struct dirent* ent;
    while((ent=readdir(dir))!=NULL)
         {
    	  char* cThis=ent->d_name;
    	  if(strcmp(cThis,".")==0||strcmp(cThis,"..")==0)
    			continue;
        string strThis(readfile(strDir.c_str(),cThis));
        listOut.push_back(strThis);
          }
    closedir(dir);
}
/**
 * Read a file according to the filenName.
 */
const char* DocPreProcessor::readfile(const char* dir, char *fileName)
{// load the content
	string strFilename(fileName);
   string strDir(dir);
   if(strDir[strDir.length()-1]!='/')
       {
       string strLast("/");
       strDir+=strLast;
        }
	strFilename=strDir+strFilename;
	char * buffer;
	long size;
	ifstream file (strFilename.c_str(), ios::in|ios::binary|ios::ate);
	size = file.tellg();
	file.seekg (0, ios::beg);
	buffer = new char [size];
	file.read (buffer, size);
	file.close();
   return buffer;
}

bool DocPreProcessor::getDocs(wiselib::DynamicArray<wiselib::YString>& docNames, wiselib::DynamicArray<wiselib::YString>& docContents)
{
	list<string> strFileList;
	ReadDir(_strDir,strFileList);
    map<string,string> mapIDContent;//map a doc ID to its content.
    list<string>::iterator iter=strFileList.begin();
    int count=1;
    for(;iter!=strFileList.end();iter++){
    	//parseFile(*iter,mapIDContent);
    	genSynFile(*iter,mapIDContent,count);
     }
//    map<string,string>::iterator iterMap=mapIDContent.begin();
//    docNames.reserve(20000);
//    docContents.reserve(20000);
//    for(;iterMap!=mapIDContent.end();iterMap++)
//    {
//    	YString strID(iterMap->first);
//    	docNames.push_back(strID);
//    	YString strContent(iterMap->second);
//    	docContents.push_back(strContent);
//    }
//    mapIDContent.clear();
//    cout<<"docNum:"<<docNames.size()<<endl;
//    cout<<"DocNum:"<<docContents.size()<<endl;
      map<string,string>::iterator iterMap=mapIDContent.begin();
      for(;iterMap!=mapIDContent.end();iterMap++)
       {
    	  string fDocName="/home/jinglei/workspace/dup-db/syndb/";
    	  fDocName+=iterMap->first;
    	  ofstream fDoc(fDocName.c_str());
    	  fDoc<<iterMap->second;
    	  fDoc.close();
       }

    return true;
}

void DocPreProcessor::parseFile(const string& strFile, map<string,string>&mapIDContent)
{
	size_t index=0;
	while((index=strFile.find("<DOCNO>",index))!=string::npos)
	{
		int beginID=strFile.find(" ",index); //must be there.
		int endID=strFile.find(" ",beginID+1);
		string strID=strFile.substr(beginID+1,endID-beginID-1);
		int endContent=strFile.find("</DOC>",index);
		string strContent=strFile.substr(endID+1,endContent-endID-1);
		//Eliminate meta SGML tags.
		cutMetaTag(strContent);
		//Normalize it.
		string strNormal;
		normalize(strContent,strNormal);
		mapIDContent.insert(make_pair(strID,strNormal));
		cout<<"NO:"<<strID<<endl;
		index=endContent;
	}
}

void DocPreProcessor::genSynFile(const string& strFile, map<string,string>&mapIDContent,int& count)
{
	size_t index=0;
	while((index=strFile.find("<DOCNO>",index))!=string::npos)
	{
		int beginID=strFile.find(" ",index); //must be there.
		int endID=strFile.find(" ",beginID+1);
		string strID=strFile.substr(beginID+1,endID-beginID-1);
		int endContent=strFile.find("</DOC>",index);
		string strContent=strFile.substr(endID+1,endContent-endID-1);
		//Eliminate meta SGML tags.
		cutMetaTag(strContent);
		//Normalize it.
		string strNormal;
		normalize(strContent,strNormal);
		if(count==1)
		{
			tokenize(strNormal,_vecTerm);
		}
		if(count%611==0){
		mapIDContent.insert(make_pair(strID,strNormal));
		cout<<"NO:"<<strID<<endl;
		string strSyn;
		synDupGenerate(strNormal,strSyn);
		string strSynID=strID+"-syn";
		mapIDContent.insert(make_pair(strSynID,strSyn));
		cout<<"NO:"<<strSynID<<endl;
		}
		index=endContent;
		count++;
	}
}
void DocPreProcessor::cutMetaTag(string& strContent)
{
	size_t index=0;
	while((index=strContent.find_first_of("<",index))!=string::npos)
	{
		int endTag=strContent.find_first_of(">",index+1);
		strContent.replace(index,endTag-index+1," ");
		//strContent.erase(index,endTag-index+1);
	}
}

void DocPreProcessor::normalize(const string& str, string& tokens, const string& delimiters)
{
	tokens.clear();
	// Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos=str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos)
    {
    	//Found a token, add it to the vector.
    	string strToken=str.substr(lastPos, pos-lastPos);
		for(int i=0;i<(int)strToken.length();i++)
		{
			strToken[i]=tolower(strToken[i]);
        }
		strToken+=" ";
		tokens+=strToken;
		// Skip delimiters.
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
     }
}

void DocPreProcessor::tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
	tokens.clear();
	// Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos=str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos)
    {
    	//Found a token, add it to the vector.
    	string strToken=str.substr(lastPos, pos-lastPos);
		for(int i=0;i<(int)strToken.length();i++)
		{
			strToken[i]=tolower(strToken[i]);
        }
		tokens.push_back(strToken);
	  	// Skip delimiters.
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
     }
}

void DocPreProcessor::synDupGenerate(const string& strOrigDoc, string& strSynDupDoc)
{
	srand((unsigned)time(0));
	std::vector<string> vecTokens;
	tokenize(strOrigDoc,vecTokens);
	//the random generated positions where term changes.
	set<size_t> setRandPos;
	while(setRandPos.size()<10)
	{
		int randPos=rand()%vecTokens.size();
		setRandPos.insert(randPos);
	}
	set<size_t> setDelPos; //positions to be delete
	set<size_t> setAddPos; //positions to be add
	for(set<size_t>::iterator iterSet=setRandPos.begin();iterSet!=setRandPos.end();iterSet++)
	{

		int randNum=rand()%4;
		switch(randNum){
		case 0://nothing done
			break;
		case 1://delete the term
			setDelPos.insert(*iterSet);
			break;
		case 2://Replace the term with one from term list;
		{
			int randTermPos=rand()%(_vecTerm.size());
			vecTokens[*iterSet]=_vecTerm[randTermPos];
		}
			break;
		case 3://add a term from term list;
			setAddPos.insert(*iterSet);
		}
	}
	strSynDupDoc.clear();
	for(size_t i=0;i<vecTokens.size();i++)
	{
		if(setDelPos.find(i)==setDelPos.end())
		{
			if(setAddPos.find(i)!=setAddPos.end())
			{
				int randTermPos=rand()%(_vecTerm.size());
				strSynDupDoc=strSynDupDoc+_vecTerm[randTermPos]+" ";
			}
			strSynDupDoc=strSynDupDoc+vecTokens[i]+" ";
		}

	}

}

