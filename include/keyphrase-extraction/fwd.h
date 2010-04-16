///
/// @file fwd.h
/// @brief Farword head file.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef _IDMKPEFWD_H_
#define _IDMKPEFWD_H_

#include "input.hpp"
#include "output.hpp"
#include "algorithm.hpp"

NS_IDMLIB_KPE_BEGIN

template <class IDManagerType>
class KPE_ALL : public Algorithm1< IDManagerType , OutputType<true, true> >
{
typedef Algorithm1< IDManagerType , OutputType<true, true> > BaseType;
typedef OutputType<true, true> ThisOutputType;

public:
typedef ThisOutputType::function_type function_type;
    KPE_ALL(IDManagerType* idManager, function_type func, const std::string& working)
    : BaseType( idManager, ThisOutputType(func), working)
    {
    }
};

template <class IDManagerType>
class KPE_DF : public Algorithm1< IDManagerType , OutputType<true, false> >
{
typedef Algorithm1< IDManagerType , OutputType<true, false> > BaseType;
typedef OutputType<true, false> ThisOutputType;
public:
typedef ThisOutputType::function_type function_type;
    KPE_DF(IDManagerType* idManager, function_type func, const std::string& working)
    : BaseType( idManager, ThisOutputType(func), working)
    {
    }
};

template <class IDManagerType>
class KPE_NONE : public Algorithm1< IDManagerType , OutputType<false, false> >
{
typedef Algorithm1< IDManagerType , OutputType<false, false> > BaseType;

typedef OutputType<false, false> ThisOutputType;
public:
typedef ThisOutputType::function_type function_type;
    KPE_NONE(IDManagerType* idManager, function_type func, const std::string& working)
    : BaseType( idManager, ThisOutputType(func), working)
    {
    }
};

NS_IDMLIB_KPE_END

#endif 
