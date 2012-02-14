/**
   @file charikar_algo.cpp
   @author Kevin Hu
   @date 2009.11.24
 */
#include <idmlib/duplicate-detection/charikar_algo.h>
#include <idmlib/duplicate-detection/IntegerHashFunction.h>
#include <util/hashFunction.h>

using namespace idmlib::dd;

/********************************************************************************
Description: CharikarAlgorithm implements random projection based near-duplicate
               algorithm found in Charikar's paper, "Similarity Estimation Techniques
               from Rounding Algorithms". This algorithm implementation is based
               on Henzinger in "Finding Near-Duplicate Web Pages: A Large-Scale
               Evaluation of Algorithms"
             Given a document, represented as a sequence of token strings,
               a document signature is nBit vector where all of the sum
               of random projections of document tokens are later cast to either
               1 (when positive) or 0 (when negative).
             Given the two document signature objects, the number of
               agreed-on bits represents the cosine similarity between the two
               vectors and any number of bit matches higher than a threshold
               is construted as a near-duplicate.

Comments   :
History    : Yeogirl Yun                                      1/22/07
               Initial Revision
********************************************************************************/

void CharikarAlgo::generate_document_signature(
        const std::vector<std::string>& docTokens,
        std::vector<uint64_t>& signature) const
{
    std::vector<double> weights(docTokens.size(), 1.0);
    generate_document_signature(docTokens, weights, signature);
}

void CharikarAlgo::generate_document_signature(
        const std::vector<std::string>& docTokens,
        const std::vector<double>& weights,
        std::vector<uint64_t>& signature) const
{
    std::vector<double> vec(num_dimensions(), 0.0);
    static const double va[] = {-1.0, 1.0};
    for (uint32_t i = 0; i < docTokens.size(); i++)
    {
        uint64_t key = izenelib::util::HashFunction<std::string>::generateHash64(docTokens[i]);
        for (uint32_t n = 0, l = 0; n < num_dimensions(); n++, l++)
        {
            if (l == 64)
            {
                key = int_hash::hash64shift(key);
                l = 0;
            }
            double v = va[key & 0x01] * weights[i];
            vec[n] += v;
            key >>= 1;
        }
    }

    signature.resize((vec.size() + 63) / 64);
    for (uint32_t i = 0; i < vec.size(); i++)
    {
        uint32_t j = i >> 6;
        if (vec[i] >= 0) signature[j] |= uint64_t(1) << (i & 0x3f);
    }
}
