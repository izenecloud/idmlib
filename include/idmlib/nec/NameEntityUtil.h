/*
 * NameEntityUtil.h
 *
 *  Created on: Apr 8, 2010
 *      Author: eric
 */

#ifndef DM_NAMEENTITYUTIL_H_
#define DM_NAMEENTITYUTIL_H_

#include <ml/ClassificationDataUtil.h>
#include "util/Vectors.h"
#include "NameEntity.h"
#include <idmlib/util/IntIdMgr.h>
#include "NameEntityDict.h"
#include <wiselib/ustring/algo.hpp>

using namespace idmlib;

namespace ml
{
    void addData(
    		std::vector<wiselib::UString>& featureList,
    		ml::Schema& schema, ml::Instance& inst,
    		double featureWeight)
   {
    	std::vector<ml::AttrID> featureIds;
		idmlib::IntIdMgr::getTermIdListByTermStringList(featureList, featureIds);
		for (size_t i=0; i<featureIds.size(); ++i)
		{
			inst.x.set(featureIds[i], featureWeight);
			schema.setAttr(featureIds[i], 1);
		}
   }

	template<>
	void ClassificationDataUtil<NameEntity>::transform(NameEntity& entity, ml::Schema& schema, ml::Instance& inst)
	{
		std::vector<wiselib::UString> suc = entity.suc;
		std::vector<wiselib::UString> pre = entity.pre;
		std::vector<wiselib::UString> vecCur;
		bool isChinese=false;
		if(entity.cur.length()>0&&entity.cur.isChineseChar(0))
		{
			isChinese=true;
			wiselib::UString tempStr;
			for(size_t i=0;i<entity.cur.length();i++)
			{
				vecCur.push_back(entity.cur.substr(tempStr, i, 1));
			}
		}
		else if(entity.cur.length()>0)
		{
			UString delimiter(" ", UString::UTF_8);
			wiselib::Algorithm<wiselib::UString>::make_tokens_with_delimiter(entity.cur, delimiter, vecCur);
		}

		// current character sequence
		size_t curLength = vecCur.size();
		UString cur_b, b_cur_b, cur_e, cur_a, b_cur_a, b_cur_e, t_cur_a, t_cur_e, cur_l, left, right;
		UString utag_curb("_UCB", UString::UTF_8); // the begin character of current character sequence
		UString btag_curb("_BCB", UString::UTF_8); // the begin bigram of current character sequence
		UString utag_cure("_UCE", UString::UTF_8); // the end character of current character sequence
		UString utag_cura("_UCA", UString::UTF_8); // all characters of current character sequence
		UString btag_cura("_BCA", UString::UTF_8); // all bigrams of current sequence
		UString btag_cure("_BCE", UString::UTF_8); // the end bigram
		UString ttag_cure("_TCE", UString::UTF_8); // the end trigram
//		UString utag_curl("_UL", UString::UTF_8); // length
		UString btag_curn("_BN", UString::UTF_8); // has noise
		UString btag_curo("_BO", UString::UTF_8); // has other bigram
		UString btag_curl("_BL", UString::UTF_8); // has location bigram
		UString btag_curg("_BG", UString::UTF_8); // has orgnization bigram
		UString utag_curn("_UN", UString::UTF_8); // has noise
		UString utag_curo("_UO", UString::UTF_8); // has other bigram
		UString utag_curl("_UL", UString::UTF_8); // has location bigram
		UString utag_curg("_UG", UString::UTF_8); // has orgnization bigram
		UString utag_left("_LEFT", UString::UTF_8); // has left context
		UString utag_right("_RIGHT", UString::UTF_8); // has left context
		UString blank(" ", UString::UTF_8);

		std::vector<UString> f_cur_u_b;
		std::vector<UString> f_cur_b_b;
		std::vector<UString> f_cur_u_a;
		std::vector<UString> f_cur_b_a;
		std::vector<UString> f_cur_b_e;
		std::vector<UString> f_cur_t_e;
		std::vector<UString> f_cur_u_e;
		std::vector<UString> f_cur_b_n;
		std::vector<UString> f_cur_b_o;
		std::vector<UString> f_cur_b_l;
		std::vector<UString> f_cur_b_g;
		std::vector<UString> f_cur_u_n;
		std::vector<UString> f_cur_u_o;
		std::vector<UString> f_cur_u_l;
		std::vector<UString> f_cur_u_g;
		std::vector<UString> f_cur_l;
		std::vector<UString> f_left;
		std::vector<UString> f_right;
		std::vector<UString> f_all;

		double w_cur_u_b = 3;
		double w_cur_b_b = 4;
		double w_cur_u_a = 2;
		double w_cur_b_a = 4;
		double w_cur_b_e = 16;
		double w_cur_t_e = 32;
		double w_cur_u_e = 8;
		double w_cur_b_n = 1.0;
		double w_cur_b_o = 2;
		double w_cur_b_l=3;
		double w_cur_b_g=3;
		double w_cur_u_n = 0.8;
		double w_cur_u_o = 2;
		double w_cur_u_l=3;
		double w_cur_u_g=3;
		double w_left = 2;
		double w_right = 2;

        string u_cur_e_str;
		string b_cur_e_str;
		string t_cur_e_str;

		if (curLength >= 1)
		{
			cur_b=vecCur[0];
			cur_b.append(utag_curb);
			f_cur_u_b.push_back(cur_b);	// the first unigram of current sequence
			f_all.push_back(cur_b);
			addData(f_cur_u_b, schema, inst, w_cur_u_b);

			cur_e=vecCur[curLength-1];
			cur_e.append(utag_cure);
			cur_e.convertString(u_cur_e_str, UString::UTF_8);
			f_cur_u_e.push_back(cur_e);  // the last unigram of current sequence
			f_all.push_back(cur_e);
			addData(f_cur_u_e, schema, inst, w_cur_u_e);

			bool hasNoiseUnigram=false;
			bool hasOtherUnigram=false;
			bool hasLocUnigram=false;
			bool hasOrgUnigram=false;
			for (size_t i=0; i<curLength; ++i)
			{
				cur_a=vecCur[i];
				std::string strUnigram;
				cur_a.convertString(strUnigram, wiselib::UString::UTF_8);
				cur_a.append(utag_cura);
				f_cur_u_a.push_back(cur_a);	  // unigram of current sequence
				f_all.push_back(cur_a);
				if(!isChinese&&NameEntityDict::isKownNoise(strUnigram))
					hasNoiseUnigram=true;
				else if(!isChinese&&NameEntityDict::isNoun(strUnigram))
					hasOtherUnigram=true;
				else if(!isChinese&&NameEntityDict::isKownLoc(strUnigram))
					hasLocUnigram=true;
				else if(!isChinese&&NameEntityDict::isKownOrg(strUnigram))
					hasOrgUnigram=true;
			}
			addData(f_cur_u_a, schema, inst, w_cur_u_a);
			if(hasNoiseUnigram)
			{
				f_cur_u_n.push_back(utag_curn);
				f_all.push_back(utag_curn);
				addData(f_cur_u_n, schema, inst, w_cur_u_n);
			}

			if(hasOtherUnigram)
			{
				f_cur_u_o.push_back(utag_curo);
				f_all.push_back(utag_curo);
				addData(f_cur_u_o, schema, inst, w_cur_u_o);
			}

			if(hasLocUnigram)
			{
				f_cur_u_l.push_back(utag_curl);
				f_all.push_back(utag_curl);
				addData(f_cur_u_l, schema, inst, w_cur_u_l);
			}

			if(hasOrgUnigram)
			{
				f_cur_u_g.push_back(utag_curg);
				f_all.push_back(utag_curg);
				addData(f_cur_u_g, schema, inst, w_cur_u_g);
			}
		}

		if (curLength > 1)
		{
			b_cur_b=vecCur[0];
			b_cur_b.append(vecCur[1]);
			b_cur_b.append(btag_curb);
			f_cur_b_b.push_back(b_cur_b);
			f_all.push_back(b_cur_b);
			addData(f_cur_b_b, schema, inst, w_cur_b_b);

			b_cur_e=vecCur[curLength-2];
			b_cur_e.append(vecCur[curLength-1]);
			b_cur_e.convertString(b_cur_e_str, UString::UTF_8);
			b_cur_e.append(btag_cure);
			f_cur_b_e.push_back(b_cur_e);  // the last bigram of current sequence
			f_all.push_back(b_cur_e);
			addData(f_cur_b_e, schema, inst, w_cur_b_e);

			bool hasNoiseBigram=false;
			bool hasOtherBigram=false;
			bool hasLocBigram=false;
			bool hasOrgBigram=false;
			for (size_t i=0; i<curLength-1; ++i)
			{
				b_cur_a=vecCur[i];
				b_cur_a.append(vecCur[i+1]);
				std::string strBigram;
				b_cur_a.convertString(strBigram, wiselib::UString::UTF_8);
				b_cur_a.append(btag_cura);
				f_cur_b_a.push_back(b_cur_a);  // bigram of current sequence
				f_all.push_back(b_cur_a);
				if(NameEntityDict::isKownNoise(strBigram))
					hasNoiseBigram=true;
				else if(NameEntityDict::isNoun(strBigram))
					hasOtherBigram=true;
				else if(NameEntityDict::isKownLoc(strBigram))
					hasLocBigram=true;
				else if(NameEntityDict::isKownOrg(strBigram))
					hasOrgBigram=true;
			}
			addData(f_cur_b_a, schema, inst, w_cur_b_a);
			if(isChinese&&hasNoiseBigram)
			{
				f_cur_b_n.push_back(btag_curn);
				f_all.push_back(btag_curn);
				addData(f_cur_b_n, schema, inst, w_cur_b_n);
			}

			if(hasOtherBigram)
			{
				f_cur_b_o.push_back(btag_curo);
				f_all.push_back(btag_curo);
				addData(f_cur_b_o, schema, inst, w_cur_b_o);
			}

			if(hasLocBigram)
			{
				f_cur_b_l.push_back(btag_curl);
				f_all.push_back(btag_curl);
				addData(f_cur_b_l, schema, inst, w_cur_b_l);
			}

			if(hasOrgBigram)
			{
				f_cur_b_g.push_back(btag_curg);
				f_all.push_back(btag_curg);
				addData(f_cur_b_g, schema, inst, w_cur_b_g);
			}
		}

		if (curLength > 2)
		{
			t_cur_e=vecCur[curLength-3];
            t_cur_e.append(vecCur[curLength-2]);
            t_cur_e.append(vecCur[curLength-1]);
			t_cur_e.convertString(t_cur_e_str, UString::UTF_8);
			t_cur_e.append(ttag_cure);

			f_cur_t_e.push_back(t_cur_e);  // the last trigram of current sequence
			f_all.push_back(t_cur_e);
			addData(f_cur_t_e, schema, inst, w_cur_t_e);
		}

		//Using known NER suffix from the lexicon as the features.
		// whether is locaction suffix
		std::vector<UString> f_cur_loc;
		double w_cur_loc = 10;
		std::vector<UString> f_cur_loc2;
		double w_cur_loc2 = 12;
    	std::vector<UString> f_cur_loc3;
		double w_cur_loc3 = 12;
		UString utag_cure_loc("_UCEL", UString::UTF_8);
		UString btag_cure_loc("_BCEL", UString::UTF_8);
		UString ttag_cure_loc("_TCEL", UString::UTF_8);
		std::vector<UString> f_cur_org;
		double w_cur_org = 16;
		std::vector<UString> f_cur_org2;
		double w_cur_org2 = 16;
		std::vector<UString> f_cur_org3;
		double w_cur_org3 = 16;
		// whether is org suffix
		UString utag_cure_org("_UCEO", UString::UTF_8);
		UString btag_cure_org("_BCEO", UString::UTF_8);
		UString ttag_cure_org("_TCEO", UString::UTF_8);
		std::vector<UString> f_cur_peop;
		double w_cur_peop = 4;
		std::vector<UString> f_cur_peop2;
		double w_cur_peop2 = 12;
		std::vector<UString> f_cur_peop3;
		double w_cur_peop3 = 12;
		// whether is org suffix
		UString utag_cure_peop("_UCEP", UString::UTF_8);
		UString btag_cure_peop("_BCEP", UString::UTF_8);
		UString ttag_cure_peop("_TCEP", UString::UTF_8);

		if (curLength >= 1)
		{
			if(NameEntityDict::isLocSuffix(u_cur_e_str))
			{
				f_cur_loc.push_back(utag_cure_loc);
				f_all.push_back(utag_cure_loc);
				addData(f_cur_loc, schema, inst, w_cur_loc);
			}
			if(NameEntityDict::isOrgSuffix(u_cur_e_str))
			{
				f_cur_org.push_back(utag_cure_org);
				f_all.push_back(utag_cure_org);
				addData(f_cur_org, schema, inst, w_cur_org);
			}
			if(NameEntityDict::isPeopSuffix(u_cur_e_str))
			{
				f_cur_peop.push_back(utag_cure_peop);
				f_all.push_back(utag_cure_peop);
				addData(f_cur_peop, schema, inst, w_cur_peop);
			}
		}
		if (curLength >= 2)
		{
			if (NameEntityDict::isLocSuffix(b_cur_e_str))
			{
				f_cur_loc2.push_back(btag_cure_loc);
				f_all.push_back(btag_cure_loc);
				addData(f_cur_loc2, schema, inst, w_cur_loc2);
			}
			if (NameEntityDict::isOrgSuffix(b_cur_e_str))
			{
				f_cur_org2.push_back(btag_cure_org);
				f_all.push_back(btag_cure_org);
				addData(f_cur_org2, schema, inst, w_cur_org2);
			}
			if (NameEntityDict::isPeopSuffix(b_cur_e_str))
			{
				f_cur_peop2.push_back(btag_cure_peop);
				f_all.push_back(btag_cure_peop);
				addData(f_cur_peop2, schema, inst, w_cur_peop2);
			}
		}
		if (curLength >= 3)
		{
			if (NameEntityDict::isLocSuffix(t_cur_e_str))
			{
				f_cur_loc3.push_back(ttag_cure_loc);
				f_all.push_back(ttag_cure_loc);
				addData(f_cur_loc3, schema, inst, w_cur_loc3);
			}
			if (NameEntityDict::isOrgSuffix(t_cur_e_str))
			{
				f_cur_org3.push_back(ttag_cure_org);
				f_all.push_back(ttag_cure_org);
				addData(f_cur_org3, schema, inst, w_cur_org3);
			}
			if (NameEntityDict::isPeopSuffix(t_cur_e_str))
			{
				f_cur_peop3.push_back(ttag_cure_peop);
				f_all.push_back(ttag_cure_peop);
				addData(f_cur_peop3, schema, inst, w_cur_peop3);
			}

		}

	// whether is name prefix
		std::vector<UString> f_cur_surname;
		std::vector<ml::AttrID> id_cur_surname;
		double w_cur_surname = 16;
		UString utag_curb_name("_UCBN", UString::UTF_8);
		if (curLength == 3||curLength ==2)
		{
			UString cur_b=vecCur[0];
			string name;
			cur_b.convertString(name, UString::UTF_8);
			if(NameEntityDict::isNamePrefix(name))
			{
				f_cur_surname.push_back(utag_curb_name);
				f_all.push_back(utag_curb_name);
			}
		}
		addData(f_cur_surname, schema, inst, w_cur_surname);

/// Add the contextual features.
		UString peopTag("_PEOP", wiselib::UString::UTF_8);
		UString locTag("_LOC", wiselib::UString::UTF_8);
		UString orgTag("_ORG", wiselib::UString::UTF_8);
		UString theTag("_THE", wiselib::UString::UTF_8);
		if(pre.size()>0)
		{
			f_left.clear();
			for(size_t i=0;i<pre.size();i++)
			{
				std::string strItem;
				pre[i].convertString(strItem, wiselib::UString::UTF_8);
				if(NameEntityDict::isThe(strItem))
				{
					UString preChar=theTag;
					f_left.push_back(preChar);
					f_all.push_back(preChar);
				}
				if(NameEntityDict::isPeopLeft(strItem))
				{
					UString preChar=pre[i];
					preChar.append(utag_left);
                   	preChar.append(peopTag);
					f_left.push_back(preChar);
					f_all.push_back(preChar);
				}
				if(NameEntityDict::isLocLeft(strItem))
				{
					UString preChar=pre[i];
					preChar.append(utag_left);
                   	preChar.append(locTag);
					f_left.push_back(preChar);
					f_all.push_back(preChar);
				}
				if(NameEntityDict::isOrgLeft(strItem))
				{
					UString preChar=pre[i];
					preChar.append(utag_left);
                   	preChar.append(orgTag);
					f_left.push_back(preChar);
					f_all.push_back(preChar);
				}
			}

			addData(f_left, schema, inst, w_left);
		}
		if(suc.size()>0)
		{
			f_right.clear();
			for(size_t i=0;i<suc.size();i++)
			{
				std::string strItem;
				suc[i].convertString(strItem, wiselib::UString::UTF_8);
				if(NameEntityDict::isPeopRight(strItem))
				{
					UString preChar=suc[i];
					preChar.append(utag_right);
                   	preChar.append(peopTag);
					f_right.push_back(preChar);
					f_all.push_back(preChar);
				}
				else if(NameEntityDict::isLocRight(strItem))
				{
					UString preChar=suc[i];
					preChar.append(utag_right);
                   	preChar.append(locTag);
					f_right.push_back(preChar);
					f_all.push_back(preChar);
				}
				else if(NameEntityDict::isOrgRight(strItem))
				{
					UString preChar=suc[i];
					preChar.append(utag_right);
                   	preChar.append(orgTag);
					f_right.push_back(preChar);
					f_all.push_back(preChar);
				}
			}

			addData(f_right, schema, inst, w_right);
		}
	}

	template<>
	void ClassificationDataUtil<NameEntity>::transform(std::vector<NameEntity>& entities, ml::InstanceBag& instBag)
	{
		std::vector<UString> features;
		std::vector<ml::AttrID> featureIds;
		std::vector<NameEntity>::const_iterator it;
		ml::Schema schema;
		for (it = entities.begin(); it != entities.end(); ++it)
		{
			NameEntity entity = (*it);
			ml::Instance inst;
			transform(entity, schema, inst);
			instBag.addInst(inst);
			std::vector<Label> labels = (*it).tagLabels;
			for (std::vector<Label>::const_iterator it = labels.begin();
							it != labels.end(); ++it)
			{
				Label label = *it;
				if (std::find(labels.begin(), labels.end(), label) == labels.end())
				{
					labels.push_back(label);
					schema.addLabel(label);
				}
			}
		}

		instBag.setSchema(schema);
	}
}

#endif /* NAMEENTITYUTIL_H_ */
