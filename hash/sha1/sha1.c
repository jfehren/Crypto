/*
 * sha1.c
 *
 *  Created on: Apr 19, 2019, 2:28:19 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha1-internal.h"
#include "memwrite.h"
#include "macros.h"
#include <assert.h>

void
sha1_init(struct sha1_ctx *ctx) {
	static const struct sha1_ctx iv =  {
			0,	/* count */
			0,	/* index */
			{	/* state */
					/* SHA initial values */
					UINT32_C(0x67452301),
					UINT32_C(0xEFCDAB89),
					UINT32_C(0x98BADCFE),
					UINT32_C(0x10325476),
					UINT32_C(0xC3D2E1F0)
			},{	/* block */
					/* block starts empty, so set it to a zero vector */
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
					0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
			}
	};
	/* Initialize context with a call to memcpy */
	memcpy(ctx, &iv, sizeof(struct sha1_ctx));
}

void
sha1_update(struct sha1_ctx *ctx, const uint8_t *src, size_t length) {
	unsigned left;
	if (ctx->index != 0) {
		/* fill partial block first */
		left = SHA1_BLOCK_SIZE - ctx->index;
		if (length < left) {
			/* Not enough to fill partial block */
			memcpy(ctx->block + ctx->index, src, length);
			ctx->index += length;
			return;
		} else {
			memcpy(ctx->block + ctx->index, src, left);
			_sha1_compress(ctx->state, ctx->block, SHA1_BLOCK_SIZE);
			++ctx->count;

			src += left;
			length -= left;
		}
	}
	/* process all full blocks of input remaining */
	left = length % SHA1_BLOCK_SIZE;
	length -= left;
	if (length >= SHA1_BLOCK_SIZE) {
		_sha1_compress(ctx->state, src, length);
		src += length;
		ctx->count += length / SHA1_BLOCK_SIZE;
	}
	/* save final partial (possibly empty) input block */
	memcpy(ctx->block, src, left);
	ctx->index = left;
}

void
sha1_digest(struct sha1_ctx *ctx, uint8_t *dst, size_t length) {
	/* Pad blocks */
	assert (0 <= ctx->index && ctx->index < SHA1_BLOCK_SIZE);
	assert (length <= SHA1_DIGEST_SIZE);
	uint64_t bit_count;
	unsigned i = ctx->index;
	/* There are 64*8 = 512 = 2^9 bits in one block */
	bit_count = (ctx->count << 9) | (ctx->index << 3);

	/* Set first byte of padding to 0x80 */
	ctx->block[i++] = 0x80;
	if (i > SHA1_BLOCK_SIZE - 8) {	/* Need 8 bytes for the length tag */
		/* No room in this block for the length tag. Zero-pad and process it */
		memset(ctx->block + i, 0, SHA1_BLOCK_SIZE - i);
		_sha1_compress(ctx->state, ctx->block, SHA1_BLOCK_SIZE);
		i = 0;
	}
	/* Zero-pad such that we are 8 bytes short of a full block */
	memset(ctx->block + i, 0, SHA1_BLOCK_SIZE - 8 - i);

	/* Append bit count */
	BE_WRITE_UINT64(ctx->block + (SHA1_BLOCK_SIZE - 8), bit_count);
	_sha1_compress(ctx->state, ctx->block, SHA1_BLOCK_SIZE);

	memwrite_be32(dst, ctx->state, length);

	/* Reset ctx */
	sha1_init(ctx);
}
