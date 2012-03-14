#ifndef IDMLIB_ISE_COMMON_HPP
#define IDMLIB_ISE_COMMON_HPP

#include <idmlib/idm_types.h>

#include <string>
#include <vector>
#include <exception>

#include <boost/assert.hpp>
#include <boost/foreach.hpp>

namespace idmlib{ namespace ise{
static const unsigned SKETCH_BIT = 128;
static const unsigned SKETCH_SIZE = 128/8;
static const unsigned SKETCH_PLAN_DIST = 3;
static const unsigned SKETCH_DIST = 4; // accept dist < 4
static const unsigned SKETCH_TIGHT_DIST = 3; // accept dist < 4
static const unsigned SKETCH_DIST_OFFLINE = 3;

static const unsigned MIN_IMAGE_SIZE = 50;
static const unsigned MIN_QUERY_IMAGE_SIZE = 20;
static const unsigned QUERY_IMAGE_SCALE_THRESHOLD = 80;
static const unsigned MAX_IMAGE_SIZE = 250;

typedef uint8_t Chunk;
typedef Chunk Sketch[SKETCH_SIZE];

struct Region
{
    float x, y, r, t;
};

struct Feature
{
    Sketch sketch;
};

// An indexed record.
struct Record
{
    struct Meta
    {
        unsigned width;
        unsigned height;
        unsigned size;
    };
    struct Source
    {
        std::string url;
        std::string parentUrl;
    };

    Meta meta;
    std::string checksum;
    std::string thumbnail;
    std::vector<Region> regions;
    std::vector<Feature> features;
    std::vector<Source> sources;

    void clear ()
    {
        meta.width = meta.height = meta.size = 0;
        checksum.clear();
        thumbnail.clear();
        regions.clear();
        features.clear();
        sources.clear();
    }

    void swap (Record &r)
    {
        Meta tmp = meta;
        meta = r.meta;
        r.meta = tmp;
        checksum.swap(r.checksum);
        thumbnail.swap(r.thumbnail);
        regions.swap(r.regions);
        features.swap(r.features);
        sources.swap(r.sources);
    }

};


}}

#endif
