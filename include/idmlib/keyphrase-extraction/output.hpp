///
/// @file output.hpp
/// @brief indicate the interfaces of output class for keyphrase extraction algorithm.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef _IDMKPEOUTPUT_HPP_
#define _IDMKPEOUTPUT_HPP_

#include <string>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <wiselib/ustring/UString.h>
#include "../idm_types.h"

NS_IDMLIB_KPE_BEGIN

template <bool DF = true, bool DOC = true>
class OutputType
{
typedef std::pair<uint32_t, uint32_t> id2count_t;
public:
    typedef wiselib::UString string_type;
    typedef boost::function<void (const string_type&, const std::vector<id2count_t>&, uint8_t) > function_type;
    enum {NEED_DF = true};
    enum {NEED_DOC = true};
    OutputType(function_type function)
    {
        function_ = function;
    }
    
    void output(const string_type& str, const std::vector<id2count_t>& id2countList, uint32_t df, uint8_t score)
    {
        function_(str, id2countList, score);
    }
private:    
    function_type function_;
};

template <>
class OutputType<false, false>
{
typedef std::pair<uint32_t, uint32_t> id2count_t;
public:
    typedef wiselib::UString string_type;
    typedef boost::function<void (const string_type&, uint8_t) > function_type;
    enum {NEED_DF = false};
    enum {NEED_DOC = false};
    OutputType(function_type function)
    {
        function_ = function;
    }
    
    void output(const string_type& str, const std::vector<id2count_t>& id2countList, uint32_t df, uint8_t score)
    {
        function_(str, score);
    }
private:    
    function_type function_;
};

template <>
class OutputType<true, false> 
{
typedef std::pair<uint32_t, uint32_t> id2count_t;
public:
    typedef wiselib::UString string_type;
    typedef boost::function<void (const string_type&, uint32_t, uint8_t) > function_type;
    enum {NEED_DF = true};
    enum {NEED_DOC = false};
    OutputType(function_type function)
    {
        function_ = function;
    }
    
    void output(const string_type& str, const std::vector<id2count_t>& id2countList, uint32_t df, uint8_t score)
    {
        function_(str, df, score);
    }
private:    
    function_type function_;
};


NS_IDMLIB_KPE_END

#endif 
