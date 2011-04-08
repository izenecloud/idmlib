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


void testKnn(string trainingfile, uint32_t userId, uint32_t itemId)
{
    string dataFile(trainingfile);
    FileDataModel trainingModel;
    trainingModel.buildDataModel(dataFile);

    KnnItemBasedRecommender knn(trainingModel,5);
    knn.predict(userId, itemId);
}

bool myfunction (Prediction p1,Prediction p2)
{
    return (p1.getRating().getAverageValue()>p2.getRating().getAverageValue());
}


void testKnnNew(char *trainingfile, char* testfile, char* resultfile)
{



    clock_t start=clock();
    string dataFile(trainingfile);
    FileDataModel trainingModel;
    trainingModel.buildDataModel(dataFile);
    KnnItemBasedRecommender test(trainingModel,5);
    //test.buildDiffMatrix();

    cout<<"buildDiffMatrix()  elaplsed "<<(clock()-start)/CLOCKS_PER_SEC<<" seconds."<<endl;
    string testFile(testfile);
    FileDataModel testModel;
    testModel.buildDataModel(testFile);


    string resultFile(resultfile);
    FileDataModel resultModel;
    resultModel.buildDataModel(resultFile);
    std::map<uint32_t, std::vector<ItemPreference> > itemPrefsResult =resultModel.getUserModel();



    //std::vector<Prediction> Predictions;
    clock_t start2=clock();
    std::map<uint32_t, std::vector<ItemPreference> > itemPrefs =testModel.getUserModel();
    std::map<uint32_t, std::vector<ItemPreference> >::iterator iter= itemPrefs.begin();
    float rems =0.0;
    for (; iter!=itemPrefs.end(); iter++)
    {
        ItemPreferenceArray userRating4 =iter->second;
#ifdef DEBUG
        for (int i=0;i<userRating4.size();i++)
        {
            cout<<"itemid is "<<userRating4[i].getItemId()<<", score is "<<userRating4[i].getValue()<<endl;
        }
#endif

        std::vector<Prediction> Predictions = test.predict(userRating4);
        cout<<"Predictions size is "<<Predictions.size()<<endl;

        std::sort(Predictions.begin(),Predictions.end(), myfunction);
#ifdef DEBUG
        std::vector<Prediction>::iterator iter2 = Predictions.begin();
        for (; iter2!=Predictions.end(); iter2++)
        {
            std::cout<<"Item " <<iter2->getItemId()<< " Rating: " << iter2->getRating().getAverageValue()<<std::endl;;

        }

#endif

        for (std::map<uint32_t, std::vector<ItemPreference> >::iterator iterResult= itemPrefsResult.begin();
                iterResult!=itemPrefsResult.end();
                iterResult++)
        {
            if (iterResult->first !=iter->first)
                continue;
            int count =0;

            ItemPreferenceArray userRating =iterResult->second;
            for (uint32_t i=0;i<userRating.size(); i++)
            {


                std::vector<Prediction>::iterator iter2 = Predictions.begin();

                for (; iter2!=Predictions.end(); iter2++)
                {
                    if (userRating[i].getItemId() == iter2->getItemId())

                    {
                        float diff =userRating[i].getValue()- iter2->getRating().getAverageValue();
                        rems+= fabs( diff );
                        if ( (userRating[i].getValue() < (iter2->getRating().getAverageValue()+0.8))

                                ||(userRating[i].getValue() > (iter2->getRating().getAverageValue()-0.8)))
                            count++;

                    }
                }


            }

            cout<<"result item count is "<<userRating.size()<<", and correct count is "<<count<<", precision is "<<count*1.0/userRating.size()<<endl;
            cout<<"rems is  "<<rems/Predictions.size()<<endl;
        }

    }

    /*double const nan1 = std::numeric_limits<double>::quiet_NaN();

    cout<<"isnan() is "<<std::isnan(nan1)<<endl;;*/


    cout<<"predict time elapsed "<<(clock()-start2)*1.0/CLOCKS_PER_SEC<<" millseconds."<<endl;


}


void testSlopeOne(char *trainingfile, char* testfile, char* resultfile)
{
    clock_t start=clock();
    string dataFile(trainingfile);
    FileDataModel trainingModel;
    trainingModel.buildDataModel(dataFile);
    SlopeOneRecommender test(trainingModel) ;
    test.buildDiffMatrix();

    cout<<"buildDiffMatrix()  elaplsed "<<(clock()-start)/CLOCKS_PER_SEC<<" seconds."<<endl;
    string testFile(testfile);
    FileDataModel testModel;
    testModel.buildDataModel(testFile);


    string resultFile(resultfile);
    FileDataModel resultModel;
    resultModel.buildDataModel(resultFile);
    std::map<uint32_t, std::vector<ItemPreference> > itemPrefsResult =resultModel.getUserModel();



    //std::vector<Prediction> Predictions;
    clock_t start2=clock();
    std::map<uint32_t, std::vector<ItemPreference> > itemPrefs =testModel.getUserModel();
    std::map<uint32_t, std::vector<ItemPreference> >::iterator iter= itemPrefs.begin();
    float rems =0.0;
    for (; iter!=itemPrefs.end(); iter++)
    {
        ItemPreferenceArray userRating4 =iter->second;
        std::vector<Prediction> Predictions = test.predict(userRating4);
        cout<<"Predictions size is "<<Predictions.size()<<endl;

        std::sort(Predictions.begin(),Predictions.end(), myfunction);
#ifdef DEBUG
        std::vector<Prediction>::iterator iter2 = Predictions.begin();
        for (; iter2!=Predictions.end(); iter2++)
        {
            std::cout<<"Item " <<iter2->getItemId()<< " Rating: " << iter2->getRating().getAverageValue()<<std::endl;;

        }

#endif

        for (std::map<uint32_t, std::vector<ItemPreference> >::iterator iterResult= itemPrefsResult.begin();
                iterResult!=itemPrefsResult.end();
                iterResult++)
        {
            if (iterResult->first !=iter->first)
                continue;
            int count =0;

            ItemPreferenceArray userRating =iterResult->second;
            for (uint32_t i=0;i<userRating.size(); i++)
            {


                std::vector<Prediction>::iterator iter2 = Predictions.begin();

                for (; iter2!=Predictions.end(); iter2++)
                {
                    if (userRating[i].getItemId() == iter2->getItemId())

                    {
                        float diff =userRating[i].getValue()- iter2->getRating().getAverageValue();
                        rems+= fabs( diff );
                        if ( (userRating[i].getValue() < (iter2->getRating().getAverageValue()+0.8))

                                ||(userRating[i].getValue() > (iter2->getRating().getAverageValue()-0.8)))
                            count++;

                    }
                }


            }

            cout<<"result item count is "<<userRating.size()<<", and correct count is "<<count<<", precision is "<<count*1.0/userRating.size()<<endl;
            cout<<"rems is  "<<rems/Predictions.size()<<endl;
        }


    }

    /*double const nan1 = std::numeric_limits<double>::quiet_NaN();

    cout<<"isnan() is "<<std::isnan(nan1)<<endl;;*/











    cout<<"predict time elapsed "<<(clock()-start2)*1.0/CLOCKS_PER_SEC<<" millseconds."<<endl;


}
int main(int argc, char *argv[])
{
    /* uint32_t userId=3;
     uint32_t itemId=3;
     //testKnn(argv[1], userId, itemId);*/

    testSlopeOne(argv[1],argv[2],argv[3]);
    testKnnNew(argv[1],argv[2],argv[3]);
}

