/*
 * md6-internal.c
 *
 *  Created on: Aug 27, 2019, 2:11:55 PM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <string.h>

#include "md6-internal.h"
#include "macros.h"

/* Define MD6_COMPRESS_SMALL to non-zero to use a limited
 * working array, rather than the 4169-word default array.
 * The limited array requires 112 reductions modulo 89 per
 * compression round, which may greatly impact performance,
 * however it requires only 712 extra bytes of memory,
 * rather than 33,352 bytes, so it saves a considerable
 * amount of space, making it preferable on systems with
 * limited memory resources. */
/*
#ifndef MD6_COMPRESS_SMALL
#define MD6_COMPRESS_SMALL 0
#endif
*/

/* Define CRYPTO_ALLOCA to non-zero to use a working
 * array that is dynamically allocated on the stack with
 * only as much memory as is needed, rather than the fixed
 * allocation of the default. The memory usage of this
 * version will almost always be lower than that of the
 * default with almost identical speed, however not all
 * compilers support the alloca function.
 *
 * MD6_COMPRESS_SMALL takes precedence over CRYPTO_ALLOCA
 */
/*
#ifndef CRYPTO_ALLOCA
#define CRYPTO_ALLOCA 0
#endif
*/
#ifndef MD6_COMPRESS_SMALL
#include "tmp-alloc.h"
#endif

/* Define MD6_DEBUG at compile time to enable debugging output */
#ifdef MD6_DEBUG
#include <stdio.h>
#include <inttypes.h>
#define DEBUG(i,args,...) do { \
	if (ctx->r < 10 || (i) < MD6_N || (i) >= MD6_N + (ctx->r-1)*MD6_C) { \
		printf("A[%4d] = %016" PRIx64 args "\n", (i), A(i), ##__VA_ARGS__); \
	} \
} while (0)
#else
#define DEBUG(i,args,...) do { } while (0)
#endif

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__

#define _reverse_little_endian(p, cnt) do { \
	int __be_cnt = (cnt); \
	uint64_t *__be_p = (p); \
	if (__be_cnt <= 0) { \
		break; \
	} \
	do { \
		*__be_p = BE_READ_UINT64(__be_p); \
		++__be_p; \
	} while (--__be_cnt); \
} while (0)

#else

#define _reverse_little_endian(p,cnt) do { } while (0)

#endif

/* 15 64-bit words */
static const uint64_t Q[MD6_Q] = {
	0x7311c2812425cfa0ULL,
	0x6432286434aac8e7ULL,
	0xb60450e9ef68b7c1ULL,
	0xe8fb23908d9f06f1ULL,
	0xdd2e76cba691e5bfULL,
	0x0cd0d63b2c30bc41ULL,
	0x1f8ccf6823058f8aULL,
	0x54e5ed5b88e3775dULL,
	0x4ad12aae0a6d6031ULL,
	0x3e7f16bb88222e0dULL,
	0x8af8671d3fb50c2cULL,
	0x995ad1178bd25c31ULL,
	0xc878c1dd04c4b633ULL,
	0x3b72066c7a1552acULL,
	0x0d6f3522631effcbULL
};

#define MAKE_NODEID(ell,i)	((((uint64_t)(ell)+1) << 56) | ((uint64_t)(i)))
#define MAKE_CONTROL_WORD(ctx,z,p)                 /*  4 bits reserved */\
	(((((uint64_t)(ctx->r)) & 0xFFF) << 48) |      /* 12 bits */ \
	 ((((uint64_t)(ctx->L)) & 0xFF) << 40) |       /*  8 bits */ \
	 ((((uint64_t)(z)) & 0xF) << 36) |             /*  4 bits */ \
	 ((((uint64_t)(p)) & 0xFFFF) << 20) |          /* 16 bits */ \
	 ((((uint64_t)(ctx->keylen)) & 0xFFF) << 12) | /*  8 bits */ \
	 (((uint64_t)(ctx->d)) & 0xFFF))               /* 12 bits */

#define MD6_Q_OFF	(0)
#define MD6_K_OFF	(MD6_Q+MD6_Q_OFF)
#define MD6_U_OFF	(MD6_K+MD6_K_OFF)
#define MD6_V_OFF	(MD6_U+MD6_U_OFF)
#define MD6_B_OFF	(MD6_V+MD6_V_OFF)

#define  t0		17
#define  t1		18
#define  t2		21
#define  t3		31
#define  t4		67
#define  t5		89

#define RL00 loop_body(10,11, 0)
#define RL01 loop_body( 5,24, 1)
#define RL02 loop_body(13, 9, 2)
#define RL03 loop_body(10,16, 3)
#define RL04 loop_body(11,15, 4)
#define RL05 loop_body(12, 9, 5)
#define RL06 loop_body( 2,27, 6)
#define RL07 loop_body( 7,15, 7)
#define RL08 loop_body(14, 6, 8)
#define RL09 loop_body(15, 2, 9)
#define RL10 loop_body( 7,29,10)
#define RL11 loop_body(13, 8,11)
#define RL12 loop_body(11,15,12)
#define RL13 loop_body( 7, 5,13)
#define RL14 loop_body( 6,31,14)
#define RL15 loop_body(12, 9,15)

#define S0		0x0123456789abcdefULL
#define Smask	0x7311c2812425cfa0ULL

static void
_md6_compress_block(struct md6_ctx *ctx, uint64_t *C, int ell, int z) {
	unsigned p;
	int i, j;
	uint64_t x, S;
#ifdef MD6_COMPRESS_SMALL
	uint64_t N[MD6_N];
#else
	TMP_DECL(N, uint64_t);
	TMP_ALLOC(N, MD6_N + MD6_C*ctx->r);
#endif
	p = (MD6_BLOCK_SIZE - ctx->bytes[ell])*8;	/* Number of padding bits */

	memcpy(N + MD6_Q_OFF, Q, MD6_Q*8);	/* copy Q constants */
	memcpy(N + MD6_K_OFF, ctx->K, MD6_K*8);	/* copy key words */
	N[MD6_U_OFF] = MAKE_NODEID(ell, ctx->i_level[ell]);			/* Node ID */
	N[MD6_V_OFF] = MAKE_CONTROL_WORD(ctx, z, p);	/* Control Word */
	memcpy(N + MD6_B_OFF, ctx->B[ell], MD6_B*8);	/* Copy block to be compressed */

#ifdef MD6_COMPRESS_SMALL
#define A(idx)	N[(idx)%MD6_N]
#else
#define A(idx)	N[idx]
#endif

#ifdef MD6_DEBUG
	for (i = 0; i < MD6_Q; i++) {
		DEBUG(i, " Q[%2d]", i);
	}
	for (; i < MD6_Q+MD6_K; i++) {
		DEBUG(i, " key K[%2d]", i - MD6_Q);
	}
	DEBUG(i, " nodeID U = (ell,i) = (%d,%" PRIu64 ")", ell+1, ctx->i_level[ell]); i++;
	DEBUG(i, " control word V = (r,L,z,p,keylen,d) = (%d,%d,%d,%d,%d,%d)",
			ctx->r, ctx->L, z, p, ctx->keylen, ctx->d); i++;
	if (ell == ctx->L && ctx->i_level[ell] == 0) {
		for (; i < MD6_N-MD6_B+MD6_C; i++) {
			DEBUG(i, " data B[%2d] IV", i - (MD6_N-MD6_B));
		}
	}
	if (ell == 0) {
		for (j = 0; i < MD6_N-MD6_B+(ctx->bytes[ell]+7)/8; i++, j++) {
			DEBUG(i, " data B[%2d] input word %d", i - (MD6_N-MD6_B), j);
		}
	} else {
		for (j = 0; i < MD6_N-MD6_B+(ctx->bytes[ell]+7)/8; i++, j++) {
			DEBUG(i, " data B[%2d] chaining from (%d,%" PRIu64 ")",
					i - (MD6_N-MD6_B), ell,
					((ctx->i_level[ell-1] - 1) & (-4)) + (j/16));
		}
	}
	for (; i < MD6_N; i++) {
		DEBUG(i, " data B[%2d] padding", i - (MD6_N-MD6_B));
	}
#endif

#define loop_body(rs,ls,step) \
	x = S; \
	x ^=  A(i+step-t5); \
	x ^=  A(i+step-t0); \
	x ^= (A(i+step-t1) & A(i+step-t2)); \
	x ^= (A(i+step-t3) & A(i+step-t4)); \
	x ^= (x >> rs); \
	A(i+step) = x ^ (x << ls); \
	DEBUG(i+step, "");

	/* Main Compression Loop */
	S = S0;
	i = MD6_N;
	for (j = 0; j < ctx->r; j++) {
#ifndef MD6_DEBUG
		/* Unroll loop c=16 times (One "round" of computation)
		 * Shift amounts are embedded in macros RLnn. */
		RL00 RL01 RL02 RL03 RL04 RL05 RL06 RL07
		RL08 RL09 RL10 RL11 RL12 RL13 RL14 RL15
#else
		/* RL00 */
		x = S;
		x ^=  A(i+ 0-t5);
		x ^=  A(i+ 0-t0);
		x ^= (A(i+ 0-t1) & A(i+ 0-t2));
		x ^= (A(i+ 0-t3) & A(i+ 0-t4));
		x ^= (x >> 10);
		x ^= (x << 11);
		A(i+ 0) = x;
		DEBUG(i+ 0, "");
		/* RL01 */
		x = S;
		x ^=  A(i+ 1-t5);
		x ^=  A(i+ 1-t0);
		x ^= (A(i+ 1-t1) & A(i+ 1-t2));
		x ^= (A(i+ 1-t3) & A(i+ 1-t4));
		x ^= (x >>  5);
		x ^= (x << 24);
		A(i+ 1) = x;
		DEBUG(i+ 1, "");
		/* RL02 */
		x = S;
		x ^=  A(i+ 2-t5);
		x ^=  A(i+ 2-t0);
		x ^= (A(i+ 2-t1) & A(i+ 2-t2));
		x ^= (A(i+ 2-t3) & A(i+ 2-t4));
		x ^= (x >> 13);
		x ^= (x <<  9);
		A(i+ 2) = x;
		DEBUG(i+ 2, "");
		/* RL03 */
		x = S;
		x ^=  A(i+ 3-t5);
		x ^=  A(i+ 3-t0);
		x ^= (A(i+ 3-t1) & A(i+ 3-t2));
		x ^= (A(i+ 3-t3) & A(i+ 3-t4));
		x ^= (x >> 10);
		x ^= (x << 16);
		A(i+ 3) = x;
		DEBUG(i+ 3, "");
		/* RL04 */
		x = S;
		x ^=  A(i+ 4-t5);
		x ^=  A(i+ 4-t0);
		x ^= (A(i+ 4-t1) & A(i+ 4-t2));
		x ^= (A(i+ 4-t3) & A(i+ 4-t4));
		x ^= (x >> 11);
		x ^= (x << 15);
		A(i+ 4) = x;
		DEBUG(i+ 4, "");
		/* RL05 */
		x = S;
		x ^=  A(i+ 5-t5);
		x ^=  A(i+ 5-t0);
		x ^= (A(i+ 5-t1) & A(i+ 5-t2));
		x ^= (A(i+ 5-t3) & A(i+ 5-t4));
		x ^= (x >> 12);
		x ^= (x <<  9);
		A(i+ 5) = x;
		DEBUG(i+ 5, "");
		/* RL06 */
		x = S;
		x ^=  A(i+ 6-t5);
		x ^=  A(i+ 6-t0);
		x ^= (A(i+ 6-t1) & A(i+ 6-t2));
		x ^= (A(i+ 6-t3) & A(i+ 6-t4));
		x ^= (x >>  2);
		x ^= (x << 27);
		A(i+ 6) = x;
		DEBUG(i+ 6, "");
		/* RL07 */
		x = S;
		x ^=  A(i+ 7-t5);
		x ^=  A(i+ 7-t0);
		x ^= (A(i+ 7-t1) & A(i+ 7-t2));
		x ^= (A(i+ 7-t3) & A(i+ 7-t4));
		x ^= (x >>  7);
		x ^= (x << 15);
		A(i+ 7) = x;
		DEBUG(i+ 7, "");
		/* RL08 */
		x = S;
		x ^=  A(i+ 8-t5);
		x ^=  A(i+ 8-t0);
		x ^= (A(i+ 8-t1) & A(i+ 8-t2));
		x ^= (A(i+ 8-t3) & A(i+ 8-t4));
		x ^= (x >> 14);
		x ^= (x <<  6);
		A(i+ 8) = x;
		DEBUG(i+ 8, "");
		/* RL09 */
		x = S;
		x ^=  A(i+ 9-t5);
		x ^=  A(i+ 9-t0);
		x ^= (A(i+ 9-t1) & A(i+ 9-t2));
		x ^= (A(i+ 9-t3) & A(i+ 9-t4));
		x ^= (x >> 15);
		x ^= (x <<  2);
		A(i+ 9) = x;
		DEBUG(i+ 9, "");
		/* RL10 */
		x = S;
		x ^=  A(i+10-t5);
		x ^=  A(i+10-t0);
		x ^= (A(i+10-t1) & A(i+10-t2));
		x ^= (A(i+10-t3) & A(i+10-t4));
		x ^= (x >>  7);
		x ^= (x << 29);
		A(i+10) = x;
		DEBUG(i+10, "");
		/* RL11 */
		x = S;
		x ^=  A(i+11-t5);
		x ^=  A(i+11-t0);
		x ^= (A(i+11-t1) & A(i+11-t2));
		x ^= (A(i+11-t3) & A(i+11-t4));
		x ^= (x >> 13);
		x ^= (x <<  8);
		A(i+11) = x;
		DEBUG(i+11, "");
		/* RL12 */
		x = S;
		x ^=  A(i+12-t5);
		x ^=  A(i+12-t0);
		x ^= (A(i+12-t1) & A(i+12-t2));
		x ^= (A(i+12-t3) & A(i+12-t4));
		x ^= (x >> 11);
		x ^= (x << 15);
		A(i+12) = x;
		DEBUG(i+12, "");
		/* RL13 */
		x = S;
		x ^=  A(i+13-t5);
		x ^=  A(i+13-t0);
		x ^= (A(i+13-t1) & A(i+13-t2));
		x ^= (A(i+13-t3) & A(i+13-t4));
		x ^= (x >>  7);
		x ^= (x <<  5);
		A(i+13) = x;
		DEBUG(i+13, "");
		/* RL14 */
		x = S;
		x ^=  A(i+14-t5);
		x ^=  A(i+14-t0);
		x ^= (A(i+14-t1) & A(i+14-t2));
		x ^= (A(i+14-t3) & A(i+14-t4));
		x ^= (x >>  6);
		x ^= (x << 31);
		A(i+14) = x;
		DEBUG(i+14, "");
		/* RL15 */
		x = S;
		x ^=  A(i+15-t5);
		x ^=  A(i+15-t0);
		x ^= (A(i+15-t1) & A(i+15-t2));
		x ^= (A(i+15-t3) & A(i+15-t4));
		x ^= (x >> 12);
		x ^= (x <<  9);
		A(i+15) = x;
		DEBUG(i+15, "");

		if (j == ctx->r-2) {
			printf("\n");
		}
#endif
		/* Advance round constant S to the next round constant. */
		S = (S << 1) ^ (S >> 63) ^ (S & Smask);
		i += 16;
	}
#undef loop_body
#undef A

#ifdef MD6_DEBUG
	printf("\n");
#endif

	assert (i == MD6_N + ctx->r*MD6_C);		/* Sanity check */

	/* Output to C */
#ifdef MD6_COMPRESS_SMALL
	i = (i - MD6_C) % MD6_N;
	if (i + MD6_C <= MD6_N) {
		memcpy(C, N + i, MD6_CHUNK_SIZE);
	} else {
		memcpy(C, N + i, 8*(MD6_N - i));
		memcpy(C + MD6_N - i, N, 8*(MD6_C - (MD6_N - i)));
	}
#else
	memcpy(C, N + i - MD6_C, MD6_CHUNK_SIZE);
	TMP_FREE(N);
#endif

	ctx->bytes[ell] = 0;	/* Clear bytes set on this level */
	ctx->i_level[ell]++;	/* Set the index of the next block on this level */
}


void
_md6_process(struct md6_ctx *ctx) {
	int ell, next;
	uint64_t *C;
	/* Not final pass - more input may come later */
	ell = 0;
	while (ctx->bytes[ell] == MD6_BLOCK_SIZE) {
		/* Save result to next level */
		assert (ell <= ctx->top && ctx->top <= ctx->L);		/* Sanity check */
		next = min(ell + 1, ctx->L);
		if (next == ctx->L) {
			/* Next level is sequential */
			if (ctx->i_level[next] == 0 && ctx->bytes[next] == 0) {
				/* start new sequential node */
				memset(ctx->B[next], 0, MD6_CHUNK_SIZE);
				ctx->bytes[next] = MD6_CHUNK_SIZE;
				C = ctx->B[next] + MD6_C;
			} else if (next != ell) {
				/* current level is below sequential node */
				C = ctx->B[next] + (ctx->bytes[next]/8);
			} else {
				/* current level is sequential node */
				C = ctx->B[next];
			}
		} else {
			/* next level is parallel node */
			C = ctx->B[next] + (ctx->bytes[next]/8);
		}
		/* Compress block, saving it to the next level */
		_md6_compress_block(ctx, C, ell, 0);
		ctx->bytes[next] += MD6_CHUNK_SIZE;
		if (next > ctx->top) {
			ctx->top = next;
		}
		/* process next level, if necessary */
		ell = next;
	}
}

void
_md6_final(struct md6_ctx *ctx, int ell, uint8_t *digest, size_t length) {
	unsigned bytes, bits, i;
	int next;
	uint64_t *C;
	/* Final pass - no more input coming */
	assert (ctx->top <= ctx->L);	/* sanity check */
	while (ell < ctx->top) {
		/* Save result to next level */
		next = min(ell + 1, ctx->L);
		if (next == ctx->L) {
			/* Next level is sequential */
			if (ctx->i_level[next] == 0 && ctx->bytes[next] == 0) {
				/* start new sequential node */
				memset(ctx->B[next], 0, MD6_CHUNK_SIZE);
				ctx->bytes[next] = MD6_CHUNK_SIZE;
				C = ctx->B[next] + MD6_C;
			} else if (next != ell) {
				/* current level is below sequential node */
				C = ctx->B[next] + (ctx->bytes[next]/8);
			} else {
				/* current level is sequential node */
				C = ctx->B[next];
			}
		} else {
			/* next level is parallel node */
			C = ctx->B[next] + (ctx->bytes[next]/8);
		}
		/* sanity check */
		assert (next < ctx->L || !(ctx->i_level[next] == 0 && ctx->bytes[next] == 0));
		if (ctx->bytes[ell] < MD6_BLOCK_SIZE) {
			/* Zero-pad blocks on final pass */
			bytes = ctx->bytes[ell] % 8;
			if (bytes != 0) {
				/* fix partial word - should only happen on level 0 */
				assert (ell == 0);	/* sanity check */
				bytes = 8 - bytes;
				ctx->B[ell][ctx->bytes[ell] / 8] <<= 8*bytes;
			}
			memset(((uint8_t*)ctx->B[ell]) + ctx->bytes[ell] + bytes,
					0, MD6_BLOCK_SIZE - ctx->bytes[ell] - bytes);
		}
		/* Compress block, saving it to the next level */
		_md6_compress_block(ctx, C, ell, 0);
		ctx->bytes[next] += MD6_CHUNK_SIZE;
		/* process next level */
		ell = next;
	}
	/* At the top-most node */
	assert (ell == ctx->top);
	/* Ensure that the final node had not been processed incorrectly */
	assert (!(ell == ctx->L && ctx->bytes[ell] == MD6_CHUNK_SIZE && ctx->i_level[ell] > 0) ||
			!(ell != ctx->L && ell > 0 && ctx->bytes[ell] == MD6_CHUNK_SIZE));
	/* Final compression and hash trimming */
	if (ctx->bytes[ell] < MD6_BLOCK_SIZE) {
		/* Zero-pad blocks on final pass */
		bytes = ctx->bytes[ell] % 8;
		if (bytes != 0) {
			/* fix partial word - should only happen on level 0 */
			assert (ell == 0);	/* sanity check */
			bytes = 8 - bytes;
			ctx->B[ell][ctx->bytes[ell] / 8] <<= 8*bytes;
		}
		memset(((uint8_t*)ctx->B[ell]) + ctx->bytes[ell] + bytes,
				0, MD6_BLOCK_SIZE - ctx->bytes[ell] - bytes);
	}
	_md6_compress_block(ctx, ctx->B[ell], ell, 1);
	bytes = (ctx->d+7)/8;	/* total number of bytes (last byte may be partial) */
	if (length < bytes) {
		bytes = length;
		bits = 0;
	} else {
		bits = ctx->d % 8;		/* bits in partial byte (0 for no partial byte) */
		memset(digest + bytes, 0, length - bytes);
	}
	/* Write the last ceil(d/8) bytes of the final chunk to the output */
	_reverse_little_endian(ctx->B[ell] + MD6_C-(bytes+7)/8, MD6_C);
	memcpy(digest, ((uint8_t*)ctx->B[ell]) + MD6_CHUNK_SIZE-bytes, bytes);
	if (bits) {	/* only happens in non-standard cases - very unlikely but still needed */
		/* Shift result left by (8-bits) bit positions, per byte */
		--bytes;
		for (i = 0; i < bytes; i++) {
			digest[i] = (digest[i] << (8-bits)) | (digest[i+1] >> bits);
		}
		digest[bytes] = digest[bytes] << (8-bits);
	}
	/* digest now holds min(length*8, ctx->d) hash bits */
}

