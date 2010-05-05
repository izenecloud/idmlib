/*
 * t_ner_test.cpp
 *
 *  Created on: Mar 30, 2010
 *      Author: eric
 */
#include "t_nec.h"
#include <idmlib/nec/NameEntity.h>
#include <idmlib/nec/NameEntityUtil.h>
#include <ml/Evaluator.h>
#include <idmlib/nec/NameEntityDict.h>
#include <idmlib/nec/NameEntityManager.h>
#include <fstream.h>
#include <set>

using namespace idmlib;


int main()
{

    std::ofstream testOut("necResult.txt");
	std::string test_path = "../db/nec/test/test.txt";
	std::string path = "../resource/nec/";
	NameEntityManager neMgr(path);

	cout << "loading entities..." << endl;
	std::vector<NameEntity> entities2;
	loadNameEntities(entities2, test_path, "NONE");
	cout << "#enties2: " << entities2.size() << endl;

	neMgr.loadModels();
	neMgr.predict(entities2);

	int posCount=0;
	for (size_t i=0; i<entities2.size(); ++i)
	{
//		cout << i+1 << "\t";
		testOut<<i+1 << "\t";
		string cur="", pre="", suc="";

		entities2[i].cur.convertString(cur, UString::UTF_8);

//		cout << cur << "\t";
		testOut<<cur << "\t";

		std::vector<Label> labels = entities2[i].predictLabels;
		if(labels.size()>0)
		{
			if(labels[0]==entities2[i].tagLabels[0])
				posCount++;
			else
				std::cout<<cur<<entities2[i].tagLabels[0]<<","<<labels[0]<<std::endl;
		}
		else if(entities2[i].tagLabels[0]=="OTHER")
			posCount++;
		else
		{
			std::cout<<cur<<entities2[i].tagLabels[0]<<","<<"NONE"<<std::endl;
		}
		for (size_t j=0; j<labels.size(); ++j)
		{
//			cout << labels[j] << " ";
			testOut<<labels[j] << " ";
		}
//		cout << endl;
		testOut<<std::endl;
	}
	std::cout<<"The classification rate: "<<(float)posCount/entities2.size()<<std::endl;

//    std::string path="/home/jinglei/199801.txt";
//    std::ifstream corpus(path.c_str());
//    std::ofstream adj("noun.txt");
//    std::string line;
//    std::string delimiters="  ";
//    std::set<std::string> dict;
//    while(std::getline(corpus, line))
//    {
//    	if(line.length()>0)
//    	{
//    		std::vector<std::string> tokens;
//    		tokenize(line, tokens, delimiters);
//    		for(size_t i=0;i<tokens.size();i++)
//    		{
//    			std::string strWord;
//    			std::string strLabel;
//    			wiselib::UString uToken(tokens[i], wiselib::UString::GB2312);
//    			std::string utfToken;
//    			uToken.convertString(utfToken, wiselib::UString::UTF_8);
//    			if(utfToken[utfToken.length()-1]==' ')
//    				utfToken=utfToken.substr(0, utfToken.length()-1);
//    			int pos=utfToken.find("/");
//    			if(pos!=std::string::npos)
//    			{
////    				if(utfToken.find("nr")==string::npos)
////    				{
//    				strWord=utfToken.substr(0, pos);
//    				strLabel=utfToken.substr(pos+1,utfToken.length()-pos-1);
//    				if(strLabel=="n")
//    				{
//                        if(dict.find(strWord)==dict.end())
//                        {
//                        	dict.insert(strWord);
//                        }
//
//    				}
////    				}
//    			}
//
//    		}
//    	}
//    }
//
//    std::set<std::string>::iterator iter=dict.begin();
//    for(;iter!=dict.end();iter++)
//    {
//    	adj<<*iter<<std::endl;
//    }

}
