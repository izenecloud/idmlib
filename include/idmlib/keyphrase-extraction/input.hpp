///
/// @file input.hpp
/// @brief indicate the interfaces of input class for keyphrase extraction algorithm.
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2010-04-08
/// @date Updated 2010-04-08
///

#ifndef _IDMKPEINPUT_HPP_
#define _IDMKPEINPUT_HPP_

#include <string>
#include <vector>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <util/ustring/UString.h>
#include "../idm_types.h"

NS_IDMLIB_KPE_BEGIN

struct TERM_TAG
{
    static const char CHN = 'C';
    static const char ENG = 'F';
    static const char KOR = '?';
    static const char KOR_NOUN = 'N';
    static const char KOR_LOAN = 'Z';
    static const char KOR_COMP_NOUN = 'P';
    static const char NUM = 'S';
    static const char OTHER = '@';
};

class Term
{
    public:
        Term()
        {
        }
        
        Term(const izenelib::util::UString& text, uint32_t id, char tag, uint32_t position)
        :text_(text), id_(id), tag_(tag), position_(position)
        {
        }
        
        Term(const izenelib::util::UString& text, char tag, uint32_t position)
        :text_(text), id_(0), tag_(tag), position_(position)
        {
        }
        
    public:
        izenelib::util::UString text_;
        mutable uint32_t id_;
        char tag_;
        uint32_t position_;
};

template < class IDManager >
class IDInputType
{
public:
    typedef uint32_t id_type;
    typedef uint32_t value_type;
    typedef std::vector<value_type> input_type;
    typedef izenelib::util::UString string_type;
    typedef IDManager IDManagerType;
    IDInputType(IDManager* idManager): idManager_(idManager)
    {
    }
    
    void getIdByValue(const value_type& value, id_type& id)
    {
        id = value;
    }
    
    void getIdByValue(const value_type& value, char pos, id_type& id)
    {
        id = value;
    }
    
    void put(id_type id, const string_type& str)
    {
        return idManager_->put(id, str);
    }
    
    bool getStringById(id_type id, string_type& str)
    {
        return idManager_->getTermStringByTermId(id, str);
    }
    
    bool isKeyPhrase(id_type id)
    {
        //to do
        return false;
    }
    
    IDManager* getIDManager()
    {
        return idManager_;
    }
    
private:    
    IDManager* idManager_;
};

template < class IDManager >
class StringInputType
{
public:
    typedef uint32_t id_type;
    typedef izenelib::util::UString value_type;
    typedef std::vector<value_type> input_type;
    typedef izenelib::util::UString string_type;
    typedef IDManager IDManagerType;
    StringInputType(IDManager* idManager): idManager_(idManager)
    {
    }
    
    void getIdByValue(const value_type& value, id_type& id)
    {
        idManager_->getTermIdByTermString(value, id);
    }
    
    void getIdByValue(const value_type& value, char pos, id_type& id)
    {
        idManager_->getTermIdByTermString(value, pos, id);
    }
    
    void put(id_type id, const string_type& str)
    {
        return idManager_->put(id, str);
    }
    
    bool getStringById(id_type id, string_type& str)
    {
        return idManager_->getTermStringByTermId(id, str);
    }
    
    bool isKeyPhrase(id_type id)
    {
        //to do
        return false;
    }
    
    IDManager* getIDManager()
    {
        return idManager_;
    }
    
private:   
    IDManager* idManager_;
};



NS_IDMLIB_KPE_END

#endif 
