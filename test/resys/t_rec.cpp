/*
 * t_rec.cpp
 *
 *  Created on: 2011-3-28
 *      Author: yode
 */

#include <idmlib/resys/recommender/SlopeOneRecommender.h>
#include <idmlib/resys/recommender/KnnItemBasedRecommender.h>
#include <idmlib/resys/similarity/Similarity.h>

#include <vector>
#include <iostream>
#include <string>
#include <limits>
#include <cmath>
#include <time.h>

using namespace std;
using namespace idmlib::recommender;


void testItemSimilarity(string trainingfile, uint32_t itemId1, uint32_t itemId2)
{
    string dataFile(trainingfile);
    FileDataModel trainingModel;
    trainingModel.buildDataModel(dataFile);

    /*Similarity similarity(trainingModel,false,false);
    similarity.buildSimMatrix();
    similarity.getMostSimilarItems(1,5);
    double result12 = similarity.itemSimilarity(1,2)*1.0;
    double result13 = similarity.itemSimilarity(1,3)*1.0;
    double result14 = similarity.itemSimilarity(1,4)*1.0;
    cout<<result12<<" "<<result13<<" "<<result14<<endl;*/


    KnnItemBasedRecommender knn(trainingModel,5);
    knn.predict(3, 3);
}


bool myfunction (Prediction p1,Prediction p2)
{
    return (p1.getRating().getAverageValue()>p2.getRating().getAverageValue());
}

void testSlopeOne(char *trainingfile, char* testfile)
{

    //string dataFile("/home/yode/workspace/recommender/data/test.txt");

    clock_t start=clock();
    string dataFile(trainingfile);
    FileDataModel trainingModel;
    trainingModel.buildDataModel(dataFile);
    SlopeOneRecommender test(trainingModel) ;
    test.buildDiffMatrix();
    cout<<"CLOCKS_PER_SEC is "<<CLOCKS_PER_SEC<<endl;
    cout<<"buildDiffMatrix()  elaplsed "<<(clock()-start)/CLOCKS_PER_SEC<<" seconds."<<endl;
    string testFile(testfile);
    FileDataModel testModel;
    testModel.buildDataModel(testFile);
    clock_t start2=clock();
    std::map<uint32_t, std::vector<ItemPreference> > itemPrefs =testModel.getUserModel();
    std::map<uint32_t, std::vector<ItemPreference> >::iterator iter= itemPrefs.begin();
    for (; iter!=itemPrefs.end(); iter++)
    {
        ItemPreferenceArray userRating4 =iter->second;
        std::vector<Prediction> Predictions = test.predict(userRating4);
        cout<<"Predictions size is "<<Predictions.size()<<endl;
        std::vector<Prediction>::iterator iter2 = Predictions.begin();
        std::sort(Predictions.begin(),Predictions.end(), myfunction);
       /* for (; iter2!=Predictions.end(); iter2++)
        {
            std::cout<<"Item " <<iter2->getItemId()<< " Rating: " << iter2->getRating().getAverageValue()<<std::endl;;
        }*/
    }

    /*double const nan1 = std::numeric_limits<double>::quiet_NaN();

    cout<<"isnan() is "<<std::isnan(nan1)<<endl;;*/
    cout<<"predict time elapsed "<<(clock()-start2)*1.0/CLOCKS_PER_SEC<<" millseconds."<<endl;


}



int main(int argc, char *argv[])
{
    uint32_t itemId1=1;
    uint32_t itemId2=2;
    //testItemSimilarity(argv[1], itemId1, itemId2);

    testSlopeOne(argv[1],argv[2]);
}

