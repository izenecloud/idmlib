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



NameEntityManager& NameEntityManager::getInstance(const std::string& path)
{
    static NameEntityManager nem(path);
    return nem;
}


NameEntityManager::NameEntityManager(const std::string& path)
    : classifier_(0)
    , path_(path)
{
	if (!classifier_)
	{
		std::string res_path = path + "/res/";
		std::string loc_suffix_path = res_path + "loc_s";
		std::string org_suffix_path = res_path + "org_s";
		std::string peop_suffix_path = res_path + "peop_s";
		std::string loc_list_path = res_path + "loc";
		std::string peop_list_path = res_path + "peop";
		std::string org_list_path = res_path + "org";
		std::string noise_list_path = res_path + "noise";
		std::string other_list_path = res_path + "other";
		std::string noun_list_path = res_path + "noun";
		std::string name_prefix_path =res_path + "surname";

		NameEntityDict::loadLocSuffix(loc_suffix_path);
		NameEntityDict::loadOrgSuffix(org_suffix_path);
		NameEntityDict::loadPeopSuffix(peop_suffix_path);
		NameEntityDict::loadLocList(loc_list_path);
		NameEntityDict::loadOrgList(org_list_path);
		NameEntityDict::loadPeopList(peop_list_path);
		NameEntityDict::loadNoiseList(noise_list_path);
		NameEntityDict::loadOtherList(other_list_path);
		NameEntityDict::loadNounList(noun_list_path);
		NameEntityDict::loadNamePrefix(name_prefix_path);

		std::string model_path = path + "/model/";
		ml::ClassifierType type = LR;
		classifier_ = new ml::ClassificationManager<NameEntity>(model_path, type);
        loadModels();
	}
}

NameEntityManager::~NameEntityManager()
{
    if (classifier_)
    {
        classifier_->destroyModels();
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
	//Now the labels in training and test are hard-coded.
	// This needs to be make more configurable.
	if (classifier_&&entity.cur.length()>0)
	{
		string strEntity;
		//hard-coded encoding type, needs to be adjusted.
		entity.cur.convertString(strEntity, izenelib::util::UString::UTF_8);
		if(NameEntityDict::isKownNoise(strEntity))
		{
			entity.predictLabels.push_back("NOISE");
		}
		else if(NameEntityDict::isKownOther(strEntity))
		{
			entity.predictLabels.push_back("OTHER");
		}
		else if(NameEntityDict::isKownOrg(strEntity))
		{
			entity.predictLabels.push_back("ORG");
		}
		else if(NameEntityDict::isKownLoc(strEntity))
		{
			entity.predictLabels.push_back("LOC");
		}
		else if(NameEntityDict::isKownPeop(strEntity))
		{
			entity.predictLabels.push_back("PEOP");
		}
		else if(!entity.cur.isKoreanChar(0))
		{
			classifier_->predict(entity);
			postProcessing(entity);
		}
		else
		{
//			classifier_->predict(entity);
			postProcessing(entity);
		}
	}
}

void NameEntityManager::postProcessing(NameEntity& entity)
{
	if(entity.cur.length()==0) return;

	std::vector<izenelib::util::UString> vecCur;
	if(entity.cur.isChineseChar(0)||entity.cur.isKoreanChar(0))
	{
		izenelib::util::UString tempStr;
		for(size_t i=0;i<entity.cur.length();i++)
		{
			vecCur.push_back(entity.cur.substr(tempStr, i, 1));
		}
	}
	else //languages with word segmentation identifier.
	{
		UString delimiter(" ", UString::UTF_8);
		izenelib::util::Algorithm<izenelib::util::UString>::make_tokens_with_delimiter(entity.cur, delimiter, vecCur);
	}

	string strEntity;
	entity.cur.convertString(strEntity, izenelib::util::UString::UTF_8);

	if(entity.cur.isKoreanChar(0)&&entity.predictLabels.size()==0)
	{
		if(vecCur.size()==3&&entity.cur.isKoreanChar(0))
		{
	          std::string surname, name;
	          izenelib::util::UString ustrSurname, ustrName;
	          ustrSurname=vecCur[0];
	          ustrName=vecCur[1];
	          ustrName.append(vecCur[2]);
	          ustrSurname.convertString(surname, izenelib::util::UString::UTF_8);
	          ustrName.convertString(name, izenelib::util::UString::UTF_8);
	          if(NameEntityDict::isNamePrefix(surname)&&NameEntityDict::isPeopSuffix(name))
	          {
	            	entity.predictLabels.push_back("PEOP");
	          }
		}
	}
	else
	{
		if(entity.predictLabels.size()>0&&entity.predictLabels[0]=="PEOP")
		{
			if(vecCur.size()==2)
			{
				if(NameEntityDict::isPeopSuffix(strEntity)&&entity.cur.isChineseChar(0))
				{
					entity.predictLabels[0]="OTHER";
				}
			}
			if(NameEntityDict::isNoun(strEntity))
			{
				entity.predictLabels[0]="OTHER";
			}
			if(vecCur.size()>1&&!entity.cur.isChineseChar(0))
			{
				string strLast;
				vecCur[vecCur.size()-1].convertString(strLast, izenelib::util::UString::UTF_8);
				if(NameEntityDict::isOrgSuffix(strLast))
				{
					entity.predictLabels[0]="ORG";
				}
				else if (NameEntityDict::isLocSuffix(strLast))
				{
					entity.predictLabels[0]="LOC";
				}
			}
		}
		else if(entity.predictLabels.size()>0&&entity.predictLabels[0]=="ORG")
		{
			if(entity.cur.length()==2)
			{
				if(NameEntityDict::isOrgSuffix(strEntity))
				{
					entity.predictLabels[0]="OTHER";
				}
			}
			else if(NameEntityDict::isNoun(strEntity))
			{
				entity.predictLabels[0]="OTHER";
			}
		}
		else if(entity.predictLabels.size()>0&&entity.predictLabels[0]=="LOC")
		{
			if(entity.cur.length()==2)
			{
				if(NameEntityDict::isOrgSuffix(strEntity))
				{
					entity.predictLabels[0]="OTHER";
				}
			}
			if(NameEntityDict::isNoun(strEntity))
			{
				entity.predictLabels[0]="OTHER";
			}
			if(vecCur.size()>1&&!entity.cur.isChineseChar(0)&&!entity.cur.isKoreanChar(0))
			{
				string strLast;
				vecCur[vecCur.size()-1].convertString(strLast, izenelib::util::UString::UTF_8);
				if(NameEntityDict::isOrgSuffix(strLast))
				{
					entity.predictLabels[0]="ORG";
				}
			}
		}
	}
	for(size_t i=0;i<entity.pre.size();i++)
	{
		std::string strItem;
		entity.pre[i].convertString(strItem, izenelib::util::UString::UTF_8);
		if(NameEntityDict::isThe(strItem))
		{
			if(entity.predictLabels.size()>0&&entity.predictLabels[0]=="PEOP")
			{
				entity.predictLabels[0]=="OTHER";
				break;
			}
		}
	}
}


void NameEntityManager::predict(std::vector<NameEntity>& entities)
{
	if (classifier_)
	{
		std::vector<NameEntity>::iterator it;
		for (it = entities.begin(); it != entities.end(); ++it)
		{
			predict(*it);
		}
	}
}

}
