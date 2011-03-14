/**
 * @file idmlib/semantic_space/semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief 
 */
#ifndef IDMLIB_SSP_SEMANTIC_INTERPRETER_H_
#define IDMLIB_SSP_SEMANTIC_INTERPRETER_H_

#include <iostream>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_space.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{
public:
	SemanticInterpreter(boost::shared_ptr<SemanticSpace> pSSpace)
	: pSSpace_(pSSpace)
	{

	}

	virtual ~SemanticInterpreter()
	{

	}

public:

	/**
	 * Interpret semantic of a text (document)
	 * maps fragments of natural language text (document) into
	 * a interpretation vector
	 * @param[IN] text a input natural language text or a document
	 * @param[OUT] interVector interpretation vector of the input text (document)
	 * @return true on success, false on failure.
	 */
	bool interpret(std::string& text, std::vector<uint32_t>& interVector) { return false; }

	bool needKnowledge() { return true; }

	bool readKnowledge(std::string& path);

protected:
	std::string knowledgePath_;
	boost::shared_ptr<SemanticSpace> pSSpace_;
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
