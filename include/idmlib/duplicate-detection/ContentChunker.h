/*
 * ContentChunker.h
 *
 *  Created on: 2008-12-22
 *      Author: jinglei
 */

#ifndef CONTENTCHUNKER_H_
#define CONTENTCHUNKER_H_

#include <types.h>
//#include <wiselib/IntTypes.h>
#include <wiselib/YString.h>
#include <vector>
#include <utility>

namespace sf1v5{

/**
 * @brief This class computes the content addressable chunks for a given document
 *  according to
 * CDC algorithm used in the paper "Using Bloom Filters to Refine Web Search Results".
 */
class ContentChunker {
public:
	ContentChunker(unsigned int maxChunkSize=DEFAULT_MAX_CHUNK_SIZE,unsigned int makerLength=DEFAULT_MARKER_LENGTH):
		_maxChunkSize(maxChunkSize),_markerLength(makerLength){}
	virtual ~ContentChunker(){}
private:
	unsigned int _maxChunkSize;
	unsigned int _markerLength;
	static const uint64_t FINGERPRINT_PT=0xbfe6b8a5bf378d83LL;
public:
	static const int DEFAULT_MAX_CHUNK_SIZE=2048;
	static const int DEFAULT_MARKER_LENGTH=6;
	static const unsigned int MARKER=032;
	/**
	 * The widow size for overlapping rabin fingerprinting.
	 * Roughly about 8 string tokens...
	 */
	static const unsigned int WINDOW_SIZE=48;

public:
	void getChunks(wiselib::YString strFile,std::vector<wiselib::YString>& vecChunks);

};
}
#endif /* CONTENTCHUNKER_H_ */
