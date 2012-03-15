#include <idmlib/ise/extractor.hpp>
#include <idmlib/ise/imgutil.hpp>

namespace idmlib{ namespace ise{

struct ExtractorImpl
{
    typedef lshkit::Sketch<lshkit::DeltaLSB<lshkit::GaussianLsh> > LshSketch;
    LshSketch sketch_;
    Sift xtor_;
public:
    ExtractorImpl ()
        : xtor_(-1, 3, 0, -1, 3, -1, MIN_ENTROPY, true)
    {
        lshkit::DefaultRng rng;
        LshSketch::Parameter pm;
        pm.W = LSH_W;
        pm.dim = 128;
        sketch_.reset(SKETCH_SIZE, pm, rng);
    }

    void ExtractSift(const std::string &path, std::vector<Sift::Feature>& sift, bool query)
    {
        if (path.empty()) return;

        CImg<unsigned char> im(path.c_str());

        if (query)
        {
            if (lshkit::min(im.height(), im.width()) < int(MIN_QUERY_IMAGE_SIZE)) return;
        }
        else
        {
            if (lshkit::min(im.height(), im.width()) < int(MIN_IMAGE_SIZE)) return;
        }

        CImg<float> gray;

        CImgToGray(im, &gray);

        float scale = CImgLimitSize(&gray, MAX_IMAGE_SIZE);
        if (query)
        {
            scale *= CImgLimitSizeBelow(&gray, QUERY_IMAGE_SCALE_THRESHOLD);
        }

        xtor_.extract(gray, scale, &sift);
        SampleFeature(&sift, MAX_FEATURES, SAMPLE_SIZE);
    }

    void BuildSketch(std::vector<Sift::Feature>& sift, std::vector<Sketch >& sketches)
    {
        sketches.resize(sift.size());
        for (unsigned i = 0; i < sift.size(); ++i)
        {
            sketch_.apply(&sift[i].desc[0], sketches[i].desc);
        }
    }
};

Extractor::Extractor (): impl(new ExtractorImpl)
{
}

void Extractor::ExtractSift(const std::string &path, std::vector<Sift::Feature>& sift, bool query)
{
    impl->ExtractSift(path, sift, query);
}

void Extractor::BuildSketch(std::vector<Sift::Feature>& sift, std::vector<Sketch>& sketches)
{
    impl->BuildSketch(sift, sketches);
}

Extractor::~Extractor ()
{
    delete impl;
}

}}

