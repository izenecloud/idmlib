/*
 * KnnItemBasedRecommender.cpp
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */

#include <idmlib/resys/recommender/KnnItemBasedRecommender.h>

NS_IDMLIB_RESYS_BEGIN

KnnItemBasedRecommender::KnnItemBasedRecommender(FileDataModel dataModel,uint32_t neighborhoodSize): dataModel_(dataModel),neighborhoodSize_(neighborhoodSize)
{
    // TODO Auto-generated constructor stub
    similarity_ .reset(new Similarity(dataModel_,false,false));
    similarity_->buildSimMatrix();

}

KnnItemBasedRecommender::~KnnItemBasedRecommender()
{
    // TODO Auto-generated destructor stub
}

std::vector<double> KnnItemBasedRecommender::getInterpolations(uint32_t itemID, uint32_t userID, std::vector<uint32_t> itemNeighborhood)
{
    int k = itemNeighborhood.size();

    double** aMatrix = new double*[k];
    for (int i=0; i<k; i++)
    {
        aMatrix[i] = new double[k];
    }
    std::vector<double> b(k,0);

    int i=0;
    int numUsers = dataModel_.getUserNums();
    for (uint32_t ii =0; ii<k; ii++)
    {
        uint32_t iitem = itemNeighborhood[ii];
        UserPreferenceArray iPrefs = dataModel_.getUserPreferencesForItem(iitem);
        int iSize = iPrefs.size();
        int j=0;
        for (uint32_t jj =0; jj<k; jj++)
        {
            uint32_t jitem = itemNeighborhood[jj];
            double value = 0.0;
            for (int pi = 0; pi < iSize; pi++)
            {
                uint32_t v = iPrefs[pi].getUserId();
                if (v == userID)
                {
                    continue;
                }
                double pj = dataModel_.getPreferenceValue(userID, jitem);
                if (pj != 0)
                {
                    value += iPrefs[pi].getValue() * pj;
                }
            }
            aMatrix[i][j] = value / numUsers;
            j++;
        }
        i++;
    }

    UserPreferenceArray iPrefs = dataModel_.getUserPreferencesForItem(itemID);
    int iSize = iPrefs.size();
    i = 0;
    for (uint32_t jj =0; jj<k; jj++)
    {
        uint32_t jitem = itemNeighborhood[jj];
        double value = 0.0;
        for (int pi = 0; pi < iSize; pi++)
        {
            uint32_t v = iPrefs[pi].getUserId();
            if (v == userID)
            {
                continue;
            }
            float pj = dataModel_.getPreferenceValue(userID, jitem);
            if (pj != 0)
            {
                value += iPrefs[pi].getValue() * pj;
            }
        }
        b[i] = value / numUsers;
        i++;
    }

    return ConjugateGradientOptimizer::optimize(aMatrix, b);

}

double KnnItemBasedRecommender::predict(uint32_t theUserID, uint32_t itemID)
{


    ItemPreferenceArray prefs = dataModel_.getItemPreferencesForUser(theUserID);
    uint32_t size = prefs.size();
    //items that the user has preferred
    std::set<uint32_t> possibleItemIDs;
    for (uint32_t i = 0; i < size; i++)
    {
        uint32_t olditemId = prefs[i].getItemId();
        if (olditemId != itemID)
            possibleItemIDs.insert(olditemId);
    }

    ItemPreferenceArray mostSimilar= similarity_->getMostSimilarItems( itemID,  neighborhoodSize_,possibleItemIDs);


    int i = 0;
    double preference = 0.0;
    double totalSimilarity = 0.0;
    for (int jitem=0; jitem<mostSimilar.size(); jitem++)
    {
        double pref=dataModel_.getPreferenceValue(theUserID, jitem);
        if (pref > 0)
        {
            cout<<pref<<" "<<mostSimilar[jitem].getValue()<<endl;
            preference += pref * mostSimilar[jitem].getValue();
            totalSimilarity +=mostSimilar[jitem].getValue();
        }

    }
    /*std::vector<uint32_t> theNeighborhood ;
    for (int i=0; i< mostSimilar.size(); i++)
    {
        theNeighborhood.push_back(mostSimilar[i].getItemId());
    }


    std::vector<double> weights = getInterpolations(itemID, theUserID, theNeighborhood);

    int i = 0;
    double preference = 0.0;
    double totalSimilarity = 0.0;
    for (int jitem=0; jitem<theNeighborhood.size(); jitem++)
    {
        double pref=dataModel_.getPreferenceValue(theUserID, jitem);
        if (pref != 0)
        {
            preference += pref * weights[i];
            totalSimilarity += weights[i];
        }
        i++;
    }*/

    totalSimilarity == 0.0 ?std::numeric_limits<double>::quiet_NaN() : (double) (preference / totalSimilarity);
    cout<<"predict simi is "<<totalSimilarity<<endl;
    return totalSimilarity;
}

NS_IDMLIB_RESYS_END
