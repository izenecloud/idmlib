/*
 * BloomFilter.cpp
 *
 *  Created on: 2008-12-23
 *      Author: jinglei
 */

#include <idmlib/duplicate-detection/BloomFilter.h>
#include <idmlib/duplicate-detection/ContentChunker.h>

namespace sf1v5
{

int BloomFilter::MAX_ONES=DEFAULT_M_VALUE;
void BloomFilter::set_fingerprinters() {
    uint64_t id = 0;
    FingerPrinter::reset_rand();
    for(int i = 0; i < k; ++i,++id) {
        fpFuncs[i].set_seed();
    }
}

void BloomFilter::generate_document_signature(const wiselib::YString& document, wiselib::CBitArray& bitArray)
{
    bitArray.SetLength(m);
    ContentChunker cdc;
    vector<wiselib::YString> chunks;
    cdc.getChunks(document,chunks);
    for(size_t i=0;i<chunks.size();i++)
    {
    	for(int j=0;j<k;j++)
    	{
    		int index=fpFuncs[j].fp(chunks[i].c_str(),chunks[i].size())%m;
    		assert(index>=0&&index<m);
    		bitArray.SetAt(index);
    	}
    }
}

int BloomFilter::neardup_score(NearDuplicateSignature& sig1,
				     NearDuplicateSignature& sig2)
{
  return neardup_score(sig1.get_bitarray(),sig2.get_bitarray());
}

int BloomFilter::neardup_score(const wiselib::CBitArray& sig1, const wiselib::CBitArray& sig2) {
	MAX_ONES=max(const_cast<wiselib::CBitArray&>(sig1).GetCount(),const_cast<wiselib::CBitArray&>(sig2).GetCount());
	return (const_cast<wiselib::CBitArray&>(sig1)&const_cast<wiselib::CBitArray&>(sig2)).GetCount();
}

bool BloomFilter::is_neardup(float neardupScore)
{
	if(MAX_ONES==0) return false;
	else
		return (neardupScore/MAX_ONES)>THRESHOLD_VALUE;
}

}
