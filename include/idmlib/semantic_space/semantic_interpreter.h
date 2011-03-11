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

#include <idmlib/idm_types.h>

NS_IDMLIB_SSP_BEGIN

class SemanticInterpreter
{
public:
	SemanticInterpreter()
	: pSSpace_(NULL)
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
	virtual bool interpret(std::string& text, std::vector& interVector) = 0;


protected:
	boost::scoped_ptr<SemanticSpace> pSSpace_;
};

NS_IDMLIB_SSP_END

#endif /* SEMANTIC_INTERPRETER_H_ */
