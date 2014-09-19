///
/// @file similb.h
/// @brief similb algorithm
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-06-08
/// @date Updated 2011-06-08
///

#ifndef _IDMLIB_TDT_SIMILB_H_
#define _IDMLIB_TDT_SIMILB_H_

#include "../idm_types.h"

#include <algorithm>
#include <cmath>

#include "ema.h"
#include "tdt_types.h"
NS_IDMLIB_TDT_BEGIN

class SimilB
{
    public:
        static const double SQRT_2 ;
        
        template<typename T>
        static void GetPsi(const std::vector<T>& s1, const std::vector<T>& s2, std::size_t start, std::size_t end, std::vector<double>& result)
        {
            if(s1.size()!=s2.size()) return;
            std::size_t count = end-start;
            if( count<3 ) return;
            result.resize(count, 0.0);
            for(std::size_t i=start+1; i<end-1; i++)
            {
                std::size_t p = i-1;
                std::size_t n = i+1;
                result[i-start] = (double)s1[i] * s2[i] - 0.5 * (s1[p]*s2[n] + s1[n]*s2[p]);
            }
        }
        
        template<typename T>
        static void GetPsi(const std::vector<T>& s1, const std::vector<T>& s2, std::vector<double>& result)
        {
            GetPsi(s1, s2, 0, s1.size(), result);
        }

        template<typename T>
        static double Sim(const std::vector<T>& s1, const std::vector<T>& s2)
        {
            return Sim(s1, s2, 0, s1.size());
        }
        
        template<typename T>
        static double Sim(const std::vector<T>& s1, const std::vector<T>& s2, std::size_t start, std::size_t end)
        {
            if(s1.size()!=s2.size()) return 0.0;
            std::size_t count = end-start;
            if( count<3 ) return 0.0;
            double denominator = 0.0;
            double nominator = 0.0;
            for(std::size_t i=start+1; i<end-1; i++)
            {
                std::size_t p = i-1;
                std::size_t n = i+1;
                double temp = (double)s1[i] * s2[i] - 0.5 * (s1[p]*s2[n] + s1[n]*s2[p]);
                temp = fabs(temp);
                nominator += temp;
                double t1 = (double)s1[i]*s1[i] - s1[p]*s1[n];
                double t2 = (double)s2[i]*s2[i] - s2[p]*s2[n];
                double squaresum = sqrt(t1*t1 + t2*t2);
                denominator += squaresum;
            }
            return (SQRT_2 * nominator) / denominator;
        }
        
};

NS_IDMLIB_TDT_END

#endif 
