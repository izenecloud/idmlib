/*
 * Preference.h
 *
 *  Created on: 2011-3-28
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_ITEM_PREFERENCE_H
#define IDMLIB_RESYS_ITEM_PREFERENCE_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

class ItemPreference
{
public:
    ItemPreference(uint32_t itemId, float value):itemId_(itemId), value_(value)
    {}
    virtual ~ItemPreference() {}
    uint32_t getItemId()
    {
        return itemId_;
    }
    void setItemId(uint32_t itemId)
    {
        this->itemId_ = itemId;
    }
    float getValue()
    {
        return value_;
    }
    void setValue(float value)
    {
        this->value_ = value;
    }
    uint32_t getTimeStamp()
    {
        return this->timestamp_;
    }
    void setTimeStamp(uint32_t timeStamp)
    {
        this->timestamp_ = timeStamp;
    }
private:
    unsigned int itemId_;
    float value_;
    unsigned int timestamp_;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_ITEM_PREFERENCE_H */
