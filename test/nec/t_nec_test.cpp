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

	for (size_t i=0; i<entities2.size(); ++i)
	{
//		cout << i+1 << "\t";
		testOut<<i+1 << "\t";
		string cur="", pre="", suc="";

		entities2[i].cur.convertString(cur, UString::UTF_8);

//		cout << cur << "\t";
		testOut<<cur << "\t";

		std::vector<Label> labels = entities2[i].predictLabels;
		for (size_t j=0; j<labels.size(); ++j)
		{
//			cout << labels[j] << " ";
			testOut<<labels[j] << " ";
		}
//		cout << endl;
		testOut<<std::endl;
	}

//	int right=0, wrong=0, total=0;
//	for (size_t i=0; i<entities.size(); ++i)
//	{
//		classifier.predict(entities[i]);
//		std::vector<Label> predicts = entities[i].predictLabels;
//		std::vector<Label> tags = entities[i].tagLabels;
//		for (size_t j=0; j<predicts.size(); ++j)
//		{
//			if (std::find(tags.begin(), tags.end(), predicts[j]) != tags.end())
//			{
//				++right;
//			}
//			else
//			{
//				++wrong;
//			}
//		}
//		total += tags.size();
//	}
//
//	double P = (double)right/(right+wrong);
//	double R = (double)right/total;
//	double F = 2*P*R/(P+R);
//
//	cout << "P: " << P << endl;
//	cout << "R: " << R << endl;
//	cout << "F: " << F << endl;

}
