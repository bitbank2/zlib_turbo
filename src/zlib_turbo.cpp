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

#include "zlib_turbo.h"

static const uint8_t len_order[19] = /* permutation of code lengths */
    {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
//
// Fixed (static) Huffman tables for block type 01
//
static const code lenfix[512] = {
    {96,7,0},{0,8,80},{0,8,16},{20,8,115},{18,7,31},{0,8,112},{0,8,48},
    {0,9,192},{16,7,10},{0,8,96},{0,8,32},{0,9,160},{0,8,0},{0,8,128},
    {0,8,64},{0,9,224},{16,7,6},{0,8,88},{0,8,24},{0,9,144},{19,7,59},
    {0,8,120},{0,8,56},{0,9,208},{17,7,17},{0,8,104},{0,8,40},{0,9,176},
    {0,8,8},{0,8,136},{0,8,72},{0,9,240},{16,7,4},{0,8,84},{0,8,20},
    {21,8,227},{19,7,43},{0,8,116},{0,8,52},{0,9,200},{17,7,13},{0,8,100},
    {0,8,36},{0,9,168},{0,8,4},{0,8,132},{0,8,68},{0,9,232},{16,7,8},
    {0,8,92},{0,8,28},{0,9,152},{20,7,83},{0,8,124},{0,8,60},{0,9,216},
    {18,7,23},{0,8,108},{0,8,44},{0,9,184},{0,8,12},{0,8,140},{0,8,76},
    {0,9,248},{16,7,3},{0,8,82},{0,8,18},{21,8,163},{19,7,35},{0,8,114},
    {0,8,50},{0,9,196},{17,7,11},{0,8,98},{0,8,34},{0,9,164},{0,8,2},
    {0,8,130},{0,8,66},{0,9,228},{16,7,7},{0,8,90},{0,8,26},{0,9,148},
    {20,7,67},{0,8,122},{0,8,58},{0,9,212},{18,7,19},{0,8,106},{0,8,42},
    {0,9,180},{0,8,10},{0,8,138},{0,8,74},{0,9,244},{16,7,5},{0,8,86},
    {0,8,22},{64,8,0},{19,7,51},{0,8,118},{0,8,54},{0,9,204},{17,7,15},
    {0,8,102},{0,8,38},{0,9,172},{0,8,6},{0,8,134},{0,8,70},{0,9,236},
    {16,7,9},{0,8,94},{0,8,30},{0,9,156},{20,7,99},{0,8,126},{0,8,62},
    {0,9,220},{18,7,27},{0,8,110},{0,8,46},{0,9,188},{0,8,14},{0,8,142},
    {0,8,78},{0,9,252},{96,7,0},{0,8,81},{0,8,17},{21,8,131},{18,7,31},
    {0,8,113},{0,8,49},{0,9,194},{16,7,10},{0,8,97},{0,8,33},{0,9,162},
    {0,8,1},{0,8,129},{0,8,65},{0,9,226},{16,7,6},{0,8,89},{0,8,25},
    {0,9,146},{19,7,59},{0,8,121},{0,8,57},{0,9,210},{17,7,17},{0,8,105},
    {0,8,41},{0,9,178},{0,8,9},{0,8,137},{0,8,73},{0,9,242},{16,7,4},
    {0,8,85},{0,8,21},{16,8,258},{19,7,43},{0,8,117},{0,8,53},{0,9,202},
    {17,7,13},{0,8,101},{0,8,37},{0,9,170},{0,8,5},{0,8,133},{0,8,69},
    {0,9,234},{16,7,8},{0,8,93},{0,8,29},{0,9,154},{20,7,83},{0,8,125},
    {0,8,61},{0,9,218},{18,7,23},{0,8,109},{0,8,45},{0,9,186},{0,8,13},
    {0,8,141},{0,8,77},{0,9,250},{16,7,3},{0,8,83},{0,8,19},{21,8,195},
    {19,7,35},{0,8,115},{0,8,51},{0,9,198},{17,7,11},{0,8,99},{0,8,35},
    {0,9,166},{0,8,3},{0,8,131},{0,8,67},{0,9,230},{16,7,7},{0,8,91},
    {0,8,27},{0,9,150},{20,7,67},{0,8,123},{0,8,59},{0,9,214},{18,7,19},
    {0,8,107},{0,8,43},{0,9,182},{0,8,11},{0,8,139},{0,8,75},{0,9,246},
    {16,7,5},{0,8,87},{0,8,23},{64,8,0},{19,7,51},{0,8,119},{0,8,55},
    {0,9,206},{17,7,15},{0,8,103},{0,8,39},{0,9,174},{0,8,7},{0,8,135},
    {0,8,71},{0,9,238},{16,7,9},{0,8,95},{0,8,31},{0,9,158},{20,7,99},
    {0,8,127},{0,8,63},{0,9,222},{18,7,27},{0,8,111},{0,8,47},{0,9,190},
    {0,8,15},{0,8,143},{0,8,79},{0,9,254},{96,7,0},{0,8,80},{0,8,16},
    {20,8,115},{18,7,31},{0,8,112},{0,8,48},{0,9,193},{16,7,10},{0,8,96},
    {0,8,32},{0,9,161},{0,8,0},{0,8,128},{0,8,64},{0,9,225},{16,7,6},
    {0,8,88},{0,8,24},{0,9,145},{19,7,59},{0,8,120},{0,8,56},{0,9,209},
    {17,7,17},{0,8,104},{0,8,40},{0,9,177},{0,8,8},{0,8,136},{0,8,72},
    {0,9,241},{16,7,4},{0,8,84},{0,8,20},{21,8,227},{19,7,43},{0,8,116},
    {0,8,52},{0,9,201},{17,7,13},{0,8,100},{0,8,36},{0,9,169},{0,8,4},
    {0,8,132},{0,8,68},{0,9,233},{16,7,8},{0,8,92},{0,8,28},{0,9,153},
    {20,7,83},{0,8,124},{0,8,60},{0,9,217},{18,7,23},{0,8,108},{0,8,44},
    {0,9,185},{0,8,12},{0,8,140},{0,8,76},{0,9,249},{16,7,3},{0,8,82},
    {0,8,18},{21,8,163},{19,7,35},{0,8,114},{0,8,50},{0,9,197},{17,7,11},
    {0,8,98},{0,8,34},{0,9,165},{0,8,2},{0,8,130},{0,8,66},{0,9,229},
    {16,7,7},{0,8,90},{0,8,26},{0,9,149},{20,7,67},{0,8,122},{0,8,58},
    {0,9,213},{18,7,19},{0,8,106},{0,8,42},{0,9,181},{0,8,10},{0,8,138},
    {0,8,74},{0,9,245},{16,7,5},{0,8,86},{0,8,22},{64,8,0},{19,7,51},
    {0,8,118},{0,8,54},{0,9,205},{17,7,15},{0,8,102},{0,8,38},{0,9,173},
    {0,8,6},{0,8,134},{0,8,70},{0,9,237},{16,7,9},{0,8,94},{0,8,30},
    {0,9,157},{20,7,99},{0,8,126},{0,8,62},{0,9,221},{18,7,27},{0,8,110},
    {0,8,46},{0,9,189},{0,8,14},{0,8,142},{0,8,78},{0,9,253},{96,7,0},
    {0,8,81},{0,8,17},{21,8,131},{18,7,31},{0,8,113},{0,8,49},{0,9,195},
    {16,7,10},{0,8,97},{0,8,33},{0,9,163},{0,8,1},{0,8,129},{0,8,65},
    {0,9,227},{16,7,6},{0,8,89},{0,8,25},{0,9,147},{19,7,59},{0,8,121},
    {0,8,57},{0,9,211},{17,7,17},{0,8,105},{0,8,41},{0,9,179},{0,8,9},
    {0,8,137},{0,8,73},{0,9,243},{16,7,4},{0,8,85},{0,8,21},{16,8,258},
    {19,7,43},{0,8,117},{0,8,53},{0,9,203},{17,7,13},{0,8,101},{0,8,37},
    {0,9,171},{0,8,5},{0,8,133},{0,8,69},{0,9,235},{16,7,8},{0,8,93},
    {0,8,29},{0,9,155},{20,7,83},{0,8,125},{0,8,61},{0,9,219},{18,7,23},
    {0,8,109},{0,8,45},{0,9,187},{0,8,13},{0,8,141},{0,8,77},{0,9,251},
    {16,7,3},{0,8,83},{0,8,19},{21,8,195},{19,7,35},{0,8,115},{0,8,51},
    {0,9,199},{17,7,11},{0,8,99},{0,8,35},{0,9,167},{0,8,3},{0,8,131},
    {0,8,67},{0,9,231},{16,7,7},{0,8,91},{0,8,27},{0,9,151},{20,7,67},
    {0,8,123},{0,8,59},{0,9,215},{18,7,19},{0,8,107},{0,8,43},{0,9,183},
    {0,8,11},{0,8,139},{0,8,75},{0,9,247},{16,7,5},{0,8,87},{0,8,23},
    {64,8,0},{19,7,51},{0,8,119},{0,8,55},{0,9,207},{17,7,15},{0,8,103},
    {0,8,39},{0,9,175},{0,8,7},{0,8,135},{0,8,71},{0,9,239},{16,7,9},
    {0,8,95},{0,8,31},{0,9,159},{20,7,99},{0,8,127},{0,8,63},{0,9,223},
    {18,7,27},{0,8,111},{0,8,47},{0,9,191},{0,8,15},{0,8,143},{0,8,79},
    {0,9,255}
};
static const code distfix[32] = {
    {16,5,1},{23,5,257},{19,5,17},{27,5,4097},{17,5,5},{25,5,1025},
    {21,5,65},{29,5,16385},{16,5,3},{24,5,513},{20,5,33},{28,5,8193},
    {18,5,9},{26,5,2049},{22,5,129},{64,5,0},{16,5,2},{23,5,385},
    {19,5,25},{27,5,6145},{17,5,7},{25,5,1537},{21,5,97},{29,5,24577},
    {16,5,4},{24,5,769},{20,5,49},{28,5,12289},{18,5,13},{26,5,3073},
    {22,5,193},{64,5,0}
};

/*
   Build a set of tables to decode the provided canonical Huffman code.
   The code lengths are lens[0..codes-1].  The result starts at *table,
   whose indices are 0..2^bits-1.  work is a writable array of at least
   lens shorts, which is used as a work area.  type is the type of code
   to be generated, CODES, LENS, or DISTS.  On return, zero is success,
   -1 is an invalid code, and +1 means that ENOUGH isn't enough.  table
   on return points to the next available entry's address.  bits is the
   requested root table index bits, and on return it is the actual root
   table index bits.  It will differ if the request is greater than the
   longest code or if it is less than the shortest code.
 */
#ifdef __cplusplus
extern "C" {
#endif

static int zt_table(codetype type, uint8_t *lens, int codes, code **table, uint8_t *bits, uint16_t *work)
{
    unsigned len;               /* a code's length in bits */
    unsigned sym;               /* index of code symbols */
    unsigned min, max;          /* minimum and maximum code lengths */
    unsigned root;              /* number of index bits for root table */
    unsigned curr;              /* number of index bits for current table */
    unsigned drop;              /* code bits to drop for sub-table */
    int left;                   /* number of prefix codes available */
    unsigned used;              /* code entries in table used */
    unsigned huff;              /* Huffman code */
    unsigned incr;              /* for incrementing code, index */
    unsigned fill;              /* index for replicating entries */
    unsigned low;               /* low bits for current root entry */
    unsigned mask;              /* mask for low root bits */
    code here;                  /* table entry for duplication */
    code *next;             /* next available space in table */
    const uint16_t *base;     /* base value table to use */
    const uint16_t *extra;    /* extra bits table to use */
    unsigned match;             /* use base and extra for symbol >= match */
    uint8_t count[MAXBITS+1];    /* number of codes of each length */
    uint16_t offs[MAXBITS+1];     /* offsets in table for each length */
    uint32_t *pU32, U32Here;
    static const uint16_t lbase[31] = { /* Length codes 257..285 base */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0};
    static const uint16_t lext[31] = { /* Length codes 257..285 extra */
        16, 16, 16, 16, 16, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 18,
        19, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21, 16, 77, 202};
    static const uint16_t dbase[32] = { /* Distance codes 0..29 base */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577, 0, 0};
    static const uint16_t dext[32] = { /* Distance codes 0..29 extra */
        16, 16, 16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22,
        23, 23, 24, 24, 25, 25, 26, 26, 27, 27,
        28, 28, 29, 29, 64, 64};

    /*
       Process a set of code lengths to create a canonical Huffman code.  The
       code lengths are lens[0..codes-1].  Each length corresponds to the
       symbols 0..codes-1.  The Huffman code is generated by first sorting the
       symbols by length from short to long, and retaining the symbol order
       for codes with equal lengths.  Then the code starts with all zero bits
       for the first code of the shortest length, and the codes are integer
       increments for the same length, and zeros are appended as the length
       increases.  For the deflate format, these bits are stored backwards
       from their more natural integer increment ordering, and so when the
       decoding tables are built in the large loop below, the integer codes
       are incremented backwards.

       This routine assumes, but does not check, that all of the entries in
       lens[] are in the range 0..MAXBITS.  The caller must assure this.
       1..MAXBITS is interpreted as that code length.  zero means that that
       symbol does not occur in this code.

       The codes are sorted by computing a count of codes for each length,
       creating from that a table of starting indices for each length in the
       sorted table, and then entering the symbols in order in the sorted
       table.  The sorted table is work[], with that space being provided by
       the caller.

       The length counts are used for other purposes as well, i.e. finding
       the minimum and maximum length codes, determining if there are any
       codes at all, checking for a valid set of lengths, and looking ahead
       at length counts to determine sub-table sizes when building the
       decoding tables.
     */

    /* accumulate lengths for codes (assumes lens[] all in 0..MAXBITS) */
    for (len = 0; len <= MAXBITS; len++)
        count[len] = 0;
    for (sym = 0; sym < codes; sym++)
        count[lens[sym]]++;

    /* bound code lengths, force root to be within code lengths */
    root = *bits;
    for (max = MAXBITS; max >= 1; max--)
        if (count[max] != 0) break;
    if (root > max) root = max;
    if (max == 0) {                     /* no symbols to code at all */
        here.op = (unsigned char)64;    /* invalid code marker */
        here.bits = (unsigned char)1;
        here.val = (unsigned short)0;
        *(*table)++ = here;             /* make a table to force an error */
        *(*table)++ = here;
        *bits = 1;
        return 0;     /* no symbols, but wait for decoding to report error */
    }
    for (min = 1; min < max; min++)
        if (count[min] != 0) break;
    if (root < min) root = min;

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;
        left -= count[len];
        if (left < 0) return -1;        /* over-subscribed */
    }
    if (left > 0 && (type == CODES || max != 1))
        return -1;                      /* incomplete set */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + count[len];

    /* sort symbols by length, by symbol order within each length */
    for (sym = 0; sym < codes; sym++)
        if (lens[sym] != 0) work[offs[lens[sym]]++] = (unsigned short)sym;

    /*
       Create and fill in decoding tables.  In this loop, the table being
       filled is at next and has curr index bits.  The code being used is huff
       with length len.  That code is converted to an index by dropping drop
       bits off of the bottom.  For codes where len is less than drop + curr,
       those top drop + curr - len bits are incremented through all values to
       fill the table with replicated entries.

       root is the number of index bits for the root table.  When len exceeds
       root, sub-tables are created pointed to by the root entry with an index
       of the low root bits of huff.  This is saved in low to check for when a
       new sub-table should be started.  drop is zero when the root table is
       being filled, and drop is root when sub-tables are being filled.

       When a new sub-table is needed, it is necessary to look ahead in the
       code lengths to determine what size sub-table is needed.  The length
       counts are used for this, and so count[] is decremented as codes are
       entered in the tables.

       used keeps track of how many table entries have been allocated from the
       provided *table space.  It is checked for LENS and DIST tables against
       the constants ENOUGH_LENS and ENOUGH_DISTS to guard against changes in
       the initial root table size constants.  See the comments in inftrees.h
       for more information.

       sym increments through all symbols, and the loop terminates when
       all codes of length max, i.e. all codes, have been processed.  This
       routine permits incomplete codes, so another loop after this one fills
       in the rest of the decoding tables with invalid code markers.
     */

    /* set up for code type */
    switch (type) {
    case CODES:
        base = extra = work;    /* dummy value--not used */
        match = 20;
        break;
    case LENS:
        base = lbase;
        extra = lext;
        match = 257;
        break;
    default:    /* DISTS */
        base = dbase;
        extra = dext;
        match = 0;
    }

    /* initialize state for loop */
    huff = 0;                   /* starting code */
    sym = 0;                    /* starting code symbol */
    len = min;                  /* starting code length */
    next = *table;              /* current table to fill in */
    curr = root;                /* current table index bits */
    drop = 0;                   /* current bits to drop from code for index */
    low = (unsigned)(-1);       /* trigger new sub-table when len > root */
    used = 1U << root;          /* use root table entries */
    mask = used - 1;            /* mask for comparing low */

    /* check available table space */
    if ((type == LENS && used > ENOUGH_LENS) ||
        (type == DISTS && used > ENOUGH_DISTS))
        return 1;

    /* process all codes and make table entries */
    for (;;) {
        /* create table entry */
        here.bits = (unsigned char)(len - drop);
        if (work[sym] + 1U < match) {
            here.op = (unsigned char)0;
            here.val = work[sym];
        }
        else if (work[sym] >= match) {
            here.op = (unsigned char)(extra[work[sym] - match]);
            here.val = base[work[sym] - match];
        }
        else {
            here.op = (unsigned char)(32 + 64);         /* end of block */
            here.val = 0;
        }

        /* replicate for those indices with low len bits equal to huff */
        incr = 1U << (len - drop);
        fill = 1U << curr;
        min = fill;                 /* save offset to next table */
        pU32 = (uint32_t *)&next[(huff >> drop)]; // we know that the 'here' structure is 32-bits
        U32Here = *(uint32_t *)&here;  // but the compiler doesn't grasp that; helping it doubles the speed of this loop
        do {
            fill -= incr;
            pU32[fill] = U32Here;
            //next[(huff >> drop) + fill] = here;
        } while (fill != 0);

        /* backwards increment the len-bit code huff */
        incr = 1U << (len - 1);
        while (huff & incr)
            incr >>= 1;
        if (incr != 0) {
            huff &= incr - 1;
            huff += incr;
        }
        else
            huff = 0;

        /* go to next symbol, update count, len */
        sym++;
        if (--(count[len]) == 0) {
            if (len == max) break;
            len = lens[work[sym]];
        }

        /* create new sub-table if needed */
        if (len > root && (huff & mask) != low) {
            /* if first time, transition to sub-tables */
            if (drop == 0)
                drop = root;

            /* increment past last table */
            next += min;            /* here min is 1 << curr */

            /* determine length of next table */
            curr = len - drop;
            left = (int)(1 << curr);
            while (curr + drop < max) {
                left -= count[curr + drop];
                if (left <= 0) break;
                curr++;
                left <<= 1;
            }

            /* check for enough space */
            used += 1U << curr;
            if ((type == LENS && used > ENOUGH_LENS) ||
                (type == DISTS && used > ENOUGH_DISTS))
                return 1;

            /* point entry in root table to sub-table */
            low = huff & mask;
            (*table)[low].op = (unsigned char)curr;
            (*table)[low].bits = (unsigned char)root;
            (*table)[low].val = (unsigned short)(next - *table);
        }
    }

    /* fill in remaining table entry if code is incomplete (guaranteed to have
       at most one remaining entry, since if the code is incomplete, the
       maximum code length that was allowed to get this far is one bit) */
    if (huff != 0) {
        here.op = (unsigned char)64;            /* invalid code marker */
        here.bits = (unsigned char)(len - drop);
        here.val = (unsigned short)0;
        next[huff] = here;
    }

    /* set return parameters */
    *table += used;
    *bits = root;
    return 0;
} /* zt_table() */

int zt_init(zt_state *state)
{
    if (state) {
        memset(state, 0, sizeof(zt_state));
    } else {
        return ZT_INVALID_PARAMETER;
    }
    return ZT_SUCCESS;
} /* zt_init() */
//
// Return uncompressed size and optional filename info
// about a gzip compressed block of data. An invalid block
// or other error will return 0 as the data size
//
uint32_t zt_gzip_info(uint8_t *pCompressed, int iSize, char *szName, uint32_t *pu32Time)
{
    uint32_t iUncompSize;
    uint8_t u8Flags, u8ExtraFlags, *s = pCompressed;
    // Parse the gzip header
    if (s[0] != 0x1f || s[1] != 0x8b || s[2] != 0x08) { // not a gzip file
//        printf("Not a gzip file!\n");
        return 0;
    }
    s += 3;
    u8Flags = *s++;
    if (pu32Time) {
        *pu32Time = *(uint32_t *)s; // Unix time stamp
    }
    s += 4;
    u8ExtraFlags = *s++;
    s++; // skip the operating system
    
    if (u8Flags & 1) { // text
        s += strlen((const char *)s) + 1;
    }
    if (u8Flags & 2) { // header crc
    }
    if (u8Flags & 4) {// "Extra" follows
    }
    if (u8Flags & 8) { // Name follows
        if (szName) {
            strcpy(szName, (const char *)s);
        }
        s += strlen((const char *)s) + 1;
    }
    iUncompSize = *(uint32_t *)&pCompressed[iSize-4]; // last 4 bytes has uncompressed size
    return iUncompSize;
} /* zt_gzip_info() */

int zt_gunzip(uint8_t *pCompressed, int iSize, uint8_t *pUncompressed)
{
    zt_state state;
    zt_buffer buffer;
    uint8_t *s, u8Flags, u8ExtraFlags;
    int rc;
    uint64_t iCompSize, iUncompSize;
    
    zt_init(&state);
    state.wbits = 15; // fixed value for GZIP data

    s = pCompressed;
    // Parse the gzip header
    if (s[0] != 0x1f || s[1] != 0x8b || s[2] != 0x08) { // not a gzip file
      //  printf("Not a gzip file!\n");
        return ZT_HEADER_ERROR;
    }
    s += 3;
    u8Flags = *s++;
//    u32Time = *(uint32_t *)s; // Unix time stamp
    s += 4;
    u8ExtraFlags = *s++;
    s++; // skip the operating system
    
    if (u8Flags & 1) { // text
        s += strlen((const char *)s) + 1;
    }
    if (u8Flags & 2) { // header crc
    }
    if (u8Flags & 4) {// "Extra" follows
    }
    if (u8Flags & 8) { // Name follows
        //strcpy(szOutName, (const char *)s);
        s += strlen((const char *)s) + 1;
    }
    iUncompSize = *(uint32_t *)&pCompressed[iSize-4]; // last 4 bytes has uncompressed size
//    printf("Compressed size = %ul, uncompressed size = %ul\n", iCompSize, iUncompSize);
    buffer.avail_in = (uint32_t)(iSize - 8 - (int)(s - pCompressed));
    buffer.next_in = s;
    buffer.avail_out = (uint32_t)iUncompSize;
    buffer.total_out = 0;
    buffer.next_out = pUncompressed;
    rc = zt_inflate(&state, &buffer, 1);
    return rc;
} /* zt_gunzip() */

//
// Inflate the given deflated data into the output buffer
// This can be called repeatedly with small chunks of data,
// ** BUT ** the output buffer must be allocated large enough
// to hold all of the decompressed data in one shot. This is so
// that a separate memory window does not need to exist. This
// behavior diverges from the original zlib, but allows for a
// simpler implementation that's also faster.
//
// returns:
//
// - ZT_SUCCESS (the current set of compressed blocks is fully decoded)
// - ZT_ERROR... (an error occurred)
// - ZT_INPUT_INSUFFICIENT (the decoding isn't complete; it needs more data)
//
int zt_inflate(zt_state *state, zt_buffer *buffer, int bEnd)
{
    int ret, iLen;
    BIGUINT ulBitCount, ulBits, lmask, dmask;
    uint8_t *pBuf;
    uint8_t *pEndOfInput, *pEndOfOutput;
    uint8_t *pOut;
    uint32_t u32;
    int i;
    int bLastBlock = 0;
    uint8_t u8, *from;
    unsigned int op, dist, copy, len;
    int nlen, ndist, ncode; // huffman table vars
    code here;
    code const *lcode;
    code const *dcode;
    
    if (state == NULL || buffer == NULL) return ZT_INVALID_PARAMETER;
    state->iLastError = ZT_SUCCESS; // start by assuming success
    pOut = buffer->next_out;
    pBuf = buffer->next_in;
    
    pEndOfOutput = (uint8_t *)buffer->next_out + buffer->avail_out;
    pEndOfInput = &pBuf[(buffer->avail_in)-sizeof(BIGUINT)]; // keep it from reading past the end
    if (bEnd) {
        pEndOfInput += sizeof(BIGUINT) + sizeof(BIGUINT)/2; // this is the final blob of data, let it read until the last byte
    }
    // Get some data to start
    ulBitCount = state->ulBitCount;
    ulBits = state->ulBits;
    if (ulBitCount <= REGISTER_WIDTH/2) {
        GETMOREBITS
    }
    if (ulBitCount <= REGISTER_WIDTH/2) { // we must be starting from 0
        GETMOREBITS
    }
    if (state->wbits == 0) { // header hasn't been parsed yet
        // assume zlib header
        u8 = ulBits & 0xf; // first 4 bits are the compression type
        if (u8 != 8) // DEFLATE = 8
            return -1; // we can only handle DEFLATE streams
        state->wbits = u8 = ((ulBits >> 4) & 0xf) + 8; // log2 of window size minus 8
        //iWindowSize = 1 << u8;
        u8 = (ulBits >> 8) & 0xff; // flags
        ulBitCount -= 16;
        ulBits >>= 16;
        if (u8 & 0x20) { // fixed dictionary present
            u8 = (uint8_t)ulBits; // 8-bit checksum
            ulBitCount -= 8;
            ulBits >>= 8;
        }
    }
next_block:
    while (!bLastBlock && pBuf < pEndOfInput && pOut < pEndOfOutput) {
        if (ulBitCount <= REGISTER_WIDTH/2) { // get more bits
            GETMOREBITS;
        }
        if (state->lenbits == 0) { // need to parse the block header
            if ((pEndOfInput - pBuf) < 96) {
                goto need_more_data; // don't risk running out of data while decoding the block header
            }
            bLastBlock = BITS(1);
            DROPBITS(1);
            u8 = BITS(2);
            DROPBITS(2);
            switch (u8) {
                case 0: // stored
                    NEXTBYTE(ulBitCount);
                    u32 = ulBits & 0xffffffff;
                    ulBitCount -= 32;
                    if ((u32 & 0xffff) != ((u32 >> 16) ^ 0xffff)) {
                        // bad length
                        return -1;
                    }
                    iLen = u32 & 0xffff;
                    pBuf += ((REGISTER_WIDTH-ulBitCount) >> 3);
                    memcpy(pOut, pBuf, iLen);
                    pBuf += iLen;
                    pOut += iLen;
                    ulBits = *(BIGUINT *)pBuf; // get more bits to work with
                    ulBitCount = REGISTER_WIDTH;
                    continue; // next block
                    break;
                case 1: // static Huffman table
                    state->lencode = lenfix;
                    state->lenbits = 9;
                    state->distcode = distfix;
                    state->distbits = 5;
                    break;
                case 2: // dynamic Huffman table
                    nlen = BITS(5);
                    nlen += 257;
                    state->nlen = nlen;
                    DROPBITS(5);
                    ndist = BITS(5);
                    ndist += 1;
                    state->ndist = ndist;
                    DROPBITS(5);
                    ncode = BITS(4);
                    ncode += 4;
                    state->ncode = ncode;
                    DROPBITS(4);
                    // get the lengths of the lengths
                    for (i=0; i<ncode; i++) {
                        uint16_t codelen;
                        if (ulBitCount <= REGISTER_WIDTH/2) { // get more bits
                            GETMOREBITS;
                        }
                        codelen = BITS(3);
                        state->lens[len_order[i]] = codelen;
                        DROPBITS(3); // 3 bits per length
                    }
                    while (i < 19) {
                        state->lens[len_order[i++]] = 0;
                    }
                    state->have = 19;
                    state->next = state->codes;
                    state->lencode = (const code *)(state->next);
                    state->lenbits = 7;
                    ret = zt_table(CODES,
                                        state->lens,
                                        19,
                                        &(state->next),
                                        &(state->lenbits),
                                        state->work);
                    if (ret) {
                        //   strm->msg = (char *)"invalid code lengths set";
                        state->iLastError = ZT_DECODE_ERROR;
                        break;
                    }
                    state->have = 0;
                    
                    //state->mode = CODELENS;
                    while (state->have < state->nlen + state->ndist) {
                        if (ulBitCount <= REGISTER_WIDTH/2) { // get more bits
                            GETMOREBITS;
                        }
                        here = state->lencode[BITS(state->lenbits)];
                        if (here.val < 16) {
                            DROPBITS(here.bits);
                            state->lens[state->have++] = here.val;
                        }
                        else {
                            if (here.val == 16) {
                                if (ulBitCount <= REGISTER_WIDTH/2) GETMOREBITS
                                    //NEEDBITS(here.bits + 2);
                                    DROPBITS(here.bits);
                                if (state->have == 0) {
                                    state->iLastError = ZT_DECODE_ERROR;
                                    break;
                                }
                                len = state->lens[state->have - 1];
                                copy = 3 + BITS(2);
                                DROPBITS(2);
                            }
                            else if (here.val == 17) {
                                if (ulBitCount <= REGISTER_WIDTH/2) GETMOREBITS
                                    //NEEDBITS(here.bits + 3);
                                    DROPBITS(here.bits);
                                len = 0;
                                copy = 3 + BITS(3);
                                DROPBITS(3);
                            }
                            else {
                                if (ulBitCount <= REGISTER_WIDTH/2) GETMOREBITS
                                    //NEEDBITS(here.bits + 7);
                                    DROPBITS(here.bits);
                                len = 0;
                                copy = 11 + BITS(7);
                                DROPBITS(7);
                            }
                            if (state->have + copy > state->nlen + state->ndist) {
                                //strm->msg = (char *)"invalid bit length repeat";
                                state->iLastError = ZT_DECODE_ERROR;
                                break;
                            }
                            while (copy--)
                                state->lens[state->have++] = (uint16_t)len;
                        }
                    }
                    
                    /* handle error breaks in while */
                    // if (state->mode == BAD) break;
                    
                    /* check for end-of-block code (better have one) */
                    if (state->lens[256] == 0) {
                        // strm->msg = (char *)"invalid code -- missing end-of-block";
                        state->iLastError = ZT_DECODE_ERROR;
                        break;
                    }
                    
                    /* build code tables -- note: do not change the lenbits or distbits
                     values here (9 and 6) without reading the comments in inftrees.h
                     concerning the ENOUGH constants, which depend on those values */
                    state->next = state->codes;
                    state->lencode = (const code *)(state->next);
                    state->lenbits = 9;
                    ret = zt_table(LENS, state->lens, state->nlen, &(state->next),
                                        &(state->lenbits), state->work);
                    if (ret) {
                        // strm->msg = (char *)"invalid literal/lengths set";
                        state->iLastError = ZT_DECODE_ERROR;
                        break;
                    }
                    state->distcode = (const code *)(state->next);
                    state->distbits = 6;
                    ret = zt_table(DISTS, state->lens + state->nlen, state->ndist,
                                        &(state->next), &(state->distbits), state->work);
                    if (ret) {
                        // strm->msg = (char *)"invalid distances set";
                        state->iLastError = ZT_DECODE_ERROR;
                        break;
                    }
                    break;
                case 3: // reserved
                    break;
            } // switch on block type
            } // need to parse block header
                // Decode the block
                lmask = (1U << state->lenbits) - 1;
                dmask = (1U << state->distbits) - 1;
                //whave = state->whave;
                lcode = state->lencode;
                dcode = state->distcode;
                if (ulBitCount <= REGISTER_WIDTH/2) GETMOREBITS
                    
                while (pBuf < pEndOfInput && pOut < pEndOfOutput) {
#if REGISTER_WIDTH == 32 // this check isn't needed on 64-bit machines
                    if (ulBitCount <= REGISTER_WIDTH/2) {
                        GETMOREBITS
                    }
#endif
                    here = lcode[ulBits & lmask];
                get_length:
                    if (ulBitCount <= REGISTER_WIDTH/2) {
                        GETMOREBITS
                    }
                    DROPBITS(here.bits);
                    op = (unsigned)(here.op);
                    if (op == 0) {                          /* literal */
                       // Tracevv((stderr, here->val >= 0x20 && here->val < 0x7f ?
                       //         "inflate:         literal '%c'\n" :
                       //         "inflate:         literal 0x%02x\n", here->val));
                        *pOut++ = (unsigned char)(here.val);
                    }
                    else if (op & 16) {                     /* length base */
                        len = (unsigned)(here.val);
                        op &= 15;                           /* number of extra bits */
                        len += (unsigned)BITS(op);
                        DROPBITS(op);
                        //Tracevv((stderr, "inflate:         length %u\n", len));
#if REGISTER_WIDTH == 32 // this check isn't needed on 64-bit machines
                        if (ulBitCount <= REGISTER_WIDTH/2) {
                            GETMOREBITS
                        }
#endif
                    here = dcode[ulBits & dmask];
                get_distance:
                    if (ulBitCount <= REGISTER_WIDTH/2) {
                        GETMOREBITS
                    }
                    DROPBITS(here.bits);
                    op = (unsigned)(here.op);
                    if (op & 16) {                      /* distance base */
                        uint8_t *pEnd;
                        unsigned overlap;
                        dist = (unsigned)(here.val);
                        op &= 15;                       /* number of extra bits */
#if REGISTER_WIDTH == 32 // this check isn't needed on 64-bit machines
                        if (ulBitCount <= REGISTER_WIDTH/2) {
                            GETMOREBITS
                        }
#endif
                        dist += BITS(op);
                        DROPBITS(op);
                        from = pOut - dist;
                        pEnd = pOut+len;
                        overlap = (unsigned)(pOut-from);
                        // Check for a repeating pattern (source overlapping destination). This optimization can speed up
                        // decompression because we're only writing data instead of reading, then writing.
                        // For 1-byte patterns we create a register-sized set of bytes to blast them out faster
                        // for 2-8 byte patterns, we just write the pattern and increment the destination pointer by the pattern size
                        if (overlap < len && overlap <= sizeof(BIGUINT)) { // repeating pattern
                            BIGUINT pattern;
                            if (overlap == 1) { // special case, do it faster
                                pattern = *from;
                                pattern = pattern | (pattern << 8);
                                pattern = pattern | (pattern << 16);
#if REGISTER_WIDTH == 64
                                pattern = pattern | (pattern << 32);
#endif
                                while (pOut < pEnd) { // 2-8 byte pattern
                                    *(BIGUINT *)pOut = pattern;
                                    pOut += sizeof(BIGUINT);
                                }
                            } else { // 2-8 byte pattern
                                pattern = *(BIGUINT *)from; // use 2-8 bytes of pattern
                                while (pOut < pEnd) {
                                    *(BIGUINT *)pOut = pattern;
                                    pOut += overlap;
                                }
                            }
                        } else { // regular copy (source and dest don't overlap by <= 8 bytes)
                            while (pOut < pEnd) {
                                *(BIGUINT *)pOut = *(BIGUINT *)from;
                                pOut += sizeof(BIGUINT);
                                from += sizeof(BIGUINT);
                            }
                        }
                        // correct for possible overshoot of destination ptr
                        pOut = pEnd;
                    } // op & 16
                    else if ((op & 64) == 0) {          /* 2nd level distance code */
                        if (ulBitCount <= REGISTER_WIDTH/2) {
                            GETMOREBITS
                        }
                        here = dcode[here.val + BITS(op)];
                        goto get_distance;
                    }
                    else {
                        //strm->msg = (char *)"invalid distance code";
                        state->iLastError = ZT_DECODE_ERROR;
                        break;
                    }
                }
                else if ((op & 64) == 0) {     /* 2nd level length code */
                    if (ulBitCount <= REGISTER_WIDTH/2) {
                        GETMOREBITS
                    }
                    here = lcode[here.val + BITS(op)];
                    goto get_length;
                }
                else if (op & 32) {                     /* end-of-block */
                   // Tracevv((stderr, "inflate:         end of block\n"));
                   // state->mode = TYPE;
                   // break;
                    state->lenbits = 0; // mark that we completed the current block
                    goto next_block;
                }
                else {
                    //strm->msg = (char *)"invalid literal/length code";
                    state->iLastError = ZT_DECODE_ERROR;
                    break;
                }
                } // while decoding the current block
    } // while !bLastBlock
need_more_data:
    if (state->iLastError == ZT_SUCCESS && !bLastBlock) {
        state->iLastError = ZT_INPUT_INSUFFICIENT; // need more data
    }
    state->ulBits = ulBits;
    state->ulBitCount = ulBitCount;
    buffer->total_in += (int)(pBuf - buffer->next_in);
    buffer->avail_in -= (int)(pBuf - buffer->next_in);
    buffer->next_in = pBuf;
    state->bLastBlock = bLastBlock;
    buffer->total_out += (int)(intptr_t)(pOut - buffer->next_out);
    buffer->next_out = pOut;
    if (buffer->total_out == buffer->avail_out) {
        state->iLastError = ZT_SUCCESS; // we produced the full output, we're done
    }
    return state->iLastError;
} /* zt_inflate() */
#ifdef __cplusplus
}
#endif
//
// C++ Wrapper Class methods
//
// Initialize the structures to inflate raw deflated data
// Provide the output buffer and its capacity
//
void zlib_turbo::inflate_init(uint8_t *pOut, int iOutSize)
{
    zt_init(&_state);
    _buffer.next_out = pOut;
    _buffer.avail_out = iOutSize;
    _buffer.total_out = 0;
} /* inflate_init() */
//
// Inflate a block of deflated data
// You can feed the inflater incrementally, but the output buffer must be able to
// handle the full sized output from the first call
// The bLast flag tells the inflater that you've passed it the complete data (last chunk)
//
int zlib_turbo::inflate(uint8_t *pIn, int iInSize)
{
    _buffer.next_in = pIn;
    _buffer.avail_in = iInSize;
    return zt_inflate(&_state, &_buffer, 1);
} /* inflate() */
//
// Returns the amount of data decompressed from the gzip file
// if everything went well, this should be the same size as returned by gzip_info()
//
int zlib_turbo::outSize(void)
{
    return _buffer.total_out;
} /* outSize() */
//
// Return size, name and date/time info for a gzip file
// This is necessary to call first to know how large an output buffer will be needed
//
uint32_t zlib_turbo::gzip_info(uint8_t *pCompressed, int iSize, char *szName, uint32_t *pu32Time)
{
    return zt_gzip_info(pCompressed, iSize, szName, pu32Time);
} /* gzip_info() */
//
// Unzip a gzip file in one shot
//
int zlib_turbo::gunzip(uint8_t *pCompressed, int iInSize, uint8_t *pUncompressed)
{
    return zt_gunzip(pCompressed, iInSize, pUncompressed);
} /* gunzip() */
