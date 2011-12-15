/**
   @file charikar_algo.cpp
   @author Kevin Hu
   @date 2009.11.24
 */
#include <idmlib/duplicate-detection/charikar_algo.h>
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
    std::vector<double> weights(docTokens.size(), 1.0);
    generate_document_signature(docTokens, weights, bitArray);
}

void CharikarAlgo::generate_document_signature(const std::vector<std::string>& docTokens, const std::vector<double>& weights, izenelib::util::CBitArray& bitArray)
{
    

//     RandProj rpSum(nDimensions);
//     for (unsigned int j = 0; j < docTokens.size(); j++)
//     {
//         const RandProj& rp = rpEngine.get_random_projection(docTokens[j]);
//         rpSum += rp;
//     }
// 
//     rpSum.generate_bitarray(bitArray);
    
    std::vector<double> vec(num_dimensions(), 0.0);
    double va[] = {-1.0, 1.0};
    for(uint32_t i=0;i<docTokens.size();i++)
    {
        uint64_t key= izenelib::util::HashFunction<std::string>::generateHash64(docTokens[i]);
        for(uint32_t n=0;n<num_dimensions();n++)
        {
            double v = va[key & 0x01] * weights[i];
            vec[n] += v;
            key >>= 1;
        }
    }
    bitArray.SetLength(num_dimensions()/8);
    bitArray.ResetAll();
    for (uint32_t i = 0; i<vec.size(); i++)
    {
        if (vec[i] >= 0)
        {
            bitArray.SetAt(i);
        }
    }
}

