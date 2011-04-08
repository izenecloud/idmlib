/*
 * KnnItemBasedRecommender.h
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_KNNITEMBASEDRECOMMENDER_H
#define IDMLIB_RESYS_KNNITEMBASEDRECOMMENDER_H

#include <idmlib/resys/model/FileDataModel.h>
#include <idmlib/resys/model/Prediction.h>
#include <idmlib/resys/recommender/ConjugateGradientOptimizer.h>
#include <idmlib/resys/similarity/Similarity.h>
#include  <boost/shared_ptr.hpp>

NS_IDMLIB_RESYS_BEGIN

class KnnItemBasedRecommender
{
public:
    KnnItemBasedRecommender(FileDataModel dataModel,uint32_t neighborhoodSize);
    virtual ~KnnItemBasedRecommender();
    double predict(uint32_t theUserID, uint32_t itemID);
    std::vector<Prediction>  predict(ItemPreferenceArray& itemPreferences);
    std::vector<double> getInterpolations(uint32_t itemID, uint32_t userID, std::vector<uint32_t> itemNeighborhood);
private:
    FileDataModel dataModel_;
    int neighborhoodSize_;
    boost::shared_ptr<Similarity> similarity_;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_KNNITEMBASEDRECOMMENDER_H */
