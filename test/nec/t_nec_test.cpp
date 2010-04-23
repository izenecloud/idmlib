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

using namespace idmlib;


int main()
{


	std::string org_path = "../db/ner/train/org.txt";
	std::string loc_path = "../db/ner/train/loc.txt";
	std::string peop_path = "../db/ner/train/peop.txt";
	std::string other_path = "../db/ner/train/other.txt";
	std::string noise_path = "../db/ner/train/noise.txt";
	std::string test_path = "../db/ner/test/test.txt";

	std::string res_path = "../resource/ner/res/";
	std::string loc_suffix_path = res_path + "loc_s.txt";
	std::string org_suffix_path = res_path + "org_s.txt";
	std::string peop_suffix_path = res_path + "peop_s.txt";
//	std::string name_prefix_path = "/home/eric/Dataset/ch/name_p.txt";
//
//	std::string u_path = "/home/eric/Dataset/ch/pfr_u.txt";
//	std::string q_path = "/home/eric/Dataset/ch/pfr_q.txt";
//	std::string p_path = "/home/eric/Dataset/ch/pfr_p.txt";
//	std::string x_path = "/home/eric/Dataset/ch/pfr_x.txt";
//	std::string c_path = "/home/eric/Dataset/ch/pfr_c.txt";
//	std::string d_path = "/home/eric/Dataset/ch/pfr_d.txt";
//
	NameEntityDict::loadLocSuffix(loc_suffix_path);
	NameEntityDict::loadOrgSuffix(org_suffix_path);
	NameEntityDict::loadPeopSuffix(peop_suffix_path);
//	NameEntityDict::loadNamePrefix(name_prefix_path);
//
//	NameEntityDict::loadU(u_path);
//	NameEntityDict::loadQ(q_path);
//	NameEntityDict::loadP(p_path);
//	NameEntityDict::loadX(x_path);
//	NameEntityDict::loadC(c_path);
//	NameEntityDict::loadD(d_path);

	std::vector<NameEntity> entities;
	loadNameEntities(entities, org_path, "ORG");
	loadNameEntities(entities, loc_path, "LOC");
	loadNameEntities(entities, peop_path, "PEOP");
	loadNameEntities(entities, other_path, "OTHER");
	loadNameEntities(entities, noise_path, "NOISE");

	std::string path = "../resource/ner/";
	NameEntityManager neMgr(path);

	cout << "loading entities..." << endl;
	std::vector<NameEntity> entities2;
	loadNameEntities(entities2, test_path, "NONE");
	cout << "#enties2: " << entities2.size() << endl;

	neMgr.loadModels();
	neMgr.predict(entities2);

	for (size_t i=0; i<entities2.size(); ++i)
	{
		cout << i+1 << "\t";
		string cur="", pre="", suc="";

		entities2[i].cur.convertString(cur, UString::UTF_8);
//		entities2[i].pre.convertString(pre, UString::UTF_8);
//		entities2[i].suc.convertString(suc, UString::UTF_8);

		cout << cur << "\t";

//		classifier.predict(entities2[i]);
//		neMgr.predict(entities2[i]);
		std::vector<Label> labels = entities2[i].predictLabels;
		for (size_t j=0; j<labels.size(); ++j)
		{
			cout << labels[j] << " ";
		}
		cout << endl;
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
