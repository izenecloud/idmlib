/*
* Extractor, got from http:://code.google.com/p/nise
* It has following utilities:
* 1. Load image
* 2. SIFT feature extraction
* 3. SIFT feature filtering
* 4. SIFT feature log-scaling
* 5. L2 sketching
* Detailed algorithms for step 3,4 could be seen in Wei Dong's phd dissertation:
* High-Dimensional Similarity Search for Large Datasets. Chapter 6, A Large-Scale Image Search Engine. 2011
* Detailed L2 sketching algorithm could be seen in Wei Dong's publication:
* Asymmetric Distance Estimation with Sketches for Similarity Search in High-Dimensional Spaces. 2008
*/
#ifndef IDMLIB_ISE_EXTRACTOR_HPP
#define IDMLIB_ISE_EXTRACTOR_HPP

#include <idmlib/ise/common.hpp>
#include <idmlib/ise/sift.hpp>

namespace idmlib { namespace ise{

struct ExtractorImpl;

class Extractor {
    ExtractorImpl *impl;
public:
    Extractor ();

    ~Extractor ();

    void ExtractSift(const std::string &path, std::vector<Sift::Feature>& sift, bool query = true);

    void BuildSketch(std::vector<Sift::Feature>& sift, std::vector<Sketch>& sketches);
};

}}

#endif
