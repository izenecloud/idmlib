/*
 * SlopeOne.h
 *
 *  Created on: 2011-3-24
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_SLOPEONE_H
#define IDMLIB_RESYS_SLOPEONE_H
#include <idmlib/resys/model/FileDataModel.h>
#include <idmlib/resys/model/Prediction.h>
#include <idmlib/resys/model/ItemPreference.h>
#include <idmlib/resys/model/Rating.h>

#include <vector>
#include <map>
#include <set>
#include <iostream>

NS_IDMLIB_RESYS_BEGIN

typedef unsigned int ItemId;
typedef unsigned int UserId;
class SlopeOneRecommender
{
public:
    SlopeOneRecommender(FileDataModel& data);

    ~SlopeOneRecommender();
    void buildDiffMatrix( );
    std::vector<Prediction> predict(ItemPreferenceArray& itemPreferences);

private:

    // The Storage to keep the difference matrix
    std::map<std::pair<unsigned int, unsigned int>, Rating > diffStorage_;

    FileDataModel data_;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_SLOPEONE_H */
