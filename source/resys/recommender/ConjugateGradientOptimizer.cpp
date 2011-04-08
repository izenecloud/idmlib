/*
 * ConjugateGradientOptimizer.cpp
 *
 *  Created on: 2011-3-31
 *      Author: yode
 */
#include <idmlib/resys/recommender/ConjugateGradientOptimizer.h>

#include <vector>

NS_IDMLIB_RESYS_BEGIN

double ConjugateGradientOptimizer::CONVERGENCE_LIMIT = 0.1;
int ConjugateGradientOptimizer::MAX_ITERATIONS = 1000;
ConjugateGradientOptimizer::ConjugateGradientOptimizer()
{
    // TODO Auto-generated constructor stub

}

ConjugateGradientOptimizer::~ConjugateGradientOptimizer()
{
    // TODO Auto-generated destructor stub
}
/**
  * <p>
  * Conjugate gradient optimization. Matlab code:
  * </p>
  *
  * <p>
  *
  * <pre>
  * function [x] = conjgrad(A,b,x0)
  *   x = x0;
  *   r = b - A*x0;
  *   w = -r;
  *   for i = 1:size(A);
  *      z = A*w;
  *      a = (r'*w)/(w'*z);
  *      x = x + a*w;
  *      r = r - a*z;
  *      if( norm(r) < 1e-10 )
  *           break;
  *      end
  *      B = (r'*z)/(w'*z);
  *      w = -r + B*w;
  *   end
  * end
  * </pre>
  *
  * </p>
  *
  * @param matrix
  *          matrix nxn positions
  * @param b
  *          vector b, n positions
  * @return vector of n weights
  */

std::vector<double>& ConjugateGradientOptimizer::optimize(double** matrix, std::vector<double>& b)
{

    int k = b.size();

    std::vector<double> r;
    r.resize(k);
    std::vector<double> w;
    w.resize(k);
    std::vector<double> z;
    z.resize(k);

    std::vector<double> x (k,3.0/k);


    // r = b - A*x0;
    // w = -r;
    for (int i = 0; i < k; i++)
    {
        double v = 0.0;
        double* ai = matrix[i];
        for (int j = 0; j < k; j++)
        {
            v += ai[j] * x[j];
        }
        double ri = b[i] - v;
        r[i] = ri;
        w[i] = -ri;
    }

    for (int iteration = 0; iteration < MAX_ITERATIONS; iteration++)
    {

        // z = A*w;
        for (int i = 0; i < k; i++)
        {
            double v = 0.0;
            double* ai = matrix[i];
            for (int j = 0; j < k; j++)
            {
                v += ai[j] * w[j];
            }
            z[i] = v;
        }

        // a = (r'*w)/(w'*z);
        double anum = 0.0;
        double aden = 0.0;
        for (int i = 0; i < k; i++)
        {
            anum += r[i] * w[i];
            aden += w[i] * z[i];
        }
        double a = anum / aden;

        // x = x + a*w;
        // r = r - a*z;
        for (int i = 0; i < k; i++)
        {
            x[i] += a * w[i];
            r[i] -= a * z[i];
        }

        // stop when residual is close to 0
        double rdot = 0.0;
        for (int i = 0; i < k; i++)
        {
            double value = r[i];
            rdot += value * value;
        }
        if (rdot <= CONVERGENCE_LIMIT)
        {
            break;
        }

        // B = (r'*z)/(w'*z);
        double bnum = 0.0;
        double bden = 0.0;
        for (int i = 0; i < k; i++)
        {
            double zi = z[i];
            bnum += r[i] * zi;
            bden += w[i] * zi;
        }
        double B = bnum / bden;

        // w = -r + B*w;
        for (int i = 0; i < k; i++)
        {
            w[i] = -r[i] + B * w[i];
        }

    }

    return x;
}


NS_IDMLIB_RESYS_END
