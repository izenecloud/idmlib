#ifndef FTRL_H_
#define FTRL_H_

#include <ir/be_index/AVMapper.hpp>
#include <vector>
#include <cmath>
#include <utility>
#include <string>
#include <types.h>
#include <ir/be_index/SimpleSerialization.hpp>
#include <iostream>

using namespace izenelib::ir::be_index;

namespace idmlib {

class FTRLweights {
public:
    FTRLweights()
    {
        z = 0.0;
        n = 0.0;
    }

    void save_binary(std::ostream & os)
    {
        serialize(w, os);
        serialize(g, os);
        serialize(delta, os);
        serialize(z, os);
        serialize(n, os);
    }

    void load_binary(std::istream & is)
    {
        deserialize(is, w);
        deserialize(is, g);
        deserialize(is, delta);
        deserialize(is, z);
        deserialize(is, n);
    }

    double w;
    double g;
    double delta;
    double z;
    double n;
};

class FTRL {
public:
    FTRL()
    {
    }

    FTRL(double alpha, double beta, double lambda_1, double lambda_2)
        : alpha(alpha), beta(beta), lambda_1(lambda_1), lambda_2(lambda_2)
    {
    }

    void update(const std::vector<std::pair<std::string, std::string> > & assignment, bool clicked)
    {
        std::vector<std::pair<uint32_t, uint32_t> > pos(assignment.size());
        double sum_of_w = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.insert(assignment[i].first, assignment[i].second);
            pos[i].first = avResult.first.first;
            pos[i].second = avResult.second.first;
            bool newAttribute = avResult.first.second;
            bool newValue = avResult.second.second;

            if (newAttribute) {    // new attribute
                weights.resize(weights.size() + 1);
                weights.back().push_back(FTRLweights());
            } else if (newValue) {   // old attribute, but new value
                weights[pos[i].first].push_back(FTRLweights());
            }

            FTRLweights & unit = weights[pos[i].first][pos[i].second];
            getW(unit);
            sum_of_w += unit.w;
        }

        double p = sigmoid(sum_of_w);

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            FTRLweights & unit = weights[pos[i].first][pos[i].second];
            unit.g = p - clicked ? 1.0 : 0.0;
            unit.delta = (sqrt(unit.n + unit.g * unit.g) - sqrt(unit.n)) / alpha;
            unit.z += unit.g - unit.delta * unit.w;
            unit.n += unit.g * unit.g;
        }
    }

    double predict(const std::vector<std::pair<std::string, std::string> > & assignment)
    {
        double sum_of_w = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.has(assignment[i].first, assignment[i].second);
            if (!(avResult.first.second == true && avResult.second.second == true)) {
                continue;
            }
            sum_of_w += weights[avResult.first.first][avResult.second.first].w;
        }

        return sigmoid(sum_of_w);
    }

    void save_binary(std::ostream & os)
    {
        serialize(alpha, os);
        serialize(beta, os);
        serialize(lambda_1, os);
        serialize(lambda_2, os);

        avMapper.save_binary(os);

        serialize(weights.size(), os);
        for (std::size_t i = 0; i != weights.size(); ++i) {
            serialize(weights[i].size(), os);
            for (std::size_t j = 0; j != weights[i].size(); ++j) {
                weights[i][j].save_binary(os);
            }
        }
    }

    void load_binary(std::istream & is)
    {
        deserialize(is, alpha);
        deserialize(is, beta);
        deserialize(is, lambda_1);
        deserialize(is, lambda_2);

        avMapper.load_binary(is);

        std::size_t rowNum;
        deserialize(is, rowNum);
        weights.resize(rowNum);
        for (std::size_t i = 0; i != rowNum; ++i) {
            std::size_t colNum;
            deserialize(is, colNum);
            weights[i].resize(colNum);
            for (std::size_t j = 0; j != colNum; ++j) {
                weights[i][j].load_binary(is);
            }
        }
    }

private:
    void getW(FTRLweights & unit)
    {
        if (unit.z <= lambda_1 && unit.z >= -lambda_1) {
            unit.w = 0.0;
            return;
        }
        // otherwise...
        if (unit.z > 0) {
            unit.w = -(unit.z - lambda_1) / ((beta + sqrt(unit.n)) / alpha + lambda_2);
        } else {
            unit.w = -(unit.z + lambda_1) / ((beta + sqrt(unit.n)) / alpha + lambda_2);
        }
    }

    double sigmoid(double x)
    {
        return 1.0 / (1.0 + exp(-x));
    }

    double alpha;
    double beta;
    double lambda_1;
    double lambda_2;

    AVMapper avMapper;
    std::vector<std::vector<FTRLweights> > weights;
};

}

#endif
