/*
 * Similarity.h
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_SIMILARITY_H
#define IDMLIB_RESYS_SIMILARITY_H
#include <idmlib/resys/model/FileDataModel.h>

#include <cmath>
#include <set>

NS_IDMLIB_RESYS_BEGIN

class Similarity
{
public:
    Similarity(FileDataModel dataModel, bool centerData, bool weighted);
    virtual ~Similarity();
    ItemPreferenceArray getMostSimilarItems(uint32_t itemId, uint32_t topk, std::set<uint32_t> possibleItemIDs);
    double itemSimilarity(uint32_t itemID1, uint32_t itemID2);
    void buildSimMatrix();
private:

    double computeResult(int n, double sumXY, double sumX2, double sumY2, double sumXYdiff2);
    double normalizeWeightResult(double result, int count, int num);
    FileDataModel dataModel_;
    bool centerData_;
    bool weighted_;

    std::map<uint32_t,ItemPreferenceArray > simMatrix_;;

};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_SIMILARITY_H */
