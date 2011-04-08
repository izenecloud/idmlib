/*
 * SlopeOne.cpp
 *
 *  Created on: 2011-3-24
 *      Author: yode
 */

#include <idmlib/resys/recommender/SlopeOneRecommender.h>

#include <limits>
#include <cmath>

using namespace std;

NS_IDMLIB_RESYS_BEGIN

SlopeOneRecommender::SlopeOneRecommender(FileDataModel& data)
{
    this->data_ = data;
}

SlopeOneRecommender::~SlopeOneRecommender()
{

}

void SlopeOneRecommender::buildDiffMatrix( )
{
    std::map<uint32_t,ItemPreferenceArray>::iterator iter = data_.getUserModel().begin();
    //for every user
    for (; iter != data_.getUserModel().end(); iter++)
    {
        ItemPreferenceArray itemPreferences = iter->second;
        int length = itemPreferences.size();
        //for every item in user
        for (int i = 0; i < length; i++)
        {
            uint32_t item1Id = itemPreferences[i].getItemId();
            float item1Rating = itemPreferences[i].getValue();


            for (int j=0; j<length; j++)
            {
                uint32_t item2Id = itemPreferences[j].getItemId();
                if (item2Id <= item1Id)//we only store half of matrix, another half *(-1)
                    continue;
                float item2Rating = itemPreferences[j].getValue();

                Rating ratingDiff(0,0);
                std::pair<uint32_t, uint32_t> item_pair (item1Id, item2Id);
                if (diffStorage_.find(item_pair)!=diffStorage_.end())//if exists, update
                {
                    ratingDiff = diffStorage_.find(item_pair)->second;

                }
                ratingDiff.setValue(ratingDiff.getValue()+ item1Rating-item2Rating);
                ratingDiff.setFreq( ratingDiff.getFreq()+1);

                diffStorage_[item_pair]=ratingDiff;


            }
        }
    }
#ifdef DEBUG
    std::map<std::pair<unsigned int, unsigned int>, Rating >::iterator iterDiff = diffStorage_.begin();
    for (; iterDiff!=diffStorage_.end(); iterDiff++)
    {
        std::cout<< (iterDiff->first).first <<":"<< (iterDiff->first).second<<":"<<iterDiff->second.getAverageValue()<<":"<<iterDiff->second.getFreq()<<std::endl;
    }
#endif

}

std::vector<Prediction> SlopeOneRecommender:: predict(ItemPreferenceArray& itemPreferences)
{


    std::vector<Prediction> Predictions;
    std::set<uint32_t> items = data_.getItems();
    std::set<uint32_t> preferredItemIds;
    int length = itemPreferences.size();
    for (int k = 0; k < length; k++)
    {

        uint32_t inputItemId = itemPreferences[k].getItemId();
        preferredItemIds.insert(inputItemId);
    }
#ifdef DEBUG
    cout<<"user preferred item set size is "<<preferredItemIds.size()<<endl;
    cout<<"all of the items size is "<<items.size()<<endl;
#endif
    std::set<uint32_t>::iterator iter = items.begin();
    // all of the items
    for (; iter!=items.end(); iter++)
    {
        //if the item is is in user preferred item set, it means users has visited it, skip it.
        if (preferredItemIds.find(*iter) != preferredItemIds.end())
        {
            continue;
        }
        Rating itemRating(0,0) ;
        //new user's  input items
        for (int i = 0; i < length; i++)
        {

            ItemPreference itemPrefer = itemPreferences[i];
            uint32_t inputItemId = itemPreferences[i].getItemId();

            Rating diff(0,0);

            if (*iter >inputItemId) //reverse, as we only store half of mattrix
            {
                diff = diffStorage_.find(make_pair(inputItemId, *iter))->second;
                itemRating.setValue(itemRating.getValue()+  (diff.getAverageValue()*(-1.0)+ itemPrefer.getValue()) *diff.getFreq());
                itemRating.setFreq(itemRating.getFreq()+diff.getFreq());
            }

            else
            {
                diff = diffStorage_.find(make_pair(*iter, inputItemId))->second;
                itemRating.setValue( itemRating.getValue()+ (diff.getAverageValue()+ itemPrefer.getValue()) *diff.getFreq());
                itemRating.setFreq(itemRating.getFreq()+diff.getFreq());
            }

        }
        if (!std::isnan(itemRating.getAverageValue()))
        {
            Prediction prediction(*iter,itemRating);
            Predictions.push_back(prediction);
        }


    }
#ifdef DEBUG
    cout<<"Predictions size is "<<Predictions.size()<<endl;
    std::vector<Prediction>::iterator iter2 = Predictions.begin();
    for (; iter2!=Predictions.end(); iter2++)
    {
        std::cout<<"Item " << iter2->getItemId() << " Rating: " << iter2->getRating().getAverageValue()<<std::endl;;
    }
#endif
    return Predictions;
}

NS_IDMLIB_RESYS_END

