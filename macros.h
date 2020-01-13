/*
 * macros.h
 *
 *  Created on: Oct 19, 2017, 11:55:27 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef MACROS_H_
#define MACROS_H_

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) && __IS_x86__

#include <x86intrin.h>

/* The masking of the right shift is needed to allow n == 0 (using
   just 32 - n and 64 - n results in undefined behaviour). Most uses
   of these macros use a constant and non-zero rotation count. */
#define ROTL32(x,n) ((uint32_t)__rold((unsigned int)(x), (int)(n)))

#if __IS_x64__
#define ROTL64(x,n) ((uint64_t)__rolq((unsigned long long)(x), (int)(n)))
#else
#define __bswapq(x) \
	((long long int)( ( __bswapd((int)(((x) >> 32) & 0xFFFFFFFFLL)) & 0xFFFFFFFFLL) \
					| ((__bswapd((int) ((x) & 0xFFFFFFFFLL)) & 0xFFFFFFFFLL) << 32) ))
#endif

#define _bswapw(x)	((uint16_t)__rolw((short int)(x), 8))
#define _bswapd(x)	((uint32_t)__bswapd((int)(x)))
#define _bswapq(x)	((uint64_t)__bswapq((long long int)(x)))

#define BE_READ_UINT16(p)	_bswapw(*((short int*)(p)))
#define BE_READ_UINT32(p)	_bswapd(*((int*)(p)))
#define BE_READ_UINT64(p)	_bswapq(*((long long int*)(p)))

#define LE_READ_UINT16(p)	(*((uint16_t*)(p)))
#define LE_READ_UINT32(p)	(*((uint32_t*)(p)))
#define LE_READ_UINT64(p)	(*((uint64_t*)(p)))

#define BE_WRITE_UINT16(p, i) do *((uint16_t*)(p)) = _bswapw(i); while (0)
#define BE_WRITE_UINT32(p, i) do *((uint32_t*)(p)) = _bswapd(i); while (0)
#define BE_WRITE_UINT64(p, i) do *((uint64_t*)(p)) = _bswapq(i); while (0)

#define LE_WRITE_UINT16(p, i) do *((uint16_t*)(p)) = (uint16_t)(i); while (0)
#define LE_WRITE_UINT32(p, i) do *((uint32_t*)(p)) = (uint32_t)(i); while (0)
#define LE_WRITE_UINT64(p, i) do *((uint64_t*)(p)) = (uint64_t)(i); while (0)

#else

/* Reads a 64-bit integer, in network, big-endian, byte order */
#define BE_READ_UINT64(p) \
(  (((uint64_t) ((const unsigned char*)(p))[0]) << 0x38) \
 | (((uint64_t) ((const unsigned char*)(p))[1]) << 0x30) \
 | (((uint64_t) ((const unsigned char*)(p))[2]) << 0x28) \
 | (((uint64_t) ((const unsigned char*)(p))[3]) << 0x20) \
 | (((uint64_t) ((const unsigned char*)(p))[4]) << 0x18) \
 | (((uint64_t) ((const unsigned char*)(p))[5]) << 0x10) \
 | (((uint64_t) ((const unsigned char*)(p))[6]) << 0x08) \
 |  ((uint64_t) ((const unsigned char*)(p))[7]) << 0x00)

/* Writes a 64-bit integer, in network, big-endian, byte order */
#define BE_WRITE_UINT64(p, i) \
do { \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x38) & 0xFF); \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x30) & 0xFF); \
	((unsigned char*)(p))[2] = (unsigned char)(((i) >> 0x28) & 0xFF); \
	((unsigned char*)(p))[3] = (unsigned char)(((i) >> 0x20) & 0xFF); \
	((unsigned char*)(p))[4] = (unsigned char)(((i) >> 0x18) & 0xFF); \
	((unsigned char*)(p))[5] = (unsigned char)(((i) >> 0x10) & 0xFF); \
	((unsigned char*)(p))[6] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[7] = (unsigned char)(((i) >> 0x00) & 0xFF); \
} while(0)

/* Reads a 32-bit integer, in network, big-endian, byte order */
#define BE_READ_UINT32(p) \
(  (((uint32_t) ((const unsigned char*)(p))[0]) << 0x18) \
 | (((uint32_t) ((const unsigned char*)(p))[1]) << 0x10) \
 | (((uint32_t) ((const unsigned char*)(p))[2]) << 0x08) \
 | (((uint32_t) ((const unsigned char*)(p))[3]) << 0x00))

/* Writes a 32-bit integer, in network, big-endian, byte order */
#define BE_WRITE_UINT32(p, i) \
do { \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x18) & 0xFF); \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x10) & 0xFF); \
	((unsigned char*)(p))[2] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[3] = (unsigned char)(((i) >> 0x00) & 0xFF); \
} while(0)

/* Analogous macros, for 16 bit numbers */
#define BE_READ_UINT16(p) \
(  (((uint16_t) ((const unsigned char*)(p))[0]) << 0x08) \
 | (((uint16_t) ((const unsigned char*)(p))[1]) << 0x00))

#define BE_WRITE_UINT16(p, i) \
do { \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x00) & 0xFF); \
} while(0)

/* And the other, little-endian, byte order */
#define LE_READ_UINT64(p) \
(  (((uint64_t) (((const unsigned char*)(p))[0] & 0xFF)) << 0x00) \
 | (((uint64_t) (((const unsigned char*)(p))[1] & 0xFF)) << 0x08) \
 | (((uint64_t) (((const unsigned char*)(p))[2] & 0xFF)) << 0x10) \
 | (((uint64_t) (((const unsigned char*)(p))[3] & 0xFF)) << x018) \
 | (((uint64_t) (((const unsigned char*)(p))[4] & 0xFF)) << 0x20) \
 | (((uint64_t) (((const unsigned char*)(p))[5] & 0xFF)) << 0x28) \
 | (((uint64_t) (((const unsigned char*)(p))[6] & 0xFF)) << 0x30) \
 | (((uint64_t) (((const unsigned char*)(p))[7] & 0xFF)) << 0x38))

#define LE_WRITE_UINT64(p, i) \
do { \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x00) & 0xFF); \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[2] = (unsigned char)(((i) >> 0x10) & 0xFF); \
	((unsigned char*)(p))[3] = (unsigned char)(((i) >> 0x18) & 0xFF); \
	((unsigned char*)(p))[4] = (unsigned char)(((i) >> 0x20) & 0xFF); \
	((unsigned char*)(p))[5] = (unsigned char)(((i) >> 0x28) & 0xFF); \
	((unsigned char*)(p))[6] = (unsigned char)(((i) >> 0x30) & 0xFF); \
	((unsigned char*)(p))[7] = (unsigned char)(((i) >> 0x38) & 0xFF); \
} while (0)

#define LE_READ_UINT32(p) \
(  (((uint32_t) (((const unsigned char*)(p))[3] & 0xFF)) << 0x18) \
 | (((uint32_t) (((const unsigned char*)(p))[2] & 0xFF)) << 0x10) \
 | (((uint32_t) (((const unsigned char*)(p))[1] & 0xFF)) << 0x08) \
 | (((uint32_t) (((const unsigned char*)(p))[0] & 0xFF)) << 0x00))

#define LE_WRITE_UINT32(p, i) \
do { \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x00) & 0xFF); \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[2] = (unsigned char)(((i) >> 0x10) & 0xFF); \
	((unsigned char*)(p))[3] = (unsigned char)(((i) >> 0x18) & 0xFF); \
} while(0)

/* Analogous macros, for 16 bit numbers */
#define LE_READ_UINT16(p) \
(  (((uint16_t) (((const unsigned char*)(p))[1] & 0xFF)) << 0x08) \
 | (((uint16_t) (((const unsigned char*)(p))[0] & 0xFF)) << 0x00))

#define LE_WRITE_UINT16(p, i) \
do { \
	((unsigned char*)(p))[1] = (unsigned char)(((i) >> 0x08) & 0xFF); \
	((unsigned char*)(p))[0] = (unsigned char)(((i) >> 0x00) & 0xFF); \
} while(0)

#endif

/* Analogous macros, for 24 bit numbers */
#define BE_READ_UINT24(p) \
(  (((uint32_t) (p)[0]) << 0x10) \
 | (((uint32_t) (p)[1]) << 0x08) \
 | (((uint32_t) (p)[2]) << 0x00))

#define BE_WRITE_UINT24(p, i) do { \
	(p)[0] = ((i) >> 0x10) & 0xff; \
	(p)[1] = ((i) >> 0x08) & 0xff; \
	(p)[2] = ((i) >> 0x00) & 0xff; \
} while(0)

#define LE_READ_UINT24(p) \
(  (((uint32_t) (p)[2]) << 0x10) \
 | (((uint32_t) (p)[1]) << 0x08) \
 | (((uint32_t) (p)[0]) << 0x00))

#define LE_WRITE_UINT24(p, i) do { \
	(p)[2] = ((i) >> 0x10) & 0xff; \
	(p)[1] = ((i) >> 0x08) & 0xff; \
	(p)[0] = ((i) >> 0x00) & 0xff; \
} while(0)

/* Macro to make it easier to loop over several blocks. */
#define FOR_BLOCKS(length, dst, src, blocksize)	\
for (; (length) >= (blocksize); \
		(length) -= (blocksize), \
		(dst) += (blocksize), \
		(src) += (blocksize) )
#define FOR_BLOCKS_INPLACE(length, src, blocksize) \
for (; (length) >= (blocksize); (length) -= (blocksize), (src) += (blocksize) )

/* Requires that size > 0 */
#define INCREMENT(size, ctr) \
do { \
	unsigned increment_i = (size) - 1; \
	if (++(ctr)[increment_i] == 0) { \
		while (increment_i > 0 && ++(ctr)[--increment_i] == 0); \
	} \
} while (0)

#define INCREMENT_BY(size, ctr, incr) \
do { \
	unsigned increment_i = (size) - 1; \
	(ctr)[increment_i] += (incr); \
	if ((ctr)[increment_i] < (incr)) { \
		while (increment_i > 0 && ++(ctr)[--increment_i] == 0); \
	} \
} while (0)

/* The masking of the right shift is needed to allow n == 0 (using
   just 32 - n and 64 - n results in undefined behaviour). Most uses
   of these macros use a constant and non-zero rotation count. */
#ifndef ROTL32
#define ROTL32(x,n) (((x)<<(n)) | ((x)>>((-(n))&31)))
#endif

#ifndef ROTL64
#define ROTL64(x,n) (((x)<<(n)) | ((x)>>((-(n))&63)))
#endif

#endif /* MACROS_H_ */
