/*
 * ContentChunker.cc
 *
 *  Created on: 2008-12-22
 *      Author: jinglei
 */

#include <idmlib/duplicate-detection/ContentChunker.h>
#include <idmlib/duplicate-detection/RabinPoly.h>
#include <stdio.h>
#include <iostream>
using namespace std;
namespace sf1v5{


void ContentChunker::getChunks(wiselib::YString strFile,vector<wiselib::YString>& vecChunks)
{
	// INITIALIZE RABINPOLY CLASS

	window myRabin(FINGERPRINT_PT, DEFAULT_WINDOW_SIZE);
	myRabin.reset();
   int nChunkBegin=0; //begin of a new chunk
   uint64_t rabinf;
   int fileSize=strFile.size();
   char* fileBytes=(char*)strFile.c_str();
	for(int i=0;i<fileSize;i++)
	{
		rabinf = myRabin.slide8(fileBytes[i]);
		//printf("==>\t%LX\n", rabinf);
		uint64_t tempfp=rabinf&077;
		if(tempfp==MARKER)
		{
			//printf("temp==>\t%LX\n", tempfp);
			int nChunkSize=i-nChunkBegin+1;
			wiselib::YString strNewChunk=strFile.substr(nChunkBegin,nChunkSize);
			//cout<<strNewChunk<<endl;
			vecChunks.push_back(strNewChunk);
			nChunkBegin=i+1;
		}
	}
}

}
