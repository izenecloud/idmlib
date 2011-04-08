/*
 * Prediction.h
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_PREDICTION_H_
#define IDMLIB_RESYS_PREDICTION_H_

#include <idmlib/idm_types.h>
#include <idmlib/resys/model/Rating.h>

NS_IDMLIB_RESYS_BEGIN

class Prediction
{
public:
    Prediction(uint32_t itemId, Rating rating):itemId_(itemId),rating_(rating)
    {}

    virtual ~Prediction() {}

    uint32_t getItemId()
    {
        return this->itemId_;
    }

    Rating& getRating()
    {
        return this->rating_;
    }
private:
    uint32_t itemId_;

    Rating rating_;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_PREDICTION_H_ */
