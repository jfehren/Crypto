/*
 * gcm.c
 *
 *  Created on: Mar 8, 2019, 9:47:28 AM
 *      Author: Joshua Fehrenbach
 */

#include "gcm.h"
#include "memops.h"
#include "macros.h"
#include "crypto-internal.h"

#ifndef GCM_TABLE_BITS
#  if (SIZEOF_LONG == 8 || SIZEOF_LONG == 4 || SIZEOF_LONG == 2) && CHAR_BIT == 8
#  define GCM_TABLE_BITS SIZEOF_LONG
#  else
#  define GCM_TABLE_BITS 0
#  endif
#endif

#include "gcm-tables.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define GHASH_POLYNOMIAL 0xE1UL

#define CRYPT(ctx,dst,src,len) (*(ctx)->encrypt)((ctx)->cipher_ctx, (dst), (src), (len))


#if __IS_x86__
#define ADD32(block, x) BE_WRITE_UINT32((block).b + 12, (x) + BE_READ_UINT32((block).b + 12))
#else
#define ADD32(block, x) do { \
	unsigned __add32_index = GCM_BLOCK_SIZE - 1; \
	unsigned __add32_x = (x); \
	uint8_t* __add32_ptr = (block).b; \
	__add32_ptr[__add32_index] += __add32_x; \
	if (__add32_ptr[__add32_index] < __add32_x) { \
		while (__add32_index > GCM_BLOCK_SIZE - 4 && ++__add32_ptr[--__add32_index] == 0); \
	} \
} while (0)
#endif
#define INC32(block) ADD32(block, 1)

static void
gcm_gf_add(union crypto_block16 *r,
		const union crypto_block16 *x, const union crypto_block16 *y) {
#if SIZEOF_LONG == 8 && CHAR_BIT == 8
	r->q[0] = x->q[0] ^ y->q[0];
	r->q[1] = x->q[1] ^ y->q[1];
#elif SIZEOF_LONG == 4 && CHAR_BIT == 8
	r->d[0] = x->d[0] ^ y->d[0];
	r->d[1] = x->d[1] ^ y->d[1];
	r->d[2] = x->d[2] ^ y->d[2];
	r->d[3] = x->d[3] ^ y->d[3];
#elif SIZEOF_LONG == 2 && CHAR_BIT == 8
	r->w[0] = x->w[0] ^ y->w[0]; r->w[1] = x->w[1] ^ y->w[1];
	r->w[2] = x->w[2] ^ y->w[2]; r->w[3] = x->w[3] ^ y->w[3];
	r->w[4] = x->w[4] ^ y->w[4]; r->w[5] = x->w[5] ^ y->w[5];
	r->w[6] = x->w[6] ^ y->w[6]; r->w[7] = x->w[7] ^ y->w[7];
#else
	for (unsigned i = 0; i < (16 / sizeof(uint_least8_t)); ++i) {
		r->b[i] = (x->b[i] ^ y->b[i]) & 0xff;
	}
#endif
}

static void
gcm_gf_shift(union crypto_block16 *r, const union crypto_block16 *x) {
#if SIZEOF_LONG == 8 && CHAR_BIT == 8
#  if WORDS_BIGENDIAN
	uint64_t mask = - (x->q[1] & 1);
	r->q[1] = (x->q[1] >> 1) | ((x->q[0] & 1) << 63);
	r->q[0] = (x->q[0] >> 1) ^ (mask & (GHASH_POLYNOMIAL << 56));
#  else
#  define RSHIFT_WORD(x) \
		((((x) & 0xfefefefefefefefeUL) >> 1) \
				| (((x) & 0x0001010101010101UL) << 15))
	uint64_t mask = - ((x->q[1] >> 56) & 1);
	r->q[1] = RSHIFT_WORD(x->q[1]) | ((x->q[0] >> 49) & 0x80);
	r->q[0] = RSHIFT_WORD(x->q[0]) ^ (mask & GHASH_POLYNOMIAL);
#  undef RSHIFT_WORD
#  endif
#elif SIZEOF_LONG == 4 && CHAR_BIT == 8
#  if WORDS_BIGENDIAN
	uint32_t mask = - (x->d[3] & 1);
	r->d[3] = (x->d[3] >> 1) | ((x->d[2] & 1) << 31);
	r->d[2] = (x->d[2] >> 1) | ((x->d[1] & 1) << 31);
	r->d[1] = (x->d[1] >> 1) | ((x->d[0] & 1) << 31);
	r->d[0] = (x->d[0] >> 1) ^ (mask & (GHASH_POLYNOMIAL << 24));
#  else
#  define RSHIFT_WORD(x) ((((x) & 0xfefefefeUL) >> 1) | (((x) & 0x00010101) << 15))
	uint32_t mask = - ((x->d[3] >> 24) & 1);
	r->d[3] = RSHIFT_WORD(x->d[3]) | ((x->d[2] >> 17) & 0x80);
	r->d[2] = RSHIFT_WORD(x->d[2]) | ((x->d[1] >> 17) & 0x80);
	r->d[1] = RSHIFT_WORD(x->d[1]) | ((x->d[0] >> 17) & 0x80);
	r->d[0] = RSHIFT_WORD(x->d[0]) ^ (mask & GHASH_POLYNOMIAL);
#  undef RSHIFT_WORD
#  endif
#elif SIZEOF_LONG == 2 && CHAR_BIT == 8
#  if WORDS_BIGENDIAN
	uint16_t mask = - (x->w[7] & 1);
	r->w[7] = (x->w[7] >> 1) | ((x->w[6] & 1) << 15);
	r->w[6] = (x->w[6] >> 1) | ((x->w[5] & 1) << 15);
	r->w[5] = (x->w[5] >> 1) | ((x->w[4] & 1) << 15);
	r->w[4] = (x->w[4] >> 1) | ((x->w[3] & 1) << 15);
	r->w[3] = (x->w[3] >> 1) | ((x->w[2] & 1) << 15);
	r->w[2] = (x->w[2] >> 1) | ((x->w[1] & 1) << 15);
	r->w[1] = (x->w[1] >> 1) | ((x->w[0] & 1) << 15);
	r->w[0] = (x->w[0] >> 1) ^ (mask & (GHASH_POLYNOMIAL << 8));
#  else
#  define RSHIFT_WORD(x) ((((x) & 0xfefeUL) >> 1) | (((x) & 0x0001) << 15))
	uint16_t mask = - ((x->w[7] >> 8) & 1);
	r->w[7] = RSHIFT_WORD(x->w[7]) | ((x->w[6] >> 1) & 0x80);
	r->w[6] = RSHIFT_WORD(x->w[6]) | ((x->w[5] >> 1) & 0x80);
	r->w[5] = RSHIFT_WORD(x->w[5]) | ((x->w[4] >> 1) & 0x80);
	r->w[4] = RSHIFT_WORD(x->w[4]) | ((x->w[3] >> 1) & 0x80);
	r->w[3] = RSHIFT_WORD(x->w[3]) | ((x->w[2] >> 1) & 0x80);
	r->w[2] = RSHIFT_WORD(x->w[2]) | ((x->w[1] >> 1) & 0x80);
	r->w[1] = RSHIFT_WORD(x->w[1]) | ((x->w[0] >> 1) & 0x80);
	r->w[0] = RSHIFT_WORD(x->w[0]) ^ (mask & GHASH_POLYNOMIAL);
#  undef RSHIFT_WORD
#  endif
#else
	uint_least8_t mask = (-(x->b[15] & 1)) & 0xff;
	for (unsigned i = (16 / sizeof(uint_least8_t)) - 1; i > 0; --i) {
		r->b[i] = ((x->b[i] >> 1) & 0x7f) | ((x->b[i-1] << 7) & 0x80);
	}
	r->b[0] = ((r->b[0] >> 1) & 0x7f) ^ (mask & GHASH_POLYNOMIAL)
#endif
}

#if GCM_TABLE_BITS == 8

static void
gcm_gf_shift8(union crypto_block16 *x) {
#if WORDS_BIGENDIAN
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
	uint64_t reduce = shift_table[x->q[1] & 0xff];
	x->q[1] = (x->q[1] >> 8) | ((x->q[0] & 0xff) << 56);
	x->q[0] = (x->q[0] >> 8) ^ (reduce << 48);
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
	uint32_t reduce = shift_table[x->d[3] & 0xff];
	x->d[3] = (x->d[3] >> 8) | ((x->d[2] & 0xff) << 24);
	x->d[2] = (x->d[2] >> 8) | ((x->d[1] & 0xff) << 24);
	x->d[1] = (x->d[1] >> 8) | ((x->d[0] & 0xff) << 24);
	x->d[0] = (x->d[0] >> 8) ^ (reduce << 16);
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
	uint16_t reduce = shift_table[x->w[7] & 0xff];
	x->w[7] = (x->w[7] >> 8) | ((x->w[6] & 0xff) << 8);
	x->w[6] = (x->w[6] >> 8) | ((x->w[5] & 0xff) << 8);
	x->w[5] = (x->w[5] >> 8) | ((x->w[4] & 0xff) << 8);
	x->w[4] = (x->w[4] >> 8) | ((x->w[3] & 0xff) << 8);
	x->w[3] = (x->w[3] >> 8) | ((x->w[2] & 0xff) << 8);
	x->w[2] = (x->w[2] >> 8) | ((x->w[1] & 0xff) << 8);
	x->w[1] = (x->w[1] >> 8) | ((x->w[0] & 0xff) << 8);
	x->w[0] = (x->w[0] >> 8) ^ reduce;
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0xff];
	uint_least8_t r1 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r0 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = x->b[14] & 0xff; x->b[14] = x->b[13] & 0xff;
	x->b[13] = x->b[12] & 0xff; x->b[12] = x->b[11] & 0xff;
	x->b[11] = x->b[10] & 0xff; x->b[10] = x->b[ 9] & 0xff;
	x->b[ 9] = x->b[ 8] & 0xff; x->b[ 8] = x->b[ 7] & 0xff;
	x->b[ 7] = x->b[ 6] & 0xff; x->b[ 6] = x->b[ 5] & 0xff;
	x->b[ 5] = x->b[ 4] & 0xff; x->b[ 4] = x->b[ 3] & 0xff;
	x->b[ 3] = x->b[ 2] & 0xff; x->b[ 2] = x->b[ 1] & 0xff;
	x->b[ 1] =(x->b[ 0] ^ r0) & 0xff;
	x->b[ 0] = r1;
#  endif
#else
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
	uint64_t reduce = shift_table[(x->q[1] >> 56) & 0xff];
	x->q[1] = (x->q[1] << 8) | (x->q[0] >> 56);
	x->q[0] = (x->q[0] << 8) ^ reduce;
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
	uint32_t reduce = shift_table[(x->d[3] >> 24) & 0xff];
	x->d[3] = (x->d[3] << 8) | (x->d[2] >> 24);
	x->d[2] = (x->d[2] << 8) | (x->d[1] >> 24);
	x->d[1] = (x->d[1] << 8) | (x->d[0] >> 24);
	x->d[0] = (x->d[0] << 8) ^ reduce;
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
	uint16_t reduce = shift_table[(x->w[7] >> 8) & 0xff];
	x->w[7] = (x->w[7] << 8) | (x->w[6] >> 8);
	x->w[6] = (x->w[6] << 8) | (x->w[5] >> 8);
	x->w[5] = (x->w[5] << 8) | (x->w[4] >> 8);
	x->w[4] = (x->w[4] << 8) | (x->w[3] >> 8);
	x->w[3] = (x->w[3] << 8) | (x->w[2] >> 8);
	x->w[2] = (x->w[2] << 8) | (x->w[1] >> 8);
	x->w[1] = (x->w[1] << 8) | (x->w[0] >> 8);
	x->w[0] = (x->w[0] << 8) ^ reduce;
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0xff];
	uint_least8_t r0 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r1 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = x->b[14] & 0xff; x->b[14] = x->b[13] & 0xff;
	x->b[13] = x->b[12] & 0xff; x->b[12] = x->b[11] & 0xff;
	x->b[11] = x->b[10] & 0xff; x->b[10] = x->b[ 9] & 0xff;
	x->b[ 9] = x->b[ 8] & 0xff; x->b[ 8] = x->b[ 7] & 0xff;
	x->b[ 7] = x->b[ 6] & 0xff; x->b[ 6] = x->b[ 5] & 0xff;
	x->b[ 5] = x->b[ 4] & 0xff; x->b[ 4] = x->b[ 3] & 0xff;
	x->b[ 3] = x->b[ 2] & 0xff; x->b[ 2] = x->b[ 1] & 0xff;
	x->b[ 1] =(x->b[ 0] ^ r0) & 0xff;
	x->b[ 0] = r1;
#  endif
#endif
}

static void
gcm_gf_mul(union crypto_block16 *x, const union crypto_block16 *table) {
	union crypto_block16 z;
	unsigned i = GCM_BLOCK_SIZE;
	memcpy(z.b, table[x->b[GCM_BLOCK_SIZE-1]].b, GCM_BLOCK_SIZE);
	do {
		gcm_gf_shift8(&z);
		gcm_gf_add(&z, &z, &table[x->b[--i]]);
	} while (i > 0);
	gcm_gf_shift8(&z);
	gcm_gf_add(x, &z, &table[x->b[0]]);
}

#elif GCM_TABLE_BITS == 4

static void
gcm_gf_shift4(union crypto_block16 *x) {
#if WORDS_BIGENDIAN
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
	uint64_t reduce = shift_table[x->q[1] & 0xf];
	x->q[1] = (x->q[1] >> 4) | ((x->q[0] & 0xf) << 60);
	x->q[0] = (x->q[0] >> 4) ^ (reduce << 48);
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
	uint32_t reduce = shift_table[d[3] & 0xf];
	x->d[3] = (x->d[3] >> 4) | ((x->d[2] & 0xf) << 28);
	x->d[2] = (x->d[2] >> 4) | ((x->d[1] & 0xf) << 28);
	x->d[1] = (x->d[1] >> 4) | ((x->d[0] & 0xf) << 28);
	x->d[0] = (x->d[0] >> 4) ^ (reduce << 16);
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
	uint16_t reduce = shift_table[x->w[7] & 0xf];
	x->w[7] = (x->w[7] >> 4) | ((x->w[6] & 0xf) << 12);
	x->w[6] = (x->w[6] >> 4) | ((x->w[5] & 0xf) << 12);
	x->w[5] = (x->w[5] >> 4) | ((x->w[4] & 0xf) << 12);
	x->w[4] = (x->w[4] >> 4) | ((x->w[3] & 0xf) << 12);
	x->w[3] = (x->w[3] >> 4) | ((x->w[2] & 0xf) << 12);
	x->w[2] = (x->w[2] >> 4) | ((x->w[1] & 0xf) << 12);
	x->w[1] = (x->w[1] >> 4) | ((x->w[0] & 0xf) << 12);
	x->w[0] = (x->w[0] >> 4) ^ reduce;
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0xf];
	uint_least8_t r1 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r0 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = ((x->b[15] & 0xf0) >> 4) | ((x->b[14] & 0x0f) << 4);
	x->b[14] = ((x->b[14] & 0xf0) >> 4) | ((x->b[13] & 0x0f) << 4);
	x->b[13] = ((x->b[13] & 0xf0) >> 4) | ((x->b[12] & 0x0f) << 4);
	x->b[12] = ((x->b[12] & 0xf0) >> 4) | ((x->b[11] & 0x0f) << 4);
	x->b[11] = ((x->b[11] & 0xf0) >> 4) | ((x->b[10] & 0x0f) << 4);
	x->b[10] = ((x->b[10] & 0xf0) >> 4) | ((x->b[ 9] & 0x0f) << 4);
	x->b[ 9] = ((x->b[ 9] & 0xf0) >> 4) | ((x->b[ 8] & 0x0f) << 4);
	x->b[ 8] = ((x->b[ 8] & 0xf0) >> 4) | ((x->b[ 7] & 0x0f) << 4);
	x->b[ 7] = ((x->b[ 7] & 0xf0) >> 4) | ((x->b[ 6] & 0x0f) << 4);
	x->b[ 6] = ((x->b[ 6] & 0xf0) >> 4) | ((x->b[ 5] & 0x0f) << 4);
	x->b[ 5] = ((x->b[ 5] & 0xf0) >> 4) | ((x->b[ 4] & 0x0f) << 4);
	x->b[ 4] = ((x->b[ 4] & 0xf0) >> 4) | ((x->b[ 3] & 0x0f) << 4);
	x->b[ 3] = ((x->b[ 3] & 0xf0) >> 4) | ((x->b[ 2] & 0x0f) << 4);
	x->b[ 2] = ((x->b[ 2] & 0xf0) >> 4) | ((x->b[ 1] & 0x0f) << 4);
	x->b[ 1] =(((x->b[ 1] & 0xf0) >> 4) | ((x->b[ 0] & 0x0f) << 4)) ^ r0;
	x->b[ 0] = ((x->b[ 0] & 0xf0) >> 4) ^ r1;
#  endif
#else
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
#define RSHIFT_WORD(x) \
	((((x) & 0xf0f0f0f0f0f0f0f0UL) >> 4) \
		| (((x) & 0x000f0f0f0f0f0f0fUL) << 12))
	uint64_t reduce = shift_table[(x->q[1] >> 56) & 0xf];
	x->q[1] = RSHIFT_WORD(x->q[1]) | ((x->q[0] >> 52) & 0xf0);
	x->q[0] = RSHIFT_WORD(x->q[0]) ^ reduce;
#  undef RSHIFT_WORD
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
#define RSHIFT_WORD(x) ((((x) & 0xf0f0f0f0) >> 4) | (((x) & 0x000f0f0f) << 12))
	uint32_t reduce = shift_table[(x->d[3] >> 24) & 0xf];
	x->d[3] = RSHIFT_WORD(x->d[3]) | ((x->d[2] >> 20) & 0xf0);
	x->d[2] = RSHIFT_WORD(x->d[2]) | ((x->d[1] >> 20) & 0xf0);
	x->d[1] = RSHIFT_WORD(x->d[1]) | ((x->d[0] >> 20) & 0xf0);
	x->d[0] = RSHIFT_WORD(x->d[0]) ^ reduce;
#  undef RSHIFT_WORD
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
#define RSHIFT_WORD(x) ((((x) & 0xf0f0) >> 4) | (((x) & 0x000f) << 12))
	uint16_t reduce = shift_table[(x->w[7] >> 8) & 0xf];
	x->w[7] = RSHIFT_WORD(x->w[7]) | ((x->w[6] >> 4) & 0xf0);
	x->w[6] = RSHIFT_WORD(x->w[6]) | ((x->w[5] >> 4) & 0xf0);
	x->w[5] = RSHIFT_WORD(x->w[5]) | ((x->w[4] >> 4) & 0xf0);
	x->w[4] = RSHIFT_WORD(x->w[4]) | ((x->w[3] >> 4) & 0xf0);
	x->w[3] = RSHIFT_WORD(x->w[3]) | ((x->w[2] >> 4) & 0xf0);
	x->w[2] = RSHIFT_WORD(x->w[2]) | ((x->w[1] >> 4) & 0xf0);
	x->w[1] = RSHIFT_WORD(x->w[1]) | ((x->w[0] >> 4) & 0xf0);
	x->w[0] = RSHIFT_WORD(x->w[0]) ^ reduce;
#  undef RSHIFT_WORD
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0xf];
	uint_least8_t r0 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r1 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = (((x->b[15] & 0xf0) >> 4) | ((x->b[14] & 0x0f) << 4)) & 0xff;
	x->b[14] = (((x->b[14] & 0xf0) >> 4) | ((x->b[13] & 0x0f) << 4)) & 0xff;
	x->b[13] = (((x->b[13] & 0xf0) >> 4) | ((x->b[12] & 0x0f) << 4)) & 0xff;
	x->b[12] = (((x->b[12] & 0xf0) >> 4) | ((x->b[11] & 0x0f) << 4)) & 0xff;
	x->b[11] = (((x->b[11] & 0xf0) >> 4) | ((x->b[10] & 0x0f) << 4)) & 0xff;
	x->b[10] = (((x->b[10] & 0xf0) >> 4) | ((x->b[ 9] & 0x0f) << 4)) & 0xff;
	x->b[ 9] = (((x->b[ 9] & 0xf0) >> 4) | ((x->b[ 8] & 0x0f) << 4)) & 0xff;
	x->b[ 8] = (((x->b[ 8] & 0xf0) >> 4) | ((x->b[ 7] & 0x0f) << 4)) & 0xff;
	x->b[ 7] = (((x->b[ 7] & 0xf0) >> 4) | ((x->b[ 6] & 0x0f) << 4)) & 0xff;
	x->b[ 6] = (((x->b[ 6] & 0xf0) >> 4) | ((x->b[ 5] & 0x0f) << 4)) & 0xff;
	x->b[ 5] = (((x->b[ 5] & 0xf0) >> 4) | ((x->b[ 4] & 0x0f) << 4)) & 0xff;
	x->b[ 4] = (((x->b[ 4] & 0xf0) >> 4) | ((x->b[ 3] & 0x0f) << 4)) & 0xff;
	x->b[ 3] = (((x->b[ 3] & 0xf0) >> 4) | ((x->b[ 2] & 0x0f) << 4)) & 0xff;
	x->b[ 2] = (((x->b[ 2] & 0xf0) >> 4) | ((x->b[ 1] & 0x0f) << 4)) & 0xff;
	x->b[ 1] =((((x->b[ 1] & 0xf0) >> 4) | ((x->b[ 0] & 0x0f) << 4)) & 0xff) ^ r0;
	x->b[ 0] = (((x->b[ 0] & 0xf0) >> 4) & 0xff) ^ r1;
#  endif
#endif
}

static void
gcm_gf_mul(union crypto_block16 *x, const union crypto_block16 *table) {
	union crypto_block16 z;
	unsigned i = GCM_BLOCK_SIZE;
	uint8_t b;
	memset(z.b, 0, GCM_BLOCK_SIZE);
	do {
		b = x->b[--i];
		gcm_gf_shift4(&z);
		gcm_gf_add(&z, &z, &table[b & 0xf]);	b >>= 4;
		gcm_gf_shift4(&z);
		gcm_gf_add(&z, &z, &table[b & 0xf]);
	} while (i > 0);
	b = x->b[0];
	gcm_gf_shift4(&z);
	gcm_gf_add(&z, &z, &table[b & 0xf]);	b >>= 4;
	gcm_gf_shift4(&z);
	gcm_gf_add(x, &z, &table[b & 0xf]);
}

#elif GCM_TABLE_BITS == 2

static void
gcm_gf_shift2(union crypto_block16 *x) {
#if WORDS_BIGENDIAN
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
	uint64_t reduce = shift_table[x->q[1] & 0x3];
	x->q[1] = (x->q[1] >> 2) | ((x->q[0] & 0x3) << 62);
	x->q[0] = (x->q[0] >> 2) ^ (reduce << 48);
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
	uint32_t reduce = shift_table[d[3] & 0x3];
	x->d[3] = (x->d[3] >> 2) | ((x->d[2] & 0x3) << 30);
	x->d[2] = (x->d[2] >> 2) | ((x->d[1] & 0x3) << 30);
	x->d[1] = (x->d[1] >> 2) | ((x->d[0] & 0x3) << 30);
	x->d[0] = (x->d[0] >> 2) ^ (reduce << 16);
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
	uint16_t reduce = shift_table[x->w[7] & 0x3];
	x->w[7] = (x->w[7] >> 2) | ((x->w[6] & 0x3) << 14);
	x->w[6] = (x->w[6] >> 2) | ((x->w[5] & 0x3) << 14);
	x->w[5] = (x->w[5] >> 2) | ((x->w[4] & 0x3) << 14);
	x->w[4] = (x->w[4] >> 2) | ((x->w[3] & 0x3) << 14);
	x->w[3] = (x->w[3] >> 2) | ((x->w[2] & 0x3) << 14);
	x->w[2] = (x->w[2] >> 2) | ((x->w[1] & 0x3) << 14);
	x->w[1] = (x->w[1] >> 2) | ((x->w[0] & 0x3) << 14);
	x->w[0] = (x->w[0] >> 2) ^ reduce;
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0x3];
	uint_least8_t r1 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r0 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = ((x->b[15] & 0xfc) >> 2) | ((x->b[14] & 0x03) << 6);
	x->b[14] = ((x->b[14] & 0xfc) >> 2) | ((x->b[13] & 0x03) << 6);
	x->b[13] = ((x->b[13] & 0xfc) >> 2) | ((x->b[12] & 0x03) << 6);
	x->b[12] = ((x->b[12] & 0xfc) >> 2) | ((x->b[11] & 0x03) << 6);
	x->b[11] = ((x->b[11] & 0xfc) >> 2) | ((x->b[10] & 0x03) << 6);
	x->b[10] = ((x->b[10] & 0xfc) >> 2) | ((x->b[ 9] & 0x03) << 6);
	x->b[ 9] = ((x->b[ 9] & 0xfc) >> 2) | ((x->b[ 8] & 0x03) << 6);
	x->b[ 8] = ((x->b[ 8] & 0xfc) >> 2) | ((x->b[ 7] & 0x03) << 6);
	x->b[ 7] = ((x->b[ 7] & 0xfc) >> 2) | ((x->b[ 6] & 0x03) << 6);
	x->b[ 6] = ((x->b[ 6] & 0xfc) >> 2) | ((x->b[ 5] & 0x03) << 6);
	x->b[ 5] = ((x->b[ 5] & 0xfc) >> 2) | ((x->b[ 4] & 0x03) << 6);
	x->b[ 4] = ((x->b[ 4] & 0xfc) >> 2) | ((x->b[ 3] & 0x03) << 6);
	x->b[ 3] = ((x->b[ 3] & 0xfc) >> 2) | ((x->b[ 2] & 0x03) << 6);
	x->b[ 2] = ((x->b[ 2] & 0xfc) >> 2) | ((x->b[ 1] & 0x03) << 6);
	x->b[ 1] =(((x->b[ 1] & 0xfc) >> 2) | ((x->b[ 0] & 0x03) << 6)) ^ r0;
	x->b[ 0] = ((x->b[ 0] & 0xfc) >> 2) ^ r1;
#  endif
#else
#  if SIZEOF_LONG == 8 && CHAR_BIT == 8
#define RSHIFT_WORD(x) \
	((((x) & 0xfcfcfcfcfcfcfcfcUL) >> 2) \
		| (((x) & 0x0003030303030303UL) << 14))
	uint64_t reduce = shift_table[(x->q[1] >> 56) & 0x3];
	x->q[1] = RSHIFT_WORD(x->q[1]) | ((x->q[0] >> 50) & 0xfc);
	x->q[0] = RSHIFT_WORD(x->q[0]) ^ reduce;
#  undef RSHIFT_WORD
#  elif SIZEOF_LONG == 4 && CHAR_BIT == 8
#define RSHIFT_WORD(x) ((((x) & 0xfcfcfcfc) >> 2) | (((x) & 0x00030303) << 14))
	uint32_t reduce = shift_table[(x->d[3] >> 24) & 0x3];
	x->d[3] = RSHIFT_WORD(x->d[3]) | ((x->d[2] >> 18) & 0xfc);
	x->d[2] = RSHIFT_WORD(x->d[2]) | ((x->d[1] >> 18) & 0xfc);
	x->d[1] = RSHIFT_WORD(x->d[1]) | ((x->d[0] >> 18) & 0xfc);
	x->d[0] = RSHIFT_WORD(x->d[0]) ^ reduce;
#  undef RSHIFT_WORD
#  elif SIZEOF_LONG == 2 && CHAR_BIT == 8
#define RSHIFT_WORD(x) ((((x) & 0xfcfc) >> 2) | (((x) & 0x0003) << 14))
	uint16_t reduce = shift_table[(x->w[7] >> 8) & 0x3];
	x->w[7] = RSHIFT_WORD(x->w[7]) | ((x->w[6] >> 2) & 0xfc);
	x->w[6] = RSHIFT_WORD(x->w[6]) | ((x->w[5] >> 2) & 0xfc);
	x->w[5] = RSHIFT_WORD(x->w[5]) | ((x->w[4] >> 2) & 0xfc);
	x->w[4] = RSHIFT_WORD(x->w[4]) | ((x->w[3] >> 2) & 0xfc);
	x->w[3] = RSHIFT_WORD(x->w[3]) | ((x->w[2] >> 2) & 0xfc);
	x->w[2] = RSHIFT_WORD(x->w[2]) | ((x->w[1] >> 2) & 0xfc);
	x->w[1] = RSHIFT_WORD(x->w[1]) | ((x->w[0] >> 2) & 0xfc);
	x->w[0] = RSHIFT_WORD(x->w[0]) ^ reduce;
#  undef RSHIFT_WORD
#  else
	uint_least16_t reduce = shift_table[x->b[15] & 0x3];
	uint_least8_t r0 = (uint_least8_t)((reduce >> 8) & 0xff);
	uint_least8_t r1 = (uint_least8_t)(reduce & 0xff);
	x->b[15] = ((x->b[15] & 0xfc) >> 2) | ((x->b[14] & 0x03) << 6);
	x->b[14] = ((x->b[14] & 0xfc) >> 2) | ((x->b[13] & 0x03) << 6);
	x->b[13] = ((x->b[13] & 0xfc) >> 2) | ((x->b[12] & 0x03) << 6);
	x->b[12] = ((x->b[12] & 0xfc) >> 2) | ((x->b[11] & 0x03) << 6);
	x->b[11] = ((x->b[11] & 0xfc) >> 2) | ((x->b[10] & 0x03) << 6);
	x->b[10] = ((x->b[10] & 0xfc) >> 2) | ((x->b[ 9] & 0x03) << 6);
	x->b[ 9] = ((x->b[ 9] & 0xfc) >> 2) | ((x->b[ 8] & 0x03) << 6);
	x->b[ 8] = ((x->b[ 8] & 0xfc) >> 2) | ((x->b[ 7] & 0x03) << 6);
	x->b[ 7] = ((x->b[ 7] & 0xfc) >> 2) | ((x->b[ 6] & 0x03) << 6);
	x->b[ 6] = ((x->b[ 6] & 0xfc) >> 2) | ((x->b[ 5] & 0x03) << 6);
	x->b[ 5] = ((x->b[ 5] & 0xfc) >> 2) | ((x->b[ 4] & 0x03) << 6);
	x->b[ 4] = ((x->b[ 4] & 0xfc) >> 2) | ((x->b[ 3] & 0x03) << 6);
	x->b[ 3] = ((x->b[ 3] & 0xfc) >> 2) | ((x->b[ 2] & 0x03) << 6);
	x->b[ 2] = ((x->b[ 2] & 0xfc) >> 2) | ((x->b[ 1] & 0x03) << 6);
	x->b[ 1] =(((x->b[ 1] & 0xfc) >> 2) | ((x->b[ 0] & 0x03) << 6)) ^ r0;
	x->b[ 0] = ((x->b[ 0] & 0xfc) >> 2) ^ r1;
#  endif
#endif
}

static void
gcm_gf_mul(union crypto_block16 *x, const union crypto_block16 *table) {
	union crypto_block16 z;
	unsigned i = GCM_BLOCK_SIZE;
	uint8_t b;
	memset(z.b, 0, GCM_BLOCK_SIZE);
	do {
		b = x->b[--i];
		gcm_gf_shift2(&z);
		gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
		gcm_gf_shift2(&z);
		gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
		gcm_gf_shift2(&z);
		gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
		gcm_gf_shift2(&z);
		gcm_gf_add(&z, &z, &table[b & 3]);
	} while (i > 0);
	b = x->b[0];
	gcm_gf_shift2(&z);
	gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
	gcm_gf_shift2(&z);
	gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
	gcm_gf_shift2(&z);
	gcm_gf_add(&z, &z, &table[b & 3]);	b >>= 2;
	gcm_gf_shift2(&z);
	gcm_gf_add(x, &z, &table[b & 3]);
}

#else

static void
gcm_gf_mul(union crypto_block16 *x, const union crypto_block16 *y) {
	union crypto_block16 z;
	unsigned i = GCM_BLOCK_SIZE, j;
	uint8_t b;
	memset(z.b, 0, GCM_BLOCK_SIZE);
	do {
		b = x->b[--i];
		for (j = 0; j < 8; j++) {
			gcm_gf_shift(&z, &z);
			if (b & 0x01) {
				gcm_gf_add(&z, &z, y);
			}
			b >>= 1;
		}
	} while (i > 0);
	b = x->b[0];
	gcm_gf_shift(x, &z);
	if (b & 0x01) gcm_gf_add(x, x, y);
	for (j = 1; j < 8; j++) {
		b >>= 1;
		gcm_gf_shift(x, x);
		if (b & 0x01) {
			gcm_gf_add(x, x, y);
		}
	}
}

#endif

struct ghash_gen {
	union crypto_block16 d;
	union crypto_block16 h[1 << GCM_TABLE_BITS];
};

static void
gcm_hash_gen(union crypto_block16 *hash,
		const uint8_t *data, size_t length) {
	const union crypto_block16 *h = &hash[1];
	union crypto_block16 *x = hash;
	while (length >= GCM_BLOCK_SIZE) {
		memxor (x->b, data, GCM_BLOCK_SIZE);
		gcm_gf_mul (x, h);
		data += GCM_BLOCK_SIZE;
		length -= GCM_BLOCK_SIZE;
	}
	if (length > 0) {
		memxor (x->b, data, length);
		memset (x->b + length, 0, GCM_BLOCK_SIZE - length);
		gcm_gf_mul (x, h);
	}
}

static void
gcm_init_hash_gen(union crypto_block16 *dst,
		const union crypto_block16 *src) {
	struct ghash_gen *ghash = (struct ghash_gen*)(dst);
	union crypto_block16 *d = &ghash->d;
	union crypto_block16 *h = ghash->h;
	unsigned i = (1 << GCM_TABLE_BITS) / 2;	/* Middle element of the table */
	memset(d->b, 0, sizeof(union crypto_block16));
	memcpy(h[i].b, src->b, sizeof(union crypto_block16));
#if GCM_TABLE_BITS != 0
	/* Do powers of 2 first, then do the rest by addition */
	while (i /= 2) {
		gcm_gf_shift(&h[i], &h[2*i]);
	}
	for (i = 2; i < (1 << GCM_TABLE_BITS); i *= 2) {
		for (unsigned j = 1; j < i; j++) {
			gcm_gf_add(&h[i+j], &h[i], &h[j]);
		}
	}
#endif
}

#if __IS_x86__ && CRYPTO_FAT

static void
gcm_hash_default(union crypto_block16 *hash,
		const uint8_t *data, size_t length);

extern void
_crypto_gcm_hash_pclmul(union crypto_block16 *hash,
		const uint8_t *data, size_t length);


static void
gcm_init_hash_default(union crypto_block16 *dst,
		const union crypto_block16 *src);

extern void
_crypto_gcm_init_hash_pclmul(union crypto_block16 *dst,
		const union crypto_block16 *src);


#define gcm_hash (*_gcm_hash)
#define gcm_init_hash (*_gcm_init_hash)

static void
gcm_hash(union crypto_block16 *hash,
		const uint8_t *data, size_t length) = &gcm_hash_default;

static void
gcm_init_hash(union crypto_block16 *dst,
		const union crypto_block16 *src) = &gcm_init_hash_default;

struct ghash_pclmul {
	union crypto_block16 d;			/* Current digest */
	union crypto_block16 h1;		/* H^1 */
	union crypto_block16 h2;		/* H^2 */
	union crypto_block16 h3;		/* H^3 */
	union crypto_block16 h4;		/* H^4 */
	union crypto_block16 h1_h2_m;	/* (HI(H^1)+LO(H^1)) || (HI(H^2)+LO(H^2)) */
	union crypto_block16 h3_h4_m;	/* (HI(H^3)+LO(H^3)) || (HI(H^4)+LO(H^4)) */
};

static size_t
hash_key_size() {
	static volatile size_t _hash_key_size = -1;
	if (_hash_key_size == -1) {
		if (is_x86_64() && is_pclmul()) {
			_gcm_hash = &_crypto_gcm_hash_pclmul;
			_gcm_init_hash = &_crypto_gcm_init_hash_pclmul;
			_hash_key_size = sizeof(struct ghash_pclmul);
		} else {
			_gcm_hash = &gcm_hash_gen;
			_gcm_init_hash = &gcm_init_hash_gen;
			_hash_key_size = sizeof(struct ghash_gen);
		}
	}
	return _hash_key_size;
}

static void
gcm_hash_default(union crypto_block16 *hash,
		const uint8_t *data, size_t length) {
	/* Call hash_key_size(), which sets _gcm_hash to the appropriate function */
	hash_key_size();
	gcm_hash(hash, data, length);
}

static void
gcm_init_hash_default(union crypto_block16 *dst,
		const union crypto_block16 *src) {
	/* Call hash_key_size(), which sets _gcm_init_hash to the appropriate function */
	hash_key_size();
	gcm_init_hash(dst, src);
}

#else

static inline size_t
hash_key_size() {
	return sizeof(struct ghash_gen);
}

#define gcm_hash gcm_hash_gen
#define gcm_init_hash gcm_init_hash_gen

#endif


static inline void
gcm_pad(struct gcm_ctx *ctx) {
	if (ctx->blength != 0) {
		memset(ctx->buf + ctx->blength, 0, GCM_BLOCK_SIZE - ctx->blength);
		gcm_hash(ctx->hash, ctx->buf, GCM_BLOCK_SIZE);
		ctx->blength = 0;
	}
}

static inline void
gcm_hash_sizes(struct gcm_ctx *ctx) {
	gcm_pad(ctx);
	ctx->auth_size *= 8;
	ctx->data_size *= 8;

	BE_WRITE_UINT64(ctx->buf + 0, ctx->auth_size);
	BE_WRITE_UINT64(ctx->buf + 8, ctx->data_size);

	gcm_hash(ctx->hash, ctx->buf, GCM_BLOCK_SIZE);
}

void
gcm_init(struct gcm_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	assert (cipher_meta->block_size == GCM_BLOCK_SIZE);
	const unsigned block_size = GCM_BLOCK_SIZE;
	const unsigned context_size = cipher_meta->context_size;
	const unsigned context_align = (cipher_meta->context_align < 16) ? 16
			: cipher_meta->context_align;
	const unsigned hash_size = hash_key_size();
	const unsigned off1 = (hash_size % context_align == 0) ? 0
			: context_align - (hash_size % context_align);
	const unsigned off2 = (context_size % context_align == 0) ? 0
			: context_align - (context_size % context_align);
	uint8_t *mem = aligned_alloc(context_align,
			hash_size + off1 + context_size + off2);
	ctx->hash = (union crypto_block16*)mem;
	mem += hash_size + off1;
	ctx->cipher_ctx = mem;
	ctx->blength = 0;
	ctx->auth_size = 0;
	ctx->data_size = 0;
	ctx->digest_size = block_size;
	ctx->cipher_size = context_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->set_key = cipher_meta->set_encrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
}

void
gcm_free(struct gcm_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->hash, 0, hash_key_size());
	memset(ctx->cipher_ctx, 0, ctx->cipher_size);
	/* Release memory */
	free(ctx->hash);
}

void
gcm_set_key_default(struct gcm_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *nonce, size_t nonce_length) {
	/* Call GCM initialization with default digest size */
	gcm_set_key(ctx, key, key_length, nonce, nonce_length, GCM_MAX_DIGEST_SIZE, 0, 0);
}

void
gcm_set_key(struct gcm_ctx *ctx, const uint8_t *key, size_t key_length, const uint8_t *nonce, size_t nonce_length,
		size_t digest_length, size_t message_length, size_t aad_length) {
	/* message_length and aad_length are both ignored; they only exist for compatibility
	 * with CCM mode of operation */
	(*ctx->set_key)(ctx->cipher_ctx, key, key_length);
	/* Initialize GHASH state */
	memset(ctx->buf, 0, GCM_BLOCK_SIZE);
	CRYPT(ctx, ctx->buf, ctx->buf, GCM_BLOCK_SIZE);
	gcm_init_hash(ctx->hash, ctx->buf);
	/* Generate IV */
	if (nonce_length == GCM_IV_SIZE) {
		memcpy(ctx->iv.b, nonce, GCM_IV_SIZE);
		BE_WRITE_UINT32(ctx->iv.b + GCM_IV_SIZE, 1);
		memset(ctx->iv.b + GCM_IV_SIZE, 0, 4);
		INC32(ctx->iv);		/* IV = nonce || 0x00000001 */
	} else {
		gcm_update(ctx, nonce, nonce_length);
		gcm_pad(ctx);
		nonce_length *= 8;
		unsigned i = GCM_BLOCK_SIZE;
		while (nonce_length != 0) {
			ctx->buf[--i] = nonce_length & 0xff;
			nonce_length >>= 8;
		}
		memset(ctx->buf, 0, i);
		gcm_hash(ctx->hash, ctx->buf, GCM_BLOCK_SIZE);
		memcpy(ctx->iv.b, ctx->buf, GCM_BLOCK_SIZE);
	}
	/* Set counter */
	memcpy(ctx->ctr.b, ctx->iv.b, GCM_BLOCK_SIZE);
	INC32(ctx->ctr);	/* ctr = INC32(IV) */
	/* Reset input length counters and set tag length */
	ctx->blength = 0;
	ctx->auth_size = 0;
	ctx->data_size = 0;
	ctx->digest_size = digest_length;
}

void
gcm_update(struct gcm_ctx *ctx, const uint8_t *aad, size_t length) {
	assert (ctx->data_size == 0);
	ctx->auth_size += length;
	if (ctx->blength + length < GCM_BLOCK_SIZE) {
		/* We don't have enough data to process, so save it for later */
		memcpy(ctx->buf + ctx->blength, aad, length);
		ctx->blength += length;
		return;
	}
	if (ctx->blength != 0) {
		/* We have data saved, so append AAD and process it */
		memcpy(ctx->buf + ctx->blength, aad, GCM_BLOCK_SIZE - ctx->blength);
		aad += GCM_BLOCK_SIZE - ctx->blength;
		length -= GCM_BLOCK_SIZE - ctx->blength;
		gcm_hash(ctx->hash, ctx->buf, GCM_BLOCK_SIZE);
	}
	/* Process full blocks */
	ctx->blength = length % GCM_BLOCK_SIZE;
	length -= ctx->blength;
	gcm_hash(ctx->hash, aad, length);
	/* Save any remaining data */
	if (ctx->blength != 0) {
		memcpy(ctx->buf, aad + length, ctx->blength);
	}
}

void
gcm_digest(struct gcm_ctx *ctx, const uint8_t *digest) {
	gcm_hash_sizes(ctx);
	CRYPT(ctx, ctx->buf, ctx->iv.b, GCM_BLOCK_SIZE);
	memxor3(digest, ctx->buf, ctx->hash[0].b, ctx->digest_size);
}

static inline void
gcm_extend_ctr(union crypto_block16 *dst, const union crypto_block16 *ctr) {
	uint32_t ctr0 = BE_READ_UINT32(ctr->b + 12);
#if SIZEOF_LONG == 8
	dst[0].q[0] = dst[1].q[0] = dst[2].q[0] = dst[3].q[0] = ctr->q[0];
	dst[0].q[1] = dst[1].q[1] = dst[2].q[1] = dst[3].q[1] = ctr->q[1];
#elif SIZEOF_LONG == 4
	dst[0].d[0] = dst[1].d[0] = dst[2].d[0] = dst[3].d[0] = ctr->d[0];
	dst[0].d[1] = dst[1].d[1] = dst[2].d[1] = dst[3].d[1] = ctr->d[1];
	dst[0].d[2] = dst[1].d[2] = dst[2].d[2] = dst[3].d[2] = ctr->d[2];
#elif SIZEOF_LONG == 2
	dst[0].w[0] = dst[1].w[0] = dst[2].w[0] = dst[3].w[0] = ctr->w[0];
	dst[0].w[1] = dst[1].w[1] = dst[2].w[1] = dst[3].w[1] = ctr->w[1];
	dst[0].w[2] = dst[1].w[2] = dst[2].w[2] = dst[3].w[2] = ctr->w[2];
	dst[0].w[3] = dst[1].w[3] = dst[2].w[3] = dst[3].w[3] = ctr->w[3];
	dst[0].w[4] = dst[1].w[4] = dst[2].w[4] = dst[3].w[4] = ctr->w[4];
	dst[0].w[5] = dst[1].w[5] = dst[2].w[5] = dst[3].w[5] = ctr->w[5];
#else
	memcpy(dst[0].b, ctr->b, 12);
	memcpy(dst[1].b, ctr->b, 12);
	memcpy(dst[2].b, ctr->b, 12);
	memcpy(dst[3].b, ctr->b, 12);
#endif
	BE_WRITE_UINT32((dst[0].b + 12), 0 + ctr0);
	BE_WRITE_UINT32((dst[1].b + 12), 1 + ctr0);
	BE_WRITE_UINT32((dst[2].b + 12), 2 + ctr0);
	BE_WRITE_UINT32((dst[3].b + 12), 3 + ctr0);
}

static void
gcm_crypt(struct gcm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	if (src != dst) {
		if (length >= 4*(4*GCM_BLOCK_SIZE)) {
			union {
				union crypto_block16 ctr[4];
				uint8_t bytes[4*GCM_BLOCK_SIZE];
			} ctr;
			gcm_extend_ctr(ctr.ctr, &ctx->ctr);
			/* Take advantage of native code that may be able to handle 4 blocks at once */
			while (length >= 4*GCM_BLOCK_SIZE) {
				CRYPT(ctx, dst, ctr.bytes, 4*GCM_BLOCK_SIZE);	/* encrypt 4 blocks */
				memxor(dst, src, 4*GCM_BLOCK_SIZE);
				ADD32(ctr.ctr[0], 4); ADD32(ctr.ctr[1], 4);
				ADD32(ctr.ctr[2], 4); ADD32(ctr.ctr[3], 4);
				src += 4*GCM_BLOCK_SIZE;
				dst += 4*GCM_BLOCK_SIZE;
				length -= 4*GCM_BLOCK_SIZE;
			}
			/* store first unused counter */
			memcpy(ctx->ctr.b, ctr.ctr[0].b, GCM_BLOCK_SIZE);
		}
		while (length >= GCM_BLOCK_SIZE) {
			CRYPT(ctx, dst, ctx->ctr.b, GCM_BLOCK_SIZE);
			memxor(dst, src, GCM_BLOCK_SIZE);
			INC32(ctx->ctr);
			src += GCM_BLOCK_SIZE;
			dst += GCM_BLOCK_SIZE;
			length -= GCM_BLOCK_SIZE;
		}
	} else {
		uint8_t buffer[4*GCM_BLOCK_SIZE] ATTRIBUTE(aligned(16));
		if (length >= 4*(4*GCM_BLOCK_SIZE)) {
			union {
				union crypto_block16 ctr[4];
				uint8_t bytes[4*GCM_BLOCK_SIZE];
			} ctr;
			gcm_extend_ctr(ctr.ctr, &ctx->ctr);
			/* Take advantage of native code that may be able to handle 4 blocks at once */
			while (length >= 4*GCM_BLOCK_SIZE) {
				CRYPT(ctx, buffer, ctr.bytes, 4*GCM_BLOCK_SIZE);	/* encrypt 4 blocks */
				memxor3(dst, src, buffer, 4*GCM_BLOCK_SIZE);
				ADD32(ctr.ctr[0], 4); ADD32(ctr.ctr[1], 4);
				ADD32(ctr.ctr[2], 4); ADD32(ctr.ctr[3], 4);
				src += 4*GCM_BLOCK_SIZE;
				dst += 4*GCM_BLOCK_SIZE;
				length -= 4*GCM_BLOCK_SIZE;
			}
			/* store first unused counter */
			memcpy(ctx->ctr.b, ctr.ctr[0].b, GCM_BLOCK_SIZE);
		}
		while (length >= GCM_BLOCK_SIZE) {
			CRYPT(ctx, buffer, ctx->ctr.b, GCM_BLOCK_SIZE);
			memxor3 (dst, src, buffer, GCM_BLOCK_SIZE);
			INC32 (ctx->ctr);
			src += GCM_BLOCK_SIZE;
			dst += GCM_BLOCK_SIZE;
			length -= GCM_BLOCK_SIZE;
		}
	}
	if (length > 0) {
		/* A final partial block */
		uint8_t buffer[GCM_BLOCK_SIZE] ATTRIBUTE(aligned(16));
		CRYPT(ctx, buffer, ctx->ctr.b, GCM_BLOCK_SIZE);
		memxor3 (dst, src, buffer, length);
		INC32 (ctx->ctr);
	}
}

void
gcm_encrypt(struct gcm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	/* ensure we don't process more data after the final input block */
	assert (ctx->data_size % GCM_BLOCK_SIZE == 0 || ctx->data_size == 0);
	gcm_pad(ctx);

	gcm_crypt(ctx, dst, src, length);
	gcm_hash(ctx->hash, dst, length);

	ctx->data_size += length;
}

void
gcm_decrypt(struct gcm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	/* ensure we don't process more data after the final input block */
	assert (ctx->data_size % GCM_BLOCK_SIZE == 0 || ctx->data_size == 0);
	gcm_pad(ctx);

	gcm_hash(ctx->hash, src, length);
	gcm_crypt(ctx, dst, src, length);

	ctx->data_size += length;
}

int
gcm_encrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length) {
	struct gcm_ctx ctx;
	assert (cipher_meta->block_size == GCM_BLOCK_SIZE);
	gcm_init(&ctx, cipher_meta);
	gcm_set_key(&ctx, key, key_length, nonce, nonce_length, digest_length, 0, 0);

	gcm_update(&ctx, aad, aad_length);
	gcm_encrypt(&ctx, dst, src, msg_length);
	gcm_digest(&ctx, dst + msg_length);

	gcm_free(&ctx);
	return 1;
}

int
gcm_decrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length) {
	struct gcm_ctx ctx;
	uint8_t tag[GCM_BLOCK_SIZE];
	assert (cipher_meta->block_size == GCM_BLOCK_SIZE);
	gcm_init(&ctx, cipher_meta);
	gcm_set_key(&ctx, key, key_length, nonce, nonce_length, digest_length, 0, 0);

	gcm_update(&ctx, aad, aad_length);
	gcm_decrypt(&ctx, dst, src, msg_length);
	gcm_digest(&ctx, tag);

	gcm_free(&ctx);
	return memeql_sec(tag, src + msg_length, digest_length);
}


unsigned ATTRIBUTE(pure)
gcm_get_block_size(const struct gcm_ctx *ctx) {
	return GCM_BLOCK_SIZE;
}

unsigned ATTRIBUTE(pure)
gcm_get_key_size(const struct gcm_ctx *ctx) {
	return ctx->key_size;
}

unsigned ATTRIBUTE(pure)
gcm_get_nonce_size(const struct gcm_ctx *ctx) {
	return GCM_IV_SIZE;
}

unsigned ATTRIBUTE(pure)
gcm_get_digest_size(const struct gcm_ctx *ctx) {
	return ctx->digest_size;
}

