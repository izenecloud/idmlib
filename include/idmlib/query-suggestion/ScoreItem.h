/*
 * ScoreItem.h
 *
 *  Created on: 2010-4-27
 *      Author: jinglei
 */

#ifndef SCOREITEM_H_
#define SCOREITEM_H_

#include <util/izene_serialization.h>

namespace idmlib
{
template<typename KeyType>
class ScoreItem
{
public:
	ScoreItem()
	{
	}
	ScoreItem(KeyType itemId, float score) :
		itemId_(itemId),
		score_(score)
    {
    }
    ScoreItem(const std::pair<KeyType, float>& idScore):
    	itemId_(idScore.first),
    	score_(idScore.second)
    {
    }
	virtual ~ScoreItem()
	{

	}
    ScoreItem& operator=(const std::pair<KeyType, float>& idScore)
    {
        itemId_ = idScore.first;
        score_ = idScore.second;

        return *this;
    }
	bool operator<(const ScoreItem& rightItem) const
	{
		return score_>rightItem.score_;

	}

	KeyType itemId_;
	float score_;

private:
    // For serialization
    friend class boost::serialization::access;

    template<class Archive>
    void serialize( Archive & ar, const unsigned int version )
    {
        ar & itemId_;
        ar & score_;
    } // serialize()

    DATA_IO_LOAD_SAVE(ScoreItem, &itemId_&score_)
};

}

#endif /* SCOREITEM_H_ */

