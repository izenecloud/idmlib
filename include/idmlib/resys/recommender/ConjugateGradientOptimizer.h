/*
 * ConjugateGradientOptimizer.h
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */

#ifndef IDMLIB_RESYS_CONJUGATEGRADIENTOPTIMIZER_H
#define IDMLIB_RESYS_CONJUGATEGRADIENTOPTIMIZER_H
#include <vector>

#include <idmlib/idm_types.h>

NS_IDMLIB_RESYS_BEGIN

class ConjugateGradientOptimizer
{
public:
    ConjugateGradientOptimizer();
    virtual ~ConjugateGradientOptimizer();
    static std::vector<double>& optimize(double** matrix, std::vector<double>& b) ;

private:
    static double CONVERGENCE_LIMIT ;
    static  int MAX_ITERATIONS ;
};

NS_IDMLIB_RESYS_END

#endif /* IDMLIB_RESYS_CONJUGATEGRADIENTOPTIMIZER_H */
