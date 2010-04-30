/*
 * Document.h
 *
 *  Created on: Mar 19, 2010
 *      Author: eric
 */

#ifndef DM_SCDDOCUMENT_H_
#define DM_SCDDOCUMENT_H_

#include <ml/ClassificationData.h>
#include <vector>
#include <ext/hash_map>

using __gnu_cxx::hash_map;

namespace idmlib {

typedef int DocId;


enum LangType {
	LANG_EN = 0,	// English
	LANG_KR, 		// Korean
	LANG_CN, 		// Chinese
	LANG_JP 		// Japanese
};

/**
 * ScdDocument is target of classification
 */

class ScdDocument : public ml::ClassificationData
{

public:

	// term frequency map
	hash_map<ml::AttrID, int> tfMap;

	DocId id;
	LangType lang;
	std::string docId;
	std::string content;

	ScdDocument();
//	ScdDocument(const ScdDocument& doc);
	~ScdDocument();

	void setContent(const std::string& content, bool process=true);

};

}

#endif /* SCDDOCUMENT_H_ */
