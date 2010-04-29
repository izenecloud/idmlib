#include <idmlib/nec/NameEntity.h>
#include <iostream>

using namespace idmlib;
using namespace wiselib;

void loadNameEntities(std::vector<NameEntity>& entities, string file, Label label)
{
	ifstream inStream(file.c_str());
	string line = "";
	while (std::getline(inStream, line))
	{
		if (line.length() > 0)
		{
//			int pos1 = line.find('\t', 0);
//			if (pos1 != -1)
//			{
//				int pos2 = line.find('\t', pos1+1);
//				if (pos2 != -1)
//				{
					string cur = line;
//					string pre = line.substr(0, pos1);
//					string cur = line.substr(pos1+1, pos2-pos1-1);
//					string suc = line.substr(pos2+1, line.length() - pos2-1);
//					cout << pre << "\t" << cur << "\t" << suc << endl;
					NameEntity entity;
					entity.cur = UString(cur, UString::UTF_8);
//					entity.pre = UString(pre, UString::UTF_8);
//					entity.suc = UString(suc, UString::UTF_8);
					entity.tagLabels.push_back(label);
					entities.push_back(entity);
//				}
//			}
		}
	}
	inStream.close();

}
