///
/// @file macd_histogram.h
/// @brief For macd computation
/// @author Jia Guo <guojia@gmail.com>
/// @date Created 2011-04-19
/// @date Updated 2011-04-19
///

#ifndef _IDMLIB_TDT_MACDHISTOGRAM_H_
#define _IDMLIB_TDT_MACDHISTOGRAM_H_

#include "../idm_types.h"
#include "../util/Util.hpp"
#include <boost/tuple/tuple.hpp>
#include <algorithm>
#include <cmath>
#include <am/external_sort/izene_sort.hpp>
#include <am/sequence_file/SimpleSequenceFile.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>
NS_IDMLIB_TDT_BEGIN

template <uint8_t S, uint8_t L, uint8_t M, uint32_t G = L+M-2>
class MacdHistogram
{
    public:
        const static uint32_t G_Value = G;
        MacdHistogram()
        {
            std::vector<double> init(1, 1.0);
            std::vector<double> emas;
            std::vector<double> emal;
            Ema_(S, init, emas);
            Ema_(L, init, emal);
            macd_coef_.assign(emas.begin(), emas.end());
//             std::vector<double> macd(emas);
            AddTo_(macd_coef_, emal, -1.0);
            PrintVec_(emas);
            PrintVec_(emal);
            PrintVec_(macd_coef_);
            std::vector<double> signal;
            Ema_(M, macd_coef_, signal);
            PrintVec_(signal);
            std::vector<double> histogram(macd_coef_);
            AddTo_(histogram, signal, -1.0);
            coef_ = histogram;
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
        
        static void AddTo_(std::vector<double>& to, const std::vector<double>& from, double x)
        {
            uint32_t count = to.size()>from.size()?from.size():to.size();
            for(uint32_t i=0;i<count;i++)
            {
                to[i] += from[i]*x;
            }
            uint32_t to_size = to.size();
            for(uint32_t i=to_size;i<from.size();i++)
            {
                to.push_back(from[i]*x);
            }
        }
        
        template <typename T>
        void Compute(const std::vector<T>& source, std::vector<double>& result)
        {
            Compute_(coef_, source, result);
        }
        
        template <typename T>
        void ComputeMacd(const std::vector<T>& source, std::vector<double>& result)
        {
            Compute_(macd_coef_, source, result);
        }
        
        
        
        void Print()
        {
            PrintVec_(coef_);
        }
        
    
        
    private:
        
        template <typename T>
        void Compute_(const std::vector<double>& coef, const std::vector<T>& source, std::vector<double>& result)
        {
//             std::cout<<"before compute"<<std::endl;
            result.resize(source.size(), 0.0);
            for(uint32_t i=0;i<source.size();i++)
            {
                uint32_t count = (i+1) > coef.size()?coef.size():(i+1);
//                 uint32_t gap = coef_.size()-count;
                if(count<=G)
                {
                    result[i] = 0.0;
                    continue;
                }
                
                for(uint32_t j=0;j<count;j++)
                {
                    result[i] += coef[j] * source[i-j];
                }
            }
//             std::cout<<"after compute"<<std::endl;
        }
        
        template <typename T>
        void PrintVec_(const std::vector<T>& vec)
        {
            for(uint32_t i=0;i<vec.size();i++)
            {
                std::cout<<vec[i]<<",";
            }
            std::cout<<std::endl;
        }
        
    private:
        
        std::vector<double> coef_;
        std::vector<double> macd_coef_;
};

NS_IDMLIB_TDT_END

#endif 
