//
// zlib_turbo
// An optimized version of zlib inflate
// Created by Larry Bank on 4/19/24.
// Copyright (C) 2024 BitBank Software, Inc.
//
// Original code written by Mark Adler and Jean-loup Gailly
/*   Copyright (C) 1995-2024 Jean-loup Gailly and Mark Adler

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  Jean-loup Gailly        Mark Adler
  jloup@gzip.org          madler@alumni.caltech.edu


  The data format used by the zlib library is described by RFCs (Request for
  Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
  (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
*/

#ifndef zlib_turbo_h
#define zlib_turbo_h

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAXBITS 15
#define ENOUGH_LENS 852
#define ENOUGH_DISTS 592
#define ENOUGH (ENOUGH_LENS+ENOUGH_DISTS)

// Error / success codes
enum {
    ZT_SUCCESS,
    ZT_HEADER_ERROR,
    ZT_DECODE_ERROR,
    ZT_OUTPUT_INSUFFICIENT,
    ZT_INPUT_INSUFFICIENT,
    ZT_INVALID_PARAMETER
};

/* Type of code to build for inflate_table() */
typedef enum {
    CODES,
    LENS,
    DISTS
} codetype;

typedef struct code_tag {
    uint8_t bits;         /* bits in this part of the code */
    uint8_t op;           /* operation, extra bits, table bits */
    uint16_t val;         /* offset in table or code value */
} code;
/* op values as set by inflate_table():
    00000000 - literal
    0000tttt - table link, tttt != 0 is the number of table index bits
    0001eeee - length or distance, eeee is the number of extra bits
    01100000 - end of block
    01000000 - invalid code
 */

typedef struct zt_buffer_tag {
    uint8_t     *next_in;  /* next input byte */
    uint32_t    avail_in;  /* number of bytes available at next_in */
    uint32_t    total_in;  /* total number of input bytes read so far */

    uint8_t     *next_out; /* next output byte will go here */
    uint32_t    avail_out; /* remaining free space at next_out */
    uint32_t    total_out; /* total number of bytes output so far */
//    uLong   adler;      /* Adler-32 or CRC-32 value of the uncompressed data */
} zt_buffer;

//#define BITS(len) (ulBits & ((1U << len)-1))
#define DROPBITS(len) {ulBitCount -= len; ulBits >>= len;}
#if (INTPTR_MAX == INT64_MAX)
// This generates the fewest masking instructions (at least on arm)
#define BITS(len) (ulBits & ~(~((uint64_t)0) << len))
#define ALLOWS_UNALIGNED
#define REGISTER_WIDTH 64
#define BIGINT int64_t
#define BIGUINT uint64_t
#define GETMOREBITS {uint64_t u64; u64 = *(uint32_t *)pBuf; pBuf += 4; ulBits |= (u64 << ulBitCount); ulBitCount += 32;}
#else
#define REGISTER_WIDTH 32
// This generates the fewest masking instructions (at least on arm)
#define BITS(len) (ulBits & ~(~((uint32_t)0) << len))
#define BIGINT int32_t
#define BIGUINT uint32_t
#ifdef ALLOWS_UNALIGNED
#define GETMOREBITS {uint32_t u32; u32 = *(uint16_t *)pBuf; pBuf += 2; ulBits |= (u32 << ulBitCount); ulBitCount += 16;}
#else
#define GETMOREBITS {uint32_t u32; u32 = (pBuf[0] | (pBuf[1] << 8)); pBuf += 2; ulBits |= (u32 << ulBitCount); ulBitCount += 16;}
#endif // ALLOWS_UNALIGNED
#endif // 64-bit

#define NEXTBYTE(u) {BIGUINT c = u & 7; if (c) u += (8-c);}

/* State maintained between inflate() calls -- approximately 7K bytes, not
   including the allocated sliding window, which is up to 32K bytes. */
typedef struct zt_state_tag {
    uint8_t iLastError;             /* last error */
    uint8_t bLastBlock;                   /* true if processing last block */
    uint16_t wbits;             /* log base 2 of requested window size */
    BIGUINT ulBits;         /* input bit accumulator */
    BIGUINT ulBitCount;     /* number of bits in "ulBits" */
    code const *lencode;    /* starting table for length/literal codes */
    code const *distcode;   /* starting table for distance codes */
    uint8_t lenbits;           /* index bits for lencode */
    uint8_t distbits;          /* index bits for distcode */
        /* dynamic table building */
    uint16_t ncode;             /* number of code length code lengths */
    uint16_t nlen;              /* number of length code lengths */
    uint16_t ndist;             /* number of distance code lengths */
    uint16_t have;              /* number of code lengths in lens[] */
    code *next;             /* next available space in codes[] */
    uint8_t lens[320];   /* temporary storage for code lengths */
    uint16_t work[288];   /* work area for code table building */
    code codes[ENOUGH];         /* space for code tables */
} zt_state;

#ifdef __cplusplus
//
// The UNZIP class wraps portable C code which does the actual work
//
class zlib_turbo
{
  public:
    void inflate_init(uint8_t *pOut, int iOutSize);
    int inflate(uint8_t *pIn, int iInSize);
    int outSize(void);
    uint32_t gzip_info(uint8_t *pCompressed, int iSize, char *szName = NULL, uint32_t *pu32Time = NULL);
    int gunzip(uint8_t *pCompressed, int iSize, uint8_t *pUncompressed);
    
  private:
    zt_state _state;
    zt_buffer _buffer;
}; // zlib_turbo class
// C interface here
#endif

#ifdef __cplusplus
extern "C" {
#endif
int zt_inflate(zt_state *state, zt_buffer *buffer, int bEnd);
int zt_init(zt_state *state);
uint32_t zt_gzip_info(uint8_t *pCompressed, int iSize, char *szName, uint32_t *pu32Time);
int zt_gunzip(uint8_t *pCompressed, int iInSize, uint8_t *pUncompressed);
#ifdef __cplusplus
}
#endif

#endif /* zlib_turbo_h */
