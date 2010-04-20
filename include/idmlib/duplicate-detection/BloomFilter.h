/*
 * BloomFilter.h
 *
 *  Created on: 2008-12-23
 *      Author: jinglei
 */

#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_

#include "FingerPrinter.h"
#include "NearDuplicateSignature.h"
#include "NearDuplicateAlgorithm.h"

#include <types.h>
#include <wiselib/CBitArray.h>
#include <wiselib/YString.h>

#include <vector>

namespace sf1v5{
/**
 * @brief This class implements the Bloom filter algorithm for dup-detection.
 */
class BloomFilter: public NearDuplicateAlgorithm  {
private:
	/**
	 * number of different hash functions.
	 */
	int k;
	/**
	 * number of bits of bloom filter
	 */
	int m;
	vector<FingerPrinter> fpFuncs;
public:
	  /**
	   * default k value
	   */
	  static const int DEFAULT_K_VALUE = 3;
	  /**
	   * default m value
	   * The value should be 8x of or larger than the chunk num,
	   * in order to let the false positives acceptable.
	   */
	  static const int DEFAULT_M_VALUE = 384;
	  /**
	   * the threshold
	   */
	  static const double THRESHOLD_VALUE = 0.75;
	  /**
	   * The max number of ones used to determine near-dup pairs.
	   */
	  static int MAX_ONES;
private:
	//set the seed for fingerprinter.
	void set_fingerprinters();
public:
	inline BloomFilter(int ak=DEFAULT_K_VALUE,int am=DEFAULT_M_VALUE)
	:k(ak),m(am),fpFuncs(k)
	{

	}
	inline ~BloomFilter(){}
	 /**
	  * generate document's signature
	  * @param[in] document document's string value
	  * @param[out] bitArray document's signature
	  */
	void generate_document_signature(const wiselib::YString& document, wiselib::CBitArray& bitArray);
	int neardup_score(NearDuplicateSignature& sig1, NearDuplicateSignature& sig2);
	int neardup_score(const wiselib::CBitArray& sig1, const wiselib::CBitArray& sig2);
	bool is_neardup(float neardupScore);
	inline int num_dimensions()  {return m;}
	inline void generate_document_signature(const wiselib::DynamicArray<uint64_t>& docTokens, wiselib::CBitArray& bitArray) {}
	inline void generate_document_signature(const std::vector<unsigned int>& docTokens, wiselib::CBitArray& bitArray){}

};
}
#endif /* BLOOMFILTER_H_ */