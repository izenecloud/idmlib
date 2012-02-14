#ifndef IDMLIB_SIM_STRINGSIMILARITY_H_
#define IDMLIB_SIM_STRINGSIMILARITY_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
NS_IDMLIB_SIM_BEGIN


class StringSimilarity
{
typedef izenelib::util::UString UString;
typedef izenelib::util::UCS2Char UChar;

struct Temp
{
};

public:
    StringSimilarity(const UString& x)
    :x_(x)
    {
    }

    double Sim(const UString& y)
    {
        static double length_weight = 0.8;
        uint32_t match_length = 0;
        uint32_t match_count = 0;
        std::size_t iy = 0;
        while(true)
        {
            if(iy>=y.length()) break;
            UString m;
            GetMatch_(y, iy, m);
            if(!m.empty())
            {
                match_length += m.length();
                match_count++;
                iy += m.length();
            }
            else
            {
                iy++;
            }
        }
        double sim = 0.0;
        if(match_count>0)
        {
            sim = (double)match_length/y.length()*length_weight + 1.0/match_count*(1.0-length_weight);
        }
        return sim;
    }

    static double Sim(const UString& x, const UString& y)
    {
        StringSimilarity ssx(x);
        double sx = ssx.Sim(y);
        StringSimilarity ssy(y);
        double sy = ssy.Sim(x);
        return 0.5*sx+0.5*sy;

    }

private:

    void GetMatch_(const UString& y, std::size_t iy, UString& m)
    {
        for(std::size_t ix = 0;ix<x_.length();ix++)
        {
            UString im;
            GetMatch_(ix, y, iy, im);
            if(!im.empty())
            {
                if(im.length()>m.length())
                {
                    m = im;
                }
            }
        }

    }

    void GetMatch_(std::size_t ix, const UString& y, std::size_t iy, UString& m)
    {
        if(ix>=x_.length() || iy>=y.length()) return;
        if(x_[ix]==y[iy])
        {
            m += x_[ix];
            GetMatch_(ix+1, y, iy+1, m);
        }
    }

private:
    UString x_;
  
};

   
NS_IDMLIB_SIM_END

#endif 

