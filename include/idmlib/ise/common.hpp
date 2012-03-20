#ifndef IDMLIB_ISE_COMMON_HPP
#define IDMLIB_ISE_COMMON_HPP

#include <idmlib/idm_types.h>

#include <string>
#include <vector>
#include <exception>

#include <boost/assert.hpp>
#include <boost/foreach.hpp>

namespace idmlib{ namespace ise{

typedef uint8_t Chunk;

static const unsigned SKETCH_BIT = 128;
static const unsigned SKETCH_SIZE = SKETCH_BIT / (sizeof(Chunk) * 8);
static const unsigned SKETCH_PLAN_DIST = 3;
static const unsigned SKETCH_DIST = 4; // accept dist < 4
static const unsigned SKETCH_TIGHT_DIST = 3; // accept dist < 4
static const unsigned SKETCH_DIST_OFFLINE = 3;

static const unsigned MIN_IMAGE_SIZE = 50;
static const unsigned MIN_QUERY_IMAGE_SIZE = 20;
static const unsigned QUERY_IMAGE_SCALE_THRESHOLD = 80;
static const unsigned MAX_IMAGE_SIZE = 250;

typedef Chunk SketchData[SKETCH_SIZE];

static const unsigned CHUNK_BIT = sizeof(Chunk) * 8;
static const unsigned DATA_CHUNK = 128 / CHUNK_BIT;

struct Sketch
{
    SketchData desc;

    int compare(const Sketch& other) const
    {
        for (unsigned i = 0; i < SKETCH_SIZE; i++)
        {
            if (desc[i] < other.desc[i])
                return -1;
            else if (desc[i] > other.desc[i])
                return 1;
        }
        return 0;
    }

    bool operator<(const Sketch& other) const
    {
        return compare(other) == -1;
    }

    bool operator!=(const Sketch& other) const
    {
        return compare(other) != 0;
    }
};

struct Region
{
    float x, y, r, t;
};

}}

#endif
