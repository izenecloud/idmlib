/*
 * ScdDocumentLoader.h
 *
 *  Created on: Mar 12, 2010
 *      Author: eric
 */

#ifndef SCDDOCUMENTLOADER_H_
#define SCDDOCUMENTLOADER_H_

#include <idmlib/dc/ScdDocument.h>
#include <sf1v5/configuration-manager/CollectionMeta.h>

namespace sf1v5 {
/**
 * @brief This class is used for loading SCD files
 */
class ScdDocumentLoader
{
public:

	ScdDocumentLoader(
			const CollectionMeta& collectionMeta //,
//			const boost::shared_ptr<LAManager>& laManager,
//			const AnalysisInfo& analysisInfo
			) : collectionMeta_(collectionMeta) {}

	virtual ~ScdDocumentLoader() {}

public:
	/**
	 * @brief load SCD files form a directory
	 */
	bool loadFromDir(std::vector<ScdDocument>& docVec, const std::string& dirPath, bool process=true)
	{

		namespace fs = boost::filesystem;

		fs::path fpath(dirPath);
		if (!fs::exists(fpath))
		{
			std::cout << dirPath << "doesn't exist" << std::endl;
			return false;
		}
		if (!fs::is_directory(fpath)) {
			std::cout << dirPath << "is not directory" << std::endl;
			return false;
		}

		fs::directory_iterator item_begin(fpath);
		fs::directory_iterator item_end;
		for (; item_begin != item_end; item_begin++)
		{
			if (fs::is_regular_file(item_begin->status()))
			{
				cout << "loading " <<  item_begin->path().file_string() << endl;
				loadFromFile(docVec, item_begin->path().file_string(), process);
			}
		}
		return true;

	}

	/**
	 * @brief load a SCD file
	 */
	bool loadFromFile(std::vector<ScdDocument>& docVec, const std::string& filePath, bool process=true)
	{
		ScdParser parser(collectionMeta_.getEncoding());
			if (parser.load(filePath) == false)
			{
				std::cout << "can't load file " << filePath << std::endl;
				return false;
			}

			string fieldStr;
			string fieldVal;
			vector<FieldPair>::iterator p;

			for (ScdParser::iterator doc_iter = parser.begin();
					doc_iter != parser.end(); ++doc_iter )
			{
				bool mining = false;
				ScdDocument document;
				string content = "";
				SCDDocPtr doc = (*doc_iter);
				for (p = doc->begin(); p != doc->end(); p++)
				{
					std::set<PropertyConfig, PropertyComp>::iterator iter;
					p->first.convertString(fieldStr, collectionMeta_.getEncoding());
					p->second.convertString(fieldVal, collectionMeta_.getEncoding());
					if (fieldStr == "DOCID")
					{
		//				cout << fieldVal << endl;
						document.docId = fieldVal;
					}
		//			cout << fieldStr << endl;
		//			cout << fieldVal << endl;

					PropertyConfig temp;
					temp.propertyName_ = fieldStr;
					iter = collectionMeta_.getDocumentSchema().find(temp);

		//			if (iter->isMining())
		//			{
		//				cout << fieldStr << " to be mined" << endl;
		//				mining = true;
						content += fieldVal;  // set document content for classification
		//			}

					if (fieldStr == "CATEGORY")
					{
						document.tagLabels.push_back(fieldVal);
					}
				}
		//		if (mining)
		//		{
		//			cout << "content: " << endl << content << endl;
		//			cout << "label:" << document.tagLabels[0] << endl;
		//			cout << content << endl;
					document.setContent(content, process);
		//			cout << document.content << endl;
					docVec.push_back(document);
		//			cout << docVec.size() << endl;
		//			cout << docVec[docVec.size()-1].content << endl;
		//		}
			}
			return true;
	}


private:
	CollectionMeta collectionMeta_;

};

}

#endif /* SCDDOCUMENTLOADER_H_ */
