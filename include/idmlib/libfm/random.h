#ifndef RANDOM_H_
#define RANDOM_H_

#include <cstdlib>
#include <cmath>

namespace idmlib {

double ran_gaussian();
double ran_gaussian(double mean, double stdev);

double ran_uniform();

double ran_gamma(double alpha);
double ran_gamma(double alpha, double beta);

double ran_gaussian()
{
    // Joseph L. Leva: A fast normal Random number generator
    double u, v, x, y, Q;
    do {
        do {
            u = ran_uniform();
        } while (u == 0.0);
        v = 1.7156 * (ran_uniform() - 0.5);
        x = u - 0.449871;
        y = std::abs(v) + 0.386595;
        Q = x * x + y * (0.19600 * y- 0.25472 * x);
        if (Q < 0.27597) {
            break;
        }
    } while ((Q > 0.27846) || ((v * v) > (-4.0 * u * u * std::log(u))));
    return v / u;
}

double ran_gaussian(double mean, double stdev)
{
    if ((stdev == 0.0) || (std::isnan(stdev))) {
        return mean;
    } else {
        return mean + stdev * ran_gaussian();
    }
}

double ran_uniform()
{
    return rand() / ((double)RAND_MAX + 1);
}

double ran_gamma(double alpha)
{
    if (alpha < 1.0) {
        double u;
        do {
            u = ran_uniform();
        } while (u == 0.0);
        return ran_gamma(alpha + 1.0) * pow(u, 1.0 / alpha);
    } else {
        // Marsaglia and Tsang: A Simple Method for Generating Gamma Variables
        double d, c, x, v, u;
        d = alpha - 1.0 / 3.0;
        c = 1.0 / std::sqrt(9.0 * d);
        do {
            do {
                x = ran_gaussian();
                v = 1.0 + c * x;
            } while (v <= 0.0);
            v = v * v * v;
            u = ran_uniform();
        } while (
                 (u >= (1.0 - 0.0331 * (x*x) * (x*x)))
                 && (log(u) >= (0.5 * x * x + d * (1.0 - v + std::log(v))))
                  );
        return d * v;
    }
}

double ran_gamma(double alpha, double beta) {
    return ran_gamma(alpha) / beta;
}

}

#endif
