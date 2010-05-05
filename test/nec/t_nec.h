#include <idmlib/nec/NameEntity.h>
#include <iostream>

using namespace idmlib;
using namespace wiselib;

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

void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
{
	ifstream inStream(file.c_str());
	string line = "";
	while (std::getline(inStream, line))
	{
		if (line.length() > 0)
		{
			int posEnd=line.find(",");
			string cur = line.substr(0, posEnd);
			NameEntity entity;
			entity.cur = UString(cur, UString::UTF_8);
			entity.tagLabels.push_back(label);
			entities.push_back(entity);
		}
	}
	inStream.close();

}

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

void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters)
{
	// Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos=str.find_first_of(delimiters, lastPos);
    while (std::string::npos != pos || std::string::npos != lastPos)
	{
		std::string strToken=str.substr(lastPos, pos-lastPos);
		if(strToken!="") tokens.push_back(strToken);
        // Skip delimiters.
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
	}
}


