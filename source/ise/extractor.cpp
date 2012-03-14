#include <idmlib/ise/extractor.hpp>
#include <idmlib/ise/sift.hpp>
#include <idmlib/ise/imgutil.hpp>

namespace idmlib{ namespace ise{

struct ExtractorImpl
{
    typedef lshkit::Sketch<lshkit::DeltaLSB<lshkit::GaussianLsh> > LshSketch;
    LshSketch sketch;
    Sift xtor;
public:
    ExtractorImpl ()
        : xtor(-1, 3, 0, -1, 3, -1, MIN_ENTROPY, true)
    {
        lshkit::DefaultRng rng;
        LshSketch::Parameter pm;
        pm.W = LSH_W;
        pm.dim = 128; // Sift::dim();
        sketch.reset(SKETCH_SIZE, pm, rng);
    }

    void extract (const std::string &path, Record *record, bool query)
    {
        record->clear();

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

        std::vector<Sift::Feature> sift;
        CImg<float> gray;

        CImgToGray(im, &gray);

        float scale = CImgLimitSize(&gray, MAX_IMAGE_SIZE);
        if (query)
        {
            scale *= CImgLimitSizeBelow(&gray, QUERY_IMAGE_SCALE_THRESHOLD);
        }

        xtor.extract(gray, scale, &sift);

        record->meta.width = im.width();
        record->meta.height = im.height();

        if (LOG_BASE > 0)
        {
            logscale(&sift, LOG_BASE);
        }

        SampleFeature(&sift, MAX_FEATURES, SAMPLE_SIZE);

        record->regions.resize(sift.size());
        record->features.resize(sift.size());
        for (unsigned i = 0; i < sift.size(); ++i)
        {
            record->regions[i] = sift[i].region;
            sketch.apply(&sift[i].desc[0], record->features[i].sketch);
        }

    }
};

Extractor::Extractor (): impl(new ExtractorImpl)
{
}

void Extractor::extract (const std::string &image, Record *record, bool query)
{
    return impl->extract(image, record, query);
}

Extractor::~Extractor ()
{
    delete impl;
}

}}

