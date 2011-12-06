/**
   @file charikar_algo.cpp
   @author Kevin Hu
   @date 2009.11.24
 */
#include <idmlib/duplicate-detection/charikar_algo.h>

using namespace idmlib::dd;

/********************************************************************************
Description: CharikarAlgorithm implements random projection based near-duplicate
               algorithm found in Charikar's paper, "Similarity Estimation Techniques
               from Rounding Algorithms". This algorithm implementation is based
               on Henzinger in "Finding Near-Duplicate Web Pages: A Large-Scale
               Evaluation of Algorithms"
             Given a document, represented as a sequence of token strings,
               a document signature is nBit vector where all of the sum
               of random projections of document tokens are later cast to etiher
               1 (when positive) and 0 (when negative).
             Given the two document signature objects, the number of
               agreed-on bits represents the cosine similarity between the two
               vectors and any number of bit matches higher than a threshold
               is construted as a near-duplicate.

Comments   :
History    : Yeogirl Yun                                      1/22/07
               Initial Revision
********************************************************************************/

void CharikarAlgo::
generate_document_signature(const std::vector<std::string>& docTokens, izenelib::util::CBitArray& bitArray)
{
    bitArray.SetLength(num_dimensions()/8);

    RandProj rpSum(nDimensions);
    for (unsigned int j = 0; j < docTokens.size(); j++)
    {
        const RandProj& rp = rpEngine.get_random_projection(docTokens[j]);
        rpSum += rp;
    }

    rpSum.generate_bitarray(bitArray);

}

