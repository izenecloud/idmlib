/*
 * DocumentUtil.cc
 *
 *  Created on: Mar 19, 2010
 *      Author: eric
 */


#include <ml/ClassificationDataUtil.h>
#include "ScdDocument.h"
#include <util/Vectors.h>

using namespace idmlib;

namespace ml
{

	template<>
	void ClassificationDataUtil<ScdDocument>::transform(ScdDocument& doc, ml::Schema& schema, ml::Instance& inst)
	{
		hash_map<ml::AttrID, ml::AttrValue>& attrMap = schema.getAttrMap();
		hash_map<int, int>::const_iterator it;

		int id, tf;

		for (it = doc.tfMap.begin(); it != doc.tfMap.end(); ++it)
		{
			id = (*it).first;
			tf = (*it).second;
			if (attrMap.find(id) != attrMap.end())
			{
				ml::AttrValue idf = attrMap[id];
				inst.x.set(id, idf * log((double)tf+1));
			}
		}
	}
	//
	template<>
	void ClassificationDataUtil<ScdDocument>::transform(std::vector<ScdDocument>& docs, ml::InstanceBag& instBag)
	{
		// document frequency map
		hash_map<ml::AttrID, int> dfMap;
		// all labels
		std::vector<Label> labels;

		std::vector<ScdDocument>::const_iterator doc_it;

		double tf = 0.0, idf = 0.0;
		int nDoc = docs.size();
		ml::Schema schema;

		for (doc_it = docs.begin(); doc_it != docs.end(); ++doc_it)
		{
			ml::Instance inst;
			std::vector<Label> labels = (*doc_it).tagLabels;

			for (hash_map<int, int>::const_iterator it = (*doc_it).tfMap.begin();
					it != (*doc_it).tfMap.end(); ++it)
			{
				int term = it->first;
				int freq = it->second;

				if (dfMap.find(term) != dfMap.end())
				{
					dfMap[term]++;
				} else
				{
					dfMap[term] = 1;
				}

				tf = log((double)freq + 1);
				inst.x.set(term, tf);
			}

			instBag.addInst(inst);

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

		// generate schema
		for (hash_map<AttrID, int>::const_iterator it=dfMap.begin();
				it!=dfMap.end(); it++)
		{
			AttrID id = it->first;
			idf = log((double)nDoc/dfMap[id]);
			schema.setAttr(id, idf);
		}

		for (int i=0; i<nDoc; ++i)
		{
			// set tf * idf
			ml::Instance& inst = instBag.getInst(i);
			SVector s = inst.x;
			for (const SVector::Pair *p = s; p->i >= 0; p++)
			{
				int id = p->i;
				tf = p->v;
				idf = schema.getAttr(id);
				inst.x.set(id, tf*idf);
			}

			// set labels for instance
			std::vector<Label> tagLabels = docs[i].tagLabels;
			for (size_t j=0; j<tagLabels.size(); ++j)
			{
				int category = -1;
				vector<Label>::iterator it = std::find(labels.begin(), labels.end(), tagLabels[j]);
				if (it != labels.end())
				{
					category = it - labels.begin();
					instBag.setCategory(i, category);
				}
			}
		}

		instBag.setSchema(schema);

	}

}
