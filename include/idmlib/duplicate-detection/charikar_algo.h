#ifndef CHARIKAR_ALGO_H
#define CHARIKAR_ALGO_H
/**
 * @file charikar_algo.h
 * @brief impliments Charikar's algorith
 *
 * CharikarAlgorithm implements random projection based near-duplicate
 * algorithm found in Charikar's paper, "Similarity Estimation Techniques
 * from Rounding Algorithms". This algorithm implementation is based
 * on Henzinger in "Finding Near-Duplicate Web Pages: A Large-Scale
 * Evaluation of Algorithms"
 * Given a document, represented as a sequence of token strings,
 * a document signature is nBit vector where all of the sum
 * of random projections of document tokens are later cast to etiher
 * 1 (when positive) and 0 (when negative).
 * Given the two document signature objects, the number of
 * agreed-on bits represents the cosine similarity between the two
 * vectors and any number of bit matches higher than a threshold
 * is construted as a near-duplicate.
 *
 * @author Kevin Hu
 * @date 11/09/09
 */

#include <idmlib/idm_types.h>
#include "rand_proj.h"
#include "rand_proj_gen.h"

#include <util/CBitArray.h>
#include <vector>


NS_IDMLIB_DD_BEGIN
/**
   @class CharikarAlgo
 * @brief Charikar alogrithm
 */
class CharikarAlgo
{

private:

    RandProjGen rpEngine;//!< random projection engine     */
    int nDimensions; //!< dimensions number
public:
    static const int DEFAULT_NUM_DIMENSIONS = 64;//!< default threshold value
public:
    /**
     * @brief constructor of CharikarAlgo, initial members
     *
     * @param nDim dimensions number
     * @param tvalue threshold value
     */
    CharikarAlgo(int nDim=CharikarAlgo::DEFAULT_NUM_DIMENSIONS)
            :rpEngine("", nDim), nDimensions(nDim){ }

    /**
     * @brief disconstructor
     */
    ~CharikarAlgo() {}
public:
    /**
     * @brief get dimensions number
     *
     * @return dimensions number
     */
    inline int num_dimensions()
    {
        return nDimensions;
    }

    /**
        * @brief generate signature from a vector
        *
        * @param[in] docTokens input source, a term id array
        * @param[out] bitArray signature
        */
    void generate_document_signature(const std::vector<std::string>& docTokens, izenelib::util::CBitArray& bitArray);

};
NS_IDMLIB_DD_END

#endif
