/**
 * @file explicit_semantic_interpreter.h
 * @author Zhongxia Li
 * @date Mar 10, 2011
 * @brief 
 */
#ifndef EXPLICIT_SEMANTIC_INTERPRETER_H_
#define EXPLICIT_SEMANTIC_INTERPRETER_H_

#include <idmlib/idm_types.h>
#include <idmlib/semantic_space/semantic_interpreter.h>

NS_IDMLIB_SSP_BEGIN

class ExplicitSemanticInterpreter : public SemanticInterpreter
{
public:
	bool interpret(std::string& text, std::vector& interVector);
};

NS_IDMLIB_SSP_END

#endif /* EXPLICIT_SEMANTIC_INTERPRETER_H_ */
