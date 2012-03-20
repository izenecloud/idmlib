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

static const unsigned CHUNK_BIT = sizeof(Chunk) * 8;
static const unsigned DATA_CHUNK = 128 / CHUNK_BIT;

struct Sketch {
    SketchData desc;
};

struct Region
{
    float x, y, r, t;
};

class Hamming
{
    static unsigned char_bit_cnt[];
    template <typename B>
    unsigned __hamming (B a, B b)
    {
        B c = a ^ b;
        unsigned char *p = reinterpret_cast<unsigned char *>(&c);
        unsigned r = 0;
        for (unsigned i = 0; i < sizeof(B); i++)
        {
            r += char_bit_cnt[*p++];
        }
        return r;
    }
public:
    float operator () (const Chunk *first1, const Chunk *first2) 
    {
        unsigned r = 0;
        for (unsigned i = 0; i < DATA_CHUNK; ++i)
        {
            r += __hamming(first1[i], first2[i]);
        }
        return float(r);
    }
};


}}

#endif
