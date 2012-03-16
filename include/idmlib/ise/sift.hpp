/*
* C++ Wrapper for SIFT feature extraction, got from http:://code.google.com/p/nise
*/
#ifndef IDMLIB_IISE_SIFT_HPP
#define IDMLIB_IISE_SIFT_HPP

#define cimg_display 0
#define cimg_debug 0
#define cimg_use_magick 0

#include <idmlib/ise/image/CImg.h>
#include <idmlib/ise/common.hpp>
#include <idmlib/lshkit.h>

namespace idmlib{ namespace ise{

static const unsigned MAX_FEATURES = 200;
static const unsigned MAX_HASH = 20000;
static const float MIN_ENTROPY = 4.4F;
static const float LSH_W = 8.0F;
static const float LOG_BASE = 0.001F;

using cimg_library::CImg;

class Sift
{
    static const unsigned DIM = 128;
    int O, S, omin;
    double edge_thresh;
    double peak_thresh;
    double magnif;
    double e_th;
    bool do_angle;
    unsigned R;

    std::vector<std::vector<float> > black_list;
    float threshold;

    bool checkBlackList (const std::vector<float> &desc)
    {
        lshkit::metric::l2<float> l2(Sift::dim());
        BOOST_FOREACH(const std::vector<float> &b, black_list)
        {
            float d = l2(&desc[0], &b[0]);
            if (d < threshold) return false;
        }
        return true;
    }
public:
    struct Feature
    {
        Region region;
        std::vector<float> desc;
    };

    static float entropy (const std::vector<float> &desc)
    {
        std::vector<unsigned> count(256);
        fill(count.begin(), count.end(), 0);
        BOOST_FOREACH(float v, desc)
        {
            unsigned c = unsigned(floor(v * 255 + 0.5));
            if (c < 256) count[c]++;
        }
        double e = 0;
        BOOST_FOREACH(unsigned c, count)
        {
            if (c > 0)
            {
                float pr = (float)c / (float)desc.size();
                e += -pr * log(pr)/log(2.0);
            }
        }
        return float(e);
    }

    Sift (int O_ = -1, int S_ = 3, int omin_ = -1,
          double et_ = -1, double pt_ = 3, double mag_ = -1, double e_th_ = 0, bool do_angle_ = true, unsigned R_ = 256)
        : O(O_), S(S_), omin(omin_), edge_thresh(et_), peak_thresh(pt_), magnif(mag_), e_th(e_th_), do_angle(do_angle_), R(R_)
    {
    }

    void setBlackList (const std::string &path, float t)
    {
        threshold = t;
        std::vector<float> tmp(Sift::dim());
        std::ifstream is(path.c_str(), std::ios::binary);
        while (is.read((char *)&tmp[0], sizeof(float) * Sift::dim()))
        {
            black_list.push_back(tmp);
        }
    }

    static unsigned dim ()
    {
        return DIM;
    }

    void extract(const CImg<float> &image, float scale, std::vector<Feature> *list);
};

static inline void binarify (const Sift::Feature &f, std::vector<char> *b, float qt)
{
    unsigned dim = (f.desc.size() + 7) / 8;
    b->resize(dim);
    unsigned i = 0, j = 0;
    BOOST_FOREACH(float v, f.desc)
    {
        if (j == 0)
        {
            b->at(i) = 0;
        }
        if (v >= qt) b->at(i) |= (1 << j);
        ++j;
        if (j == 8)
        {
            j = 0;
            ++i;
        }
    }
}

static const unsigned SAMPLE_RANDOM = 0;
static const unsigned SAMPLE_SIZE = 1;

static inline bool sift_by_size (const Sift::Feature &f1, const Sift::Feature &f2)
{
    return f1.region.r > f2.region.r;
}

static inline void SampleFeature (std::vector<Sift::Feature> *sift, unsigned count, unsigned method)
{
    if (sift->size() < count) return;
    if (method == SAMPLE_RANDOM)
    {
        std::random_shuffle(sift->begin(), sift->end());
    }
    else if (method == SAMPLE_SIZE)
    {
        std::sort(sift->begin(), sift->end(), sift_by_size);
    }
    else
    {
        BOOST_VERIFY(0);
    }
    sift->resize(count);
}

static inline void logscale (std::vector<Sift::Feature> *v, float l)
{
    float s = -log(l);
    BOOST_FOREACH(Sift::Feature &f, *v)
    {
        BOOST_FOREACH(float &d, f.desc)
        {
            if (d < l) d = l;
            d = (log(d) + s) / s;
            if (d > 1.0) d = 1.0;
        }
    }
}

}}

#endif
