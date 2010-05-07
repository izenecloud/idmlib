#include <idmlib/nec/NameEntity.h>
#include <idmlib/util/StringUtil.hpp>
#include <iostream>
#include <string>

using namespace idmlib;
using namespace wiselib;


//
//void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
//{
//	// Skip delimiters at beginning.
//    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
//    // Find first "non-delimiter".
//    std::string::size_type pos=str.find_first_of(delimiters, lastPos);
//    while (std::string::npos != pos || std::string::npos != lastPos)
//	{
//		std::string strToken=str.substr(lastPos, pos-lastPos);
//		if(strToken!="") tokens.push_back(strToken);
//        // Skip delimiters.
//        lastPos = str.find_first_not_of(delimiters, pos);
//        // Find next "non-delimiter"
//        pos = str.find_first_of(delimiters, lastPos);
//	}
//}
//
//void trimLeft(std::string& str, const char* chars2remove)
//{
//	if (!str.empty())
//	{
//		std::string::size_type pos = str.find_first_not_of(chars2remove);
//		if (pos != std::string::npos)
//		    str.erase(0,pos);
//		else
//		    str.erase( str.begin() , str.end() ); // make empty
//	}
//}
//
//void trimRight(std::string& str, const char* chars2remove)
//{
//	if (!str.empty())
//	{
//		std::string::size_type pos = str.find_last_not_of(chars2remove);
//		if (pos != std::string::npos)
//		    str.erase(pos+1);
//		else
//		    str.erase( str.begin() , str.end() ); // make empty
//	}
//}

//void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
//{
//	ifstream inStream(file.c_str());
//	string line = "";
//	while (std::getline(inStream, line))
//	{
//		if (line.length() > 0)
//		{
////			int pos1 = line.find('\t', 0);
////			if (pos1 != -1)
////			{
////				int pos2 = line.find('\t', pos1+1);
////				if (pos2 != -1)
////				{
//					string cur = line;
////					string pre = line.substr(0, pos1);
////					string cur = line.substr(pos1+1, pos2-pos1-1);
////					string suc = line.substr(pos2+1, line.length() - pos2-1);
////					cout << pre << "\t" << cur << "\t" << suc << endl;
//					NameEntity entity;
//					entity.cur = UString(cur, UString::UTF_8);
////					entity.pre = UString(pre, UString::UTF_8);
////					entity.suc = UString(suc, UString::UTF_8);
//					entity.tagLabels.push_back(label);
//					entities.push_back(entity);
////				}
////			}
//		}
//	}
//	inStream.close();
//
//}

//void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
//{
//	ifstream inStream(file.c_str());
//	string line = "";
//	while (std::getline(inStream, line))
//	{
//		if (line.length() > 0)
//		{
//			int posEnd=line.find(",");
//			string cur = line.substr(0, posEnd);
//			NameEntity entity;
//			entity.cur = UString(cur, UString::UTF_8);
//			entity.tagLabels.push_back(label);
//			entities.push_back(entity);
//		}
//	}
//	inStream.close();
//
//}

//void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
//{
//	ifstream inStream(file.c_str());
//	string line = "";
//	while (std::getline(inStream, line))
//	{
//		if (line.length() > 0)
//		{
//			int posEnd=line.find(",");
//			string cur = line.substr(0, posEnd);
//			string strlabel=line.substr(posEnd+1, line.size()-posEnd-1);
//			std::cout<<cur<<","<<strlabel<<std::endl;
//			NameEntity entity;
//			entity.cur = UString(cur, UString::UTF_8);
//			entity.tagLabels.push_back(strlabel);
//			entities.push_back(entity);
//		}
//	}
//	inStream.close();
//
//}

///Load with context.
void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
{
//	ofstream testOut("nec_test_out.txt");
	ifstream inStream(file.c_str());
	string line ="";
	string blank=" ";
	while (std::getline(inStream, line))
	{
		if (line.length() > 0)
		{
			NameEntity entity;
			std::vector<std::string> tokens;
			tokenize(line, tokens, " ");
			for(size_t i=0;i<tokens.size();i++)
			{
				trimLeft(tokens[i], blank.c_str());
				trimRight(tokens[i], blank.c_str());
				if(i==0)
					entity.cur=UString(tokens[i], UString::UTF_8);
				else
				{
					std::string strTag, strChar, strCount;
					int count;
					strTag=tokens[i].substr(0, 1);
					size_t pos1=tokens[i].find("_", 0);
//					assert(pos1!=std::string::npos);
					if(pos1==std::string::npos)
						continue;
					size_t pos2=tokens[i].find("_", pos1+1);
					if(pos2==std::string::npos)
						continue;
//					assert(pos2!=std::string::npos);
					strChar=tokens[i].substr(pos1+1, pos2-pos1-1);
					strCount=tokens[i].substr(pos2+1, tokens[i].length()-pos2-1);
					count=atoi(strCount.c_str());
					if(strTag=="L"&&count>0)
					{
						entity.pre.push_back(UString(strChar, wiselib::UString::UTF_8));
					}
					else if(strTag=="R"&&count>0)
					{
						entity.suc.push_back(UString(strChar, wiselib::UString::UTF_8));
					}
				}
			}
			entity.tagLabels.push_back(label);
	        entities.push_back(entity);
		}
	}
//	for(size_t i=0;i<entities.size();i++)
//	{
//		entities[i].cur.displayStringValue(wiselib::UString::UTF_8);
//		for(int j=0;j<entities[i].pre.size();j++)
//		{
//			entities[i].pre[j].displayStringValue(wiselib::UString::UTF_8);
//		}
//		std::cout<<"..."<<std::endl;
//		for(int j=0;j<entities[i].suc.size();j++)
//		{
//			entities[i].suc[j].displayStringValue(wiselib::UString::UTF_8);
//		}
//		std::cout<<std::endl;
//	}

	inStream.close();

}



