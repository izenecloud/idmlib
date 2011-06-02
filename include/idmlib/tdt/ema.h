///
/// @file ema.h
/// @brief For ema computation
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-05-23
/// @date Updated 2011-05-23
///

#ifndef _IDMLIB_TDT_EMA_H_
#define _IDMLIB_TDT_EMA_H_

#include "../idm_types.h"
#include <algorithm>
#include <cmath>

NS_IDMLIB_TDT_BEGIN

template <uint8_t N, typename T=uint32_t>
class Ema
{
    public:
        
        Ema()
        {
            std::vector<double> init(1, 1.0);
            Ema_(N, init, coef_);
        }
        
        static void Ema_(uint8_t n, const std::vector<double>& input, std::vector<double>& output)
        {
            output.resize(input.size()+n-1, 0.0);
            double a = 2.0/(n+1);
            double a1 = 1-a;
            for(uint32_t i=0;i<input.size();i++)
            {
                double base = input[i];
                for(uint j=0;j<n;j++)
                {
                    output[i+j] += base * a * std::pow(a1, (double)j);
                }
            }
        }
        
        
        
        void Compute(const std::vector<T>& source, std::vector<double>& result)
        {
//             std::cout<<"before compute"<<std::endl;
            result.resize(source.size(), 0.0);
            uint32_t G = 0;
            for(uint32_t i=0;i<source.size();i++)
            {
                uint32_t count = (i+1) > coef_.size()?coef_.size():(i+1);
//                 uint32_t gap = coef_.size()-count;
                if(count<=G)
                {
                    result[i] = 0.0;
                    continue;
                }
                
                for(uint32_t j=0;j<count;j++)
                {
                    result[i] += coef_[j] * source[i-j];
                }
            }
//             std::cout<<"after compute"<<std::endl;
        }
        
        void Print()
        {
            PrintVec_(coef_);
        }
        
    private:
        template <typename U>
        void PrintVec_(const std::vector<U>& vec)
        {
            for(uint32_t i=0;i<vec.size();i++)
            {
                std::cout<<vec[i]<<",";
            }
            std::cout<<std::endl;
        }
        
    private:
        
        std::vector<double> coef_;
};

NS_IDMLIB_TDT_END

#endif 
