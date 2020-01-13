/*
 * md4.c
 *
 *  Created on: Aug 26, 2019, 10:56:59 AM
 *      Author: Joshua Fehrenbach
 */

#include "md4.h"
#include "memwrite.h"
#include "macros.h"
#include <assert.h>
#include <string.h>

/* A block, treated as a sequence of 32-bit words. */
#define MD4_DATA_LENGTH 16

/* MD4 functions */
/* #define F(x,y,z) ( ((x) & (y)) | (~(x) & (z)) ) */
#define F(x,y,z)	( (z) ^ ((x) & ((y) ^ (z))) )
/* #define G(x,y,z)	( ((x) & (y)) | ((x) & (z)) | ((y) & (z)) ) */	/* Majority function */
#define G(x,y,z)	( ((x) | (y)) & ((z) | ((x) & (y))) )
#define H(x,y,z)	( (x) ^ (y) ^ (z) )

#define ROUND(f, w, x, y, z, data, s) ( w += f(x, y, z) + data, w = ROTL32(w, s))

/* Perform MD4 transformation on one full block of 16 32-bit words */
static void
md4_transform(uint32_t *digest, const uint32_t *data) {
	uint32_t a, b, c, d;
	a = digest[0];
	b = digest[1];
	c = digest[2];
	d = digest[3];

	ROUND(F, a, b, c, d, data[ 0],  3);
	ROUND(F, d, a, b, c, data[ 1],  7);
	ROUND(F, c, d, a, b, data[ 2], 11);
	ROUND(F, b, c, d, a, data[ 3], 19);
	ROUND(F, a, b, c, d, data[ 4],  3);
	ROUND(F, d, a, b, c, data[ 5],  7);
	ROUND(F, c, d, a, b, data[ 6], 11);
	ROUND(F, b, c, d, a, data[ 7], 19);
	ROUND(F, a, b, c, d, data[ 8],  3);
	ROUND(F, d, a, b, c, data[ 9],  7);
	ROUND(F, c, d, a, b, data[10], 11);
	ROUND(F, b, c, d, a, data[11], 19);
	ROUND(F, a, b, c, d, data[12],  3);
	ROUND(F, d, a, b, c, data[13],  7);
	ROUND(F, c, d, a, b, data[14], 11);
	ROUND(F, b, c, d, a, data[15], 19);

	ROUND(G, a, b, c, d, data[ 0] + 0x5a827999,  3);
	ROUND(G, d, a, b, c, data[ 4] + 0x5a827999,  5);
	ROUND(G, c, d, a, b, data[ 8] + 0x5a827999,  9);
	ROUND(G, b, c, d, a, data[12] + 0x5a827999, 13);
	ROUND(G, a, b, c, d, data[ 1] + 0x5a827999,  3);
	ROUND(G, d, a, b, c, data[ 5] + 0x5a827999,  5);
	ROUND(G, c, d, a, b, data[ 9] + 0x5a827999,  9);
	ROUND(G, b, c, d, a, data[13] + 0x5a827999, 13);
	ROUND(G, a, b, c, d, data[ 2] + 0x5a827999,  3);
	ROUND(G, d, a, b, c, data[ 6] + 0x5a827999,  5);
	ROUND(G, c, d, a, b, data[10] + 0x5a827999,  9);
	ROUND(G, b, c, d, a, data[14] + 0x5a827999, 13);
	ROUND(G, a, b, c, d, data[ 3] + 0x5a827999,  3);
	ROUND(G, d, a, b, c, data[ 7] + 0x5a827999,  5);
	ROUND(G, c, d, a, b, data[11] + 0x5a827999,  9);
	ROUND(G, b, c, d, a, data[15] + 0x5a827999, 13);

	ROUND(H, a, b, c, d, data[ 0] + 0x6ed9eba1,  3);
	ROUND(H, d, a, b, c, data[ 8] + 0x6ed9eba1,  9);
	ROUND(H, c, d, a, b, data[ 4] + 0x6ed9eba1, 11);
	ROUND(H, b, c, d, a, data[12] + 0x6ed9eba1, 15);
	ROUND(H, a, b, c, d, data[ 2] + 0x6ed9eba1,  3);
	ROUND(H, d, a, b, c, data[10] + 0x6ed9eba1,  9);
	ROUND(H, c, d, a, b, data[ 6] + 0x6ed9eba1, 11);
	ROUND(H, b, c, d, a, data[14] + 0x6ed9eba1, 15);
	ROUND(H, a, b, c, d, data[ 1] + 0x6ed9eba1,  3);
	ROUND(H, d, a, b, c, data[ 9] + 0x6ed9eba1,  9);
	ROUND(H, c, d, a, b, data[ 5] + 0x6ed9eba1, 11);
	ROUND(H, b, c, d, a, data[13] + 0x6ed9eba1, 15);
	ROUND(H, a, b, c, d, data[ 3] + 0x6ed9eba1,  3);
	ROUND(H, d, a, b, c, data[11] + 0x6ed9eba1,  9);
	ROUND(H, c, d, a, b, data[ 7] + 0x6ed9eba1, 11);
	ROUND(H, b, c, d, a, data[15] + 0x6ed9eba1, 15);

	digest[0] += a;
	digest[1] += b;
	digest[2] += c;
	digest[3] += d;
}

static inline void
md4_compress(struct md4_ctx *ctx, const uint8_t *block) {
	uint32_t data[MD4_DATA_LENGTH];
	for (unsigned i = 0; i < 16; i++) {
		data[i] = LE_READ_UINT32(block + i*4);
	}
	md4_transform(ctx->state, data);
}


void
md4_init(struct md4_ctx *ctx) {
	/* Same constants as for md5. */
	const uint32_t iv[_MD4_DIGEST_LENGTH] = {
		0x67452301,
		0xefcdab89,
		0x98badcfe,
		0x10325476
	};
	memcpy(ctx->state, iv, sizeof(ctx->state));
	memset(ctx->block, 0, sizeof(ctx->block));

	ctx->count = 0;
	ctx->index = 0;
}

void
md4_update(struct md4_ctx *ctx, const uint8_t *data, size_t length) {
	if (ctx->index) {
		/* Fill partial block */
		unsigned left = sizeof(ctx->block) - ctx->index;
		if (length < left) {
			memcpy(ctx->block + ctx->index, data, length);
			ctx->index += length;
			return;		/* Cannot fill partial block, so return here */
		} else {
			memcpy(ctx->block + ctx->index, data, left);
			md4_compress(ctx, ctx->block);
			ctx->count++;

			data += left;
			length -= left;
		}
	}
	/* Process individual block */
	while (length >= sizeof(ctx->block)) {
		md4_compress(ctx, data);
		ctx->count++;

		data += sizeof(ctx->block);
		length -= sizeof(ctx->block);
	}
	/* Copy any leftover data into internal buffer */
	memcpy(ctx->block, data, length);
	ctx->index = length;
}

void
md4_digest(struct md4_ctx *ctx, uint8_t *digest, size_t length) {
	uint64_t bit_count;
	uint32_t data[MD4_DATA_LENGTH];
	unsigned i;

	assert(length <= MD4_DIGEST_SIZE);

	/* Apply padding */
	i = ctx->index;
	ctx->block[i++] = 0x80;
	if (i > (sizeof(ctx->block) - 8)) {
		/* No room for length in this block. Process it and pad with another one */					\
		memset(ctx->block + i, 0, sizeof(ctx->block) - i);

		md4_compress(ctx, ctx->block);
		i = 0;
	}
	memset(ctx->block + i, 0, sizeof(ctx->block) - 8 - i);

	for (i = 0; i < MD4_DATA_LENGTH - 2; i++) {
		data[i] = LE_READ_UINT32(ctx->block + 4*i);
	}

	/* There are 512 = 2^9 bits in one block
	 * Little-endian order => Least significant word first */
	bit_count = (ctx->count << 9) | (ctx->index << 3);
	data[MD4_DATA_LENGTH-2] = bit_count;
	data[MD4_DATA_LENGTH-1] = bit_count >> 32;
	md4_transform(ctx->state, data);

	memwrite_le32(digest, ctx->state, length);
	md4_init(ctx);
}

