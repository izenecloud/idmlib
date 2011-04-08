/**
 * <p>
 * An implementation of the Pearson correlation. For users X and Y, the following values are calculated:
 * </p>
 *
 * <ul>
 * <li>sumX2: sum of the square of all X's preference values</li>
 * <li>sumY2: sum of the square of all Y's preference values</li>
 * <li>sumXY: sum of the product of X and Y's preference value for all items for which both X and Y express a
 * preference</li>
 * </ul>
 *
 * <p>
 * The correlation is then:
 *
 * <p>
 * <code>sumXY / sqrt(sumX2 * sumY2)</code>
 * </p>
 *
 * <p>
 * Note that this correlation "centers" its data, shifts the user's preference values so that each of their
 * means is 0. This is necessary to achieve expected behavior on all data sets.
 * </p>
 *
 * <p>
 * This correlation implementation is equivalent to the cosine similarity since the data it receives
 * is assumed to be centered -- mean is 0. The correlation may be interpreted as the cosine of the angle
 * between the two vectors defined by the users' preference values.
 * </p>
 *
 * <p>
 * For cosine similarity on uncentered data, see {@link UncenteredCosineSimilarity}.
 * </p>
 */

#include <idmlib/resys/similarity/Similarity.h>

NS_IDMLIB_RESYS_BEGIN

Similarity::Similarity(FileDataModel dataModel, bool centerData, bool weighted)
        :dataModel_(dataModel)
        ,centerData_(centerData)
        ,weighted_(weighted)
{
    // TODO Auto-generated constructor stub

}

Similarity::~Similarity()
{
    // TODO Auto-generated destructor stub
}
double Similarity::itemSimilarity(uint32_t itemID1, uint32_t itemID2)
{
    UserPreferenceArray xPrefs = dataModel_.getUserPreferencesForItem(itemID1);
    UserPreferenceArray yPrefs = dataModel_.getUserPreferencesForItem(itemID2);
    uint32_t xLength = xPrefs.size();
    uint32_t yLength = yPrefs.size();
    //cout<<"xLength is "<<xLength<<",yLength is  "<<yLength<<endl;
    if ((xLength == 0) || (yLength == 0))
    {
        return std::numeric_limits<double>::quiet_NaN();
    }

    uint32_t xIndex = xPrefs[0].getUserId();
    uint32_t yIndex = yPrefs[0].getUserId();
    uint32_t xPrefIndex = 0;
    uint32_t yPrefIndex = 0;

    double sumX = 0.0;
    double sumX2 = 0.0;
    double sumY = 0.0;
    double sumY2 = 0.0;
    double sumXY = 0.0;
    double sumXYdiff2 = 0.0;
    int count = 0;


    while (true)
    {
        int compare = xIndex < yIndex ? -1 : xIndex > yIndex ? 1 : 0;
        if (compare == 0)
        {
            // Both users expressed a preference for the item
            double x = xPrefs[xPrefIndex].getValue();

            double y = yPrefs[yPrefIndex].getValue();
            // cout<<"x is "<<x<<", y is "<<y<<endl;
            sumXY += x * y;
            sumX += x;
            sumX2 += x * x;
            sumY += y;
            sumY2 += y * y;
            double diff = x - y;
            sumXYdiff2 += diff * diff;
            count++;
        }
        if (compare <= 0)
        {
            if (++xPrefIndex == xLength)
            {
                break;
            }
            xIndex = xPrefs[xPrefIndex].getUserId();
        }
        if (compare >= 0)
        {
            if (++yPrefIndex == yLength)
            {
                break;
            }
            yIndex = yPrefs[yPrefIndex].getUserId();
        }
    }

    double result;
    if (centerData_)
    {
        // See comments above on these computations
        double n = (double) count;
        double meanX = sumX / n;
        double meanY = sumY / n;
        // double centeredSumXY = sumXY - meanY * sumX - meanX * sumY + n * meanX * meanY;
        double centeredSumXY = sumXY - meanY * sumX;
        // double centeredSumX2 = sumX2 - 2.0 * meanX * sumX + n * meanX * meanX;
        double centeredSumX2 = sumX2 - meanX * sumX;
        // double centeredSumY2 = sumY2 - 2.0 * meanY * sumY + n * meanY * meanY;
        double centeredSumY2 = sumY2 - meanY * sumY;
        result = computeResult(count, centeredSumXY, centeredSumX2, centeredSumY2, sumXYdiff2);
    }
    else
    {

        result = computeResult(count, sumXY, sumX2, sumY2, sumXYdiff2);
    }

    /*if (similarityTransform != null) {
      result = similarityTransform.transformSimilarity(itemID1, itemID2, result);
    }*/

    if (!std::isnan(result))
    {
        result = normalizeWeightResult(result, count, dataModel_.getUserNums());
    }
    return result;
}
double Similarity::computeResult(int n, double sumXY, double sumX2, double sumY2, double sumXYdiff2)
{
    if (n == 0)
    {
        return std::numeric_limits<double>::quiet_NaN();
    }
    // Note that sum of X and sum of Y don't appear here since they are assumed to be 0;
    // the data is assumed to be centered.
    double denominator = std::sqrt(sumX2) * std::sqrt(sumY2);

    if (denominator == 0.0)
    {
        // One or both parties has -all- the same ratings;
        // can't really say much similarity under this measure
        return std::numeric_limits<double>::quiet_NaN();
    }
    return sumXY / denominator;
}
double Similarity::normalizeWeightResult(double result, int count, int num)
{
    if (weighted_)
    {
        double scaleFactor = 1.0 - (double) count / (double) (num + 1);
        if (result < 0.0)
        {
            result = -1.0 + scaleFactor * (1.0 + result);
        }
        else
        {
            result = 1.0 - scaleFactor * (1.0 - result);
        }
    }
    // Make sure the result is not accidentally a little outside [-1.0, 1.0] due to rounding:
    if (result < -1.0)
    {
        result = -1.0;
    }
    else if (result > 1.0)
    {
        result = 1.0;
    }
    return result;
}

void Similarity::buildSimMatrix()
{
    std::set<uint32_t> items = dataModel_.getItems();
    double sim =0;
    std::set<uint32_t>::iterator iter1 = items.begin();
    for (; iter1!=items.end(); iter1++)
    {
        std::set<uint32_t>::iterator iter2 = items.begin();
        for (; iter2!=items.end(); iter2++)
        {

            if (*iter1 == *iter2)
                continue;

            sim = itemSimilarity(*iter1, *iter2);
#ifdef DEBUG
            cout<<"itemID1 is "<<*iter1<<" and ItemID2 "<<*iter2<<" 's similarity is "<<sim<<endl;
#endif
            ItemPreference itemPref(*iter2,sim);
            ItemPreferenceArray itemPrefs;
            if (simMatrix_.find(*iter1)!=simMatrix_.end())
            {

                itemPrefs = simMatrix_.find(*iter1)->second;
                itemPrefs.push_back(itemPref);

            }
            else
            {

                itemPrefs.push_back(itemPref);

            }
            simMatrix_[*iter1]=itemPrefs;

        }
    }
}
bool myfunction (ItemPreference p1,ItemPreference p2)
{
    return (p1.getValue()>p2.getValue());
}
ItemPreferenceArray Similarity::getMostSimilarItems(uint32_t itemId, uint32_t topk,std::set<uint32_t> possibleItemIDs)
{
    ItemPreferenceArray simItemArray;
    ItemPreferenceArray tempSimItemArray;
    int count =0;
    if (simMatrix_.find(itemId)!=simMatrix_.end())
    {
        ItemPreferenceArray itemPrefs =simMatrix_.find(itemId)->second;

        std::sort(itemPrefs.begin(), itemPrefs.end(),myfunction);
        for (uint32_t i=0; i<itemPrefs.size(); i++)
        {
            uint32_t itemId = itemPrefs[i].getItemId();
            if (possibleItemIDs.find(itemId) != possibleItemIDs.end())
            {
                simItemArray.push_back(itemPrefs[i]);
                count++;
            }
            else
                continue;
        }
#ifdef DEBUG
        cout<<"topk is "<<topk<<", and simItemArray size is "<<simItemArray.size()<<endl;
#endif
        topk = simItemArray.size()<topk?  simItemArray.size():topk;
        for (uint32_t i=0; i<topk; i++)
        {
#ifdef DEBUG
            cout<<"MostSimilarItem is "<<simItemArray[i].getItemId()<<" "<<simItemArray[i].getValue()<<endl;
#endif
            tempSimItemArray.push_back(simItemArray[i]);
        }
        return tempSimItemArray;
    }
    return tempSimItemArray;

}

NS_IDMLIB_RESYS_END


