
#include <idmlib/duplicate-detection/CharikarAlgorithm.h>
#include <idmlib/duplicate-detection/NearDuplicateSignature.h>
#include <idmlib/duplicate-detection/GetToken.h>
/**
 * @file
 * @brief source file of CharikarAlgorithm.h
 */

namespace sf1v5{

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

/********************************************************************************
  Description: Generates a Charikar document signature from a document tokens.
  Comments   :
********************************************************************************/
void CharikarAlgorithm::
generate_document_signature(const wiselib::DynamicArray<uint64_t>& docTokens, wiselib::CBitArray& bitArray)
{
    bitArray.SetLength(num_dimensions()/8);

    RandomProjection rpSum(nDimensions);
    for (int j = 0; j < docTokens.size(); j++) {
        const RandomProjection& rp = rpEngine.get_random_projection(docTokens[j]);
        rpSum += rp;
    }

    for (int i = 0; i < rpSum.num_dimensions(); i++) {
        if (rpSum[i] >= 0) {
            bitArray.SetAt(i);
        }
    }
}

void CharikarAlgorithm::
generate_document_signature(const wiselib::YString& document, wiselib::CBitArray& bitArray) {
    bitArray.SetLength(num_dimensions()/8);

    RandomProjection rpSum(nDimensions);
    GetToken gt(document);
    uint64_t token_number = 0;
    while(gt.get_token(token_number)) {
        const RandomProjection& rp = rpEngine.get_random_projection(token_number);
        rpSum += rp;
    }

    for (int i = 0; i < rpSum.num_dimensions(); i++) {
        if (rpSum[i] >= 0) {
            bitArray.SetAt(i);
        }
    }
}

void CharikarAlgorithm::
generate_document_signature(const std::vector<std::string>& docTokens, wiselib::CBitArray& bitArray)
{
   bitArray.SetLength(num_dimensions()/8);

    RandomProjection rpSum(nDimensions);
    for (unsigned int j = 0; j < docTokens.size(); j++) {
        const RandomProjection& rp = rpEngine.get_random_projection(docTokens[j]);
        rpSum += rp;
    }

    for (int i = 0; i < rpSum.num_dimensions(); i++) {
        if (rpSum[i] >= 0) {
            bitArray.SetAt(i);
        }
    }
}

void CharikarAlgorithm::
generate_document_signature(const std::vector<unsigned int>& docTokens, wiselib::CBitArray& bitArray)
{
    bitArray.SetLength(num_dimensions()/8);

    RandomProjection rpSum(nDimensions);
    for (unsigned int j = 0; j < docTokens.size(); j++) {
        const RandomProjection& rp = rpEngine.get_random_projection(docTokens[j]);
        rpSum += rp;
    }

    for (int i = 0; i < rpSum.num_dimensions(); i++) {
        if (rpSum[i] >= 0) {
            bitArray.SetAt(i);
        }
    }
}

/********************************************************************************
  Description:
  Comments   :
********************************************************************************/
int CharikarAlgorithm::neardup_score(NearDuplicateSignature& sig1,
				     NearDuplicateSignature& sig2)
{
  return sig1.get_bitarray().NumAgreedBits(nDimensions, sig2.get_bitarray());
}

int CharikarAlgorithm::neardup_score(const wiselib::CBitArray& sig1, const wiselib::CBitArray& sig2) {
    return sig1.NumAgreedBits(nDimensions, sig2);
}

}