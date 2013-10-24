#ifndef ADPREDICTOR_H_
#define ADPREDICTOR_H_

#include <utility>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <types.h>
#include <ir/be_index/AVMapper.hpp>
#include <ir/be_index/SimpleSerialization.hpp>
#include <3rdparty/json/json.h>

using namespace izenelib::ir::be_index;

namespace idmlib {

class AdPredictor {
public:
    AdPredictor()
    {
    }

    AdPredictor(double m, double v, double b, double f): default_mean(m), default_variance(v), beta(b), forget_rate(f)
    {
    }

    void update(const std::vector<std::pair<std::string, std::string> > & assignment, bool clicked)
    {
        std::vector<std::pair<uint32_t, uint32_t> > pos(assignment.size());
        double sum_of_mean = 0.0;
        double sum_of_variance = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.insert(assignment[i].first, assignment[i].second);
            pos[i].first = avResult.first.first;
            pos[i].second = avResult.second.first;

            if (avResult.first.second == true) {    // new attribute
                weights.resize(weights.size() + 1);
                weights.back().push_back(std::make_pair(default_mean, default_variance));
            } else if (avResult.second.second == true) {   // old attribute, but new value
                weights[pos[i].first].push_back(std::make_pair(default_mean, default_variance));
            }

            sum_of_mean += weights[pos[i].first][pos[i].second].first;
            sum_of_variance += weights[pos[i].first][pos[i].second].second;
        }

        double upper_sigma_square = beta * beta + sum_of_variance;
        double upper_sigma = sqrt(upper_sigma_square);
        int flag = (clicked ? 1 : (-1));
        double temp = flag * sum_of_mean / upper_sigma;
        for (std::size_t i = 0; i != assignment.size(); ++i) {
            weights[pos[i].first][pos[i].second].first += flag * (weights[pos[i].first][pos[i].second].second / upper_sigma) * v(temp);
            weights[pos[i].first][pos[i].second].second *= 1 - (weights[pos[i].first][pos[i].second].second / upper_sigma_square) * w(temp);
        }
    }

    void forget()
    {
        for (std::size_t i = 0; i != weights.size(); ++i) {
            for (std::size_t j = 0; j != weights[i].size(); ++j) {
                double new_variance = default_variance * weights[i][j].second / ((1 - forget_rate) * default_variance + forget_rate * weights[i][j].second);
                double new_mean = new_variance * ((1 - forget_rate) * weights[i][j].first / weights[i][j].second + forget_rate * default_mean / default_variance);
                weights[i][j].first = new_mean;
                weights[i][j].second = new_variance;
            }
        }
    }

    double predict(const std::vector<std::pair<std::string, std::string> > & assignment)
    {
        double sum_of_mean = 0.0;
        double sum_of_variance = 0.0;

        for (std::size_t i = 0; i != assignment.size(); ++i) {
            std::pair<std::pair<uint32_t, bool>, std::pair<uint32_t, bool> > avResult = avMapper.has(assignment[i].first, assignment[i].second);
            if (!(avResult.first.second == true && avResult.second.second == true)) {
                continue;
            }
            sum_of_mean += weights[avResult.first.first][avResult.second.first].first;
            sum_of_variance += weights[avResult.first.first][avResult.second.first].second;
        }

        double upper_sigma_square = beta * beta + sum_of_variance;
        double upper_sigma = sqrt(upper_sigma_square);
        double temp = 1 * sum_of_mean / upper_sigma;
        return (1.0 + erf(temp / sqrt(2))) / 2.0;
    }

    void toJson(Json::Value & root)
    {
        weightsToJson(root["weights"]);
        avMapper.toJson(root["avMapper"]);
        root["default_mean"] = default_mean;
        root["default_variance"] = default_variance;
        root["beta"] = beta;
        root["forget_rate"] = forget_rate;
    }

    void fromJson(Json::Value & root)
    {
        weightsFromJson(root["weights"]);
        avMapper.fromJson(root["avMapper"]);
        default_mean = root["default_mean"].asDouble();
        default_variance = root["default_variance"].asDouble();
        beta = root["beta"].asDouble();
        forget_rate = root["forget_rate"].asDouble();
    }

    void save(std::ostream & os)
    {
        Json::FastWriter writer;
        Json::Value root;
        this->toJson(root);
        os << writer.write(root);
    }

    void load(std::istream & is)
    {
        Json::Reader reader;
        Json::Value root;
        bool successful = reader.parse(is, root);
        if (!successful) {
            std::cout << "json parse error" << std::endl;
            return;
        }
        fromJson(root);
    }

    void save_binary(std::ostream & os)
    {
        serialize(weights.size(), os);
        for (std::size_t i = 0; i != weights.size(); ++i) {
            serialize(weights[i].size(), os);
            for (std::size_t j = 0; j != weights[i].size(); ++j) {
                serialize(weights[i][j].first, os);
                serialize(weights[i][j].second, os);
            }
        }

        avMapper.save_binary(os);

        serialize(default_mean, os);
        serialize(default_variance, os);
        serialize(beta, os);
        serialize(forget_rate, os);
    }

    void load_binary(std::istream & is)
    {
        std::size_t rowNum;
        deserialize(is, rowNum);
        weights.resize(rowNum);
        for (std::size_t i = 0; i != rowNum; ++i) {
            std::size_t colNum;
            deserialize(is, colNum);
            weights[i].resize(rowNum);
            for (std::size_t j = 0; j != colNum; ++j) {
                deserialize(is, weights[i][j].first);
                deserialize(is, weights[i][j].second);
            }
        }

        avMapper.load_binary(is);

        deserialize(is, default_mean);
        deserialize(is, default_variance);
        deserialize(is, beta);
        deserialize(is, forget_rate);
    }

private:
    void weightsToJson(Json::Value & root)
    {
        for (std::size_t i = 0; i != weights.size(); ++i) {
            rowWeightsToJson(root[i], weights[i]);
        }
    }

    void rowWeightsToJson(Json::Value & root, std::vector<std::pair<double, double> > & rowWeights)
    {
        for (std::size_t i = 0; i != rowWeights.size(); ++i) {
            weightPairToJson(root[i], rowWeights[i]);
        }
    }

    void weightPairToJson(Json::Value & root, std::pair<double, double> & weightPair)
    {
        root[Json::Value::UInt(0U)] = weightPair.first;
        root[Json::Value::UInt(1U)] = weightPair.second;
    }

    void weightsFromJson(Json::Value & root)
    {
        weights.resize(root.size());
        for (std::size_t i = 0; i != root.size(); ++i) {
            rowWeightsFromJson(root[i], weights[i]);
        }
    }

    void rowWeightsFromJson(Json::Value & root, std::vector<std::pair<double, double> > & rowWeights)
    {
        rowWeights.resize(root.size());
        for (std::size_t i = 0; i != root.size(); ++i) {
            weightPairFromJson(root[i], rowWeights[i]);
        }
    }

    void weightPairFromJson(Json::Value & root, std::pair<double, double> & weightPair)
    {
        weightPair.first = root[Json::Value::UInt(0U)].asDouble();
        weightPair.second = root[Json::Value::UInt(1U)].asDouble();
    }

    double v(double x) const
    {
        return sqrt(2 / M_PI) * exp(- x * x / 2) / (1 + erf(x / sqrt(2)));
    }

    double w(double x) const
    {
        double temp = v(x);
        return temp * (temp + x);
    }

    // each pair is a (mean, variance) pair
    std::vector<std::vector<std::pair<double, double> > > weights;
    AVMapper avMapper;

    double default_mean;
    double default_variance;
    double beta;

    double forget_rate;
};

}

#endif
