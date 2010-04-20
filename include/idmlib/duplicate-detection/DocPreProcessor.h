/*
 * DocPreProcessor.h
 *
 *  Created on: 2008-12-12
 *      Author: jinglei
 */

#ifndef DOCPREPROCESSOR_H_
#define DOCPREPROCESSOR_H_

#include <wiselib/DArray.h>
#include <wiselib/YString.h>
#include <fstream>
#include <list>
#include <string>
#include <map>
#include <vector>
#include <set>

/**
 *@brief Preprocess the TREC style test documents used to test duplicate algorithms in experiment.
 */
class DocPreProcessor {
public:
	DocPreProcessor(const std::string& strDir);
	virtual ~DocPreProcessor();
	/**
	 * @brief get the docs from the directory.
	 * @param strDir the directory where the SGML files in.
	 */
	bool getDocs(wiselib::DynamicArray<wiselib::YString>& docNames, wiselib::DynamicArray<wiselib::YString>& docContents);
    std::vector<std::string> _vecTerm;
protected:
	/**
	 * @brief the directory of where the sgml files in.
	 */

private:
    std::string _strDir;
	void ReadDir(const std::string &strDir,std::list<std::string>& listOut);
	const char* readfile(const char* dir, char *fileName);
	void parseFile(const std::string& strFile, std::map<std::string,std::string>&mapIDContent);
	void cutMetaTag(std::string& strContent);
	void normalize(const std::string& str, std::string& tokens, const std::string& delimiters=" \n\r");
	void tokenize(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters=" \n\r");
	void synDupGenerate(const std::string& strOrigDoc, std::string& strSynDupDoc);
	void genSynFile(const std::string& strFile, std::map<std::string,std::string>&mapIDContent,int& count);

};


#endif /* DOCPREPROCESSOR_H_ */