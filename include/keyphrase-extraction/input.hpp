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
#include <wiselib/ustring/UString.h>
#include <idm_types.h>

NS_IDMLIB_KPE_BEGIN

template < class IDManager >
class IDInputType
{
public:
    typedef uint32_t id_type;
    typedef uint32_t value_type;
    typedef std::vector<value_type> input_type;
    typedef wiselib::UString string_type;
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
    typedef wiselib::UString value_type;
    typedef std::vector<value_type> input_type;
    typedef wiselib::UString string_type;
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
