/********************************************************************************
  Description:
  Comments   :
********************************************************************************/

#include <idmlib/duplicate-detection/FingerPrinter.h>

using namespace sf1v5;
#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

wiselib::MRandom FingerPrinter::rand(FingerPrinter::RAND_INIT);
/********************************************************************************
  Description: Randomly initialize seed value.
  Comments   :
********************************************************************************/
FingerPrinter::FingerPrinter()
{
  seed = rand.integer();
  seed <<= 32;
  seed += rand.integer();
}

void FingerPrinter::set_seed() {
  seed = rand.integer();
  seed <<= 32;
  seed += rand.integer();
}

/********************************************************************************
  Description: Returns 64 bit fingerprint of the data and length.
  Comments   :
********************************************************************************/
uint64_t FingerPrinter::fp(const char* data, int alen) const
{
  register uint8_t *k = (uint8_t *)data;
  register uint32_t length = alen;
  register uint64_t a,b,c;
  register uint32_t len;

  /* Set up the internal state */
  len = length;
  a = b = seed ;  /* the golden ratio; an arbitrary value */
  c = 0x7902cac39f392d18LL;

  /*---------------------------------------- handle most of the key */
  while (len >= 24)
    {
      a += (k[0] +((uint64_t)k[1]<<8) +((uint64_t)k[2]<<16) +((uint64_t)k[3]<<24)
	    +((uint64_t)k[4]<<32)+((uint64_t)k[5]<<40)+((uint64_t)k[6]<<48)+((uint64_t)k[7]<<56));
      b += (k[8] +((uint64_t)k[9]<<8) +((uint64_t)k[10]<<16) +((uint64_t)k[11]<<24)
	    +((uint64_t)k[12]<<32)+((uint64_t)k[13]<<40)+((uint64_t)k[14]<<48)+((uint64_t)k[15]<<56));
      c += (k[16] +((uint64_t)k[17]<<8) +((uint64_t)k[18]<<16)+((uint64_t)k[19]<<24)
	    +((uint64_t)k[20]<<32)+((uint64_t)k[21]<<40)+((uint64_t)k[22]<<48)+((uint64_t)k[23]<<56));
      mix(a,b,c);
      k += 24; len -= 24;
    }

  /*------------------------------------- handle the last 11 bytes */
  c += length;
  switch(len)              /* all the case statements fall through */
    {
    case 23 : c+=((uint64_t)k[22]<<56);
    case 22 : c+=((uint64_t)k[21]<<48);
    case 21 : c+=((uint64_t)k[20]<<40);
    case 20 : c+=((uint64_t)k[19]<<32);
    case 19 : c+=((uint64_t)k[18]<<24);
    case 18 : c+=((uint64_t)k[17]<<16);
    case 17 : c+=((uint64_t)k[16]<<8);
      /* the first byte of c is reserved for the length */
    case 16 : b+=((uint64_t)k[15]<<56);
    case 15 : b+=((uint64_t)k[14]<<48);
    case 14 : b+=((uint64_t)k[13]<<40);
    case 13 : b+=((uint64_t)k[12]<<32);
    case 12 : b+=((uint64_t)k[11]<<24);
    case 11 : b+=((uint64_t)k[10]<<16);
    case 10 : b+=((uint64_t)k[9]<<8);
    case 9 : b+=k[8];
    case 8 : a+=((uint64_t)k[7]<<56);
    case 7 : a+=((uint64_t)k[6]<<48);
    case 6 : a+=((uint64_t)k[5]<<40);
    case 5 : a+=((uint64_t)k[4]<<32);
    case 4 : a+=((uint64_t)k[3]<<24);
    case 3 : a+=((uint64_t)k[2]<<16);
    case 2 : a+=((uint64_t)k[1]<<8);
    case 1 : a+=k[0];
      /* case 0: nothing left to add */
    }
  mix(a,b,c);
  /*-------------------------------------------- report the result */
  return c;
}
