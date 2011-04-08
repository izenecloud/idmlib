/*
 * Preference.h
 *
 *  Created on: 2011-3-28
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_USER_PREFERENCE_H
#define IDMLIB_RESYS_USER_PREFERENCE_H

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

class UserPreference
{
public:
    UserPreference(uint32_t userId, float value):userId_(userId), value_(value)
    {
    }
    virtual ~UserPreference() {}
    uint32_t getUserId()
    {
        return userId_;
    }
    void setUserId(uint32_t userId)
    {
        this->userId_ = userId;
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
    unsigned int userId_;
    float value_;
    unsigned int timestamp_;
};

NS_IDMLIB_RESYS_END
#endif /* IDMLIB_RESYS_USER_PREFERENCE_H */
