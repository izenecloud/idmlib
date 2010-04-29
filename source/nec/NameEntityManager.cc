/*
 * NameEntityManager.cc
 *
 *  Created on: Apr 9, 2010
 *      Author: eric
 */

#include <idmlib/nec/NameEntityUtil.h>
#include <idmlib/nec/NameEntityManager.h>
#include <idmlib/nec/NameEntityDict.h>


namespace idmlib
{

ml::ClassificationManager<NameEntity>* NameEntityManager::classifier_ = 0;

NameEntityManager::NameEntityManager(const std::string& path): path_(path)
{
	if (!classifier_)
	{
		std::string res_path = path + "/res/";
		std::string loc_suffix_path = res_path + "loc_s.txt";
		std::string org_suffix_path = res_path + "org_s.txt";
		std::string peop_suffix_path = res_path + "peop_s.txt";

		NameEntityDict::loadLocSuffix(loc_suffix_path);
		NameEntityDict::loadOrgSuffix(org_suffix_path);
		NameEntityDict::loadPeopSuffix(peop_suffix_path);

		std::string model_path = path + "/model/";
		ml::ClassifierType type = LR;
		classifier_ = new ml::ClassificationManager<NameEntity>(model_path, type);

	}
}

void NameEntityManager::train(std::vector<NameEntity>& entities)
{
	if (classifier_)
	{
		classifier_->train(entities);
	}
}

void NameEntityManager::loadModels()
{
	if (classifier_)
	{
		classifier_->loadSchema();
		classifier_->loadModels();
	}
}


void NameEntityManager::predict(NameEntity& entity)
{
	if (classifier_)
	{
		classifier_->predict(entity);
	}
}


void NameEntityManager::predict(std::vector<NameEntity>& entities)
{
	if (classifier_)
	{
		std::vector<NameEntity>::iterator it;
		for (it = entities.begin(); it != entities.end(); ++it)
		{
			classifier_->predict(*it);
		}
	}
}

}
