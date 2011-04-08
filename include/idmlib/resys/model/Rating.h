/*
 * Rating.h
 *
 *  Created on: 2011-3-25
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_RATING_H
#define IDMLIB_RESYS_RATING_H

class Rating
{
public:
    Rating() {};
    Rating(double value, unsigned int freq):value_(value),freq_(freq) {};
    virtual ~Rating() {}
    unsigned int getFreq()
    {
        return freq_;
    }
    float getValue()
    {
        return value_;
    }
    void setFreq(unsigned int freq)
    {
        freq_ = freq;
    }
    void setValue(float value)
    {
        value_ = value;
    }
    float getAverageValue()
    {
        return value_/freq_;
    }
    Rating& operator=(const Rating& rating)
    {

        this->value_ = rating.value_;
        this->freq_=rating.freq_;
        return *this;
    }



private:
    double value_;
    unsigned int freq_;

};

#endif /* IDMLIB_RESYS_RATING_H */
