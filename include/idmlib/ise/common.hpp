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
typedef Chunk SketchData[SKETCH_SIZE];

struct Sketch {
    SketchData desc;
};

struct Region
{
    float x, y, r, t;
};

}}

#endif
