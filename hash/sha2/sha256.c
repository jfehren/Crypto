/*
 * sha256.c
 *
 *  Created on: Apr 24, 2019, 3:25:07 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha256-internal.h"
#include "memwrite.h"
#include "macros.h"
#include <assert.h>

#include "sha256-table.h"

void
sha224_init(struct sha224_ctx *ctx) {
	ctx->count = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA224_H, sizeof(SHA224_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

void
sha256_init(struct sha256_ctx *ctx) {
	ctx->count = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA256_H, sizeof(SHA256_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

void
sha256_update(struct sha256_ctx *ctx, const uint8_t *src, size_t length) {
	unsigned left;
	if (ctx->index != 0) {
		/* fill partial block first */
		left = SHA256_BLOCK_SIZE - ctx->index;
		if (length < left) {
			/* Not enough to fill partial block */
			memcpy(ctx->block + ctx->index, src, length);
			ctx->index += length;
			return;
		} else {
			memcpy(ctx->block + ctx->index, src, left);
			_sha256_compress(ctx->state, ctx->block, SHA256_BLOCK_SIZE, SHA256_K);
			++ctx->count;

			src += left;
			length -= left;
		}
	}
	/* Process all full blocks of input remaining */
	left = length % SHA256_BLOCK_SIZE;
	length -= left;
	if (length >= SHA256_BLOCK_SIZE) {
		_sha256_compress(ctx->state, src, length, SHA256_K);
		src += length;
		ctx->count += length / SHA256_BLOCK_SIZE;
	}
	/* save final partial (possibly empty) input block */
	memcpy(ctx->block, src, left);
	ctx->index = left;
}

static inline void
_sha256_digest(struct sha256_ctx *ctx, uint8_t *dst, size_t length) {
	/* Pad blocks */
	assert (0 <= ctx->index && ctx->index < SHA256_BLOCK_SIZE);
	uint64_t bits;
	unsigned i = ctx->index;
	/* There are 64*8 = 512 = 2^9 bits in one block */
	bits = (ctx->count << 9) | (ctx->index << 3);

	/* Set first byte of padding to 0x80 */
	ctx->block[i++] = 0x80;
	if (i > SHA256_BLOCK_SIZE - 8) {	/* Need 8 bytes for the length tag */
		/* No room in this block for the length tag. Zero-pad and process it */
		memset(ctx->block + i, 0, SHA256_BLOCK_SIZE - i);
		_sha256_compress(ctx->state, ctx->block, SHA256_BLOCK_SIZE, SHA256_K);
		i = 0;
	}
	/* Zero-pad such that we are 8 bytes short of a full block */
	memset(ctx->block + i, 0, SHA256_BLOCK_SIZE - 8 - i);

	/* Append bit count */
	BE_WRITE_UINT64(ctx->block + (SHA256_BLOCK_SIZE - 8), bits);
	_sha256_compress(ctx->state, ctx->block, SHA256_BLOCK_SIZE, SHA256_K);

	memwrite_be32(dst, ctx->state, length);
}

void
sha224_digest(struct sha224_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA224_DIGEST_SIZE);
	_sha256_digest(ctx, digest, length);
	sha224_init(ctx);
}

void
sha256_digest(struct sha256_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA256_DIGEST_SIZE);
	_sha256_digest(ctx, digest, length);
	sha256_init(ctx);
}

