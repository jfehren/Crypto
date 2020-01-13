/*
 * sha512.c
 *
 *  Created on: Apr 28, 2019, 3:24:16 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha512-internal.h"
#include "memwrite.h"
#include "macros.h"
#include <assert.h>

#include "sha512-table.h"

void
sha384_init(struct sha384_ctx *ctx) {
	ctx->count_low = ctx->count_high = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA384_H, sizeof(SHA384_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

void
sha512_init(struct sha512_ctx *ctx) {
	ctx->count_low = ctx->count_high = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA512_H, sizeof(SHA512_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

void
sha512_224_init(struct sha512_224_ctx *ctx) {
	ctx->count_low = ctx->count_high = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA512_224_H, sizeof(SHA512_224_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

void
sha512_256_init(struct sha512_256_ctx *ctx) {
	ctx->count_low = ctx->count_high = 0;
	ctx->index = 0;
	memcpy(ctx->state, SHA512_256_H, sizeof(SHA512_256_H));
	memset(ctx->block, 0, sizeof(ctx->block));
}

#define INCR(ctx) do { \
	++(ctx)->count_low; \
	(ctx)->count_high += ((ctx)->count_low == 0); \
} while (0)
#define ADD(ctx,add) do { \
	(ctx)->count_low += (add); \
	(ctx)->count_high += ((ctx)->count_low < (add)); \
} while (0)

void
sha512_update(struct sha512_ctx *ctx, const uint8_t *src, size_t length) {
	unsigned left;
	if (ctx->index) {
		/* fill partial block first */
		left = SHA512_BLOCK_SIZE - ctx->index;
		if (length < left) {
			/* Not enough to fill partial block */
			memcpy(ctx->block + ctx->index, src, length);
			ctx->index += length;
			return;
		} else {
			memcpy(ctx->block + ctx->index, src, left);
			_sha512_compress(ctx->state, ctx->block, SHA512_BLOCK_SIZE, SHA512_K);
			INCR(ctx);	/* Increment number of blocks processed */

			src += left;
			length -= left;
		}
	}
	/* Process all full blocks of input remaining */
	left = length % SHA512_BLOCK_SIZE;
	length -= left;
	if (length >= SHA512_BLOCK_SIZE) {
		_sha512_compress(ctx->state, src, length, SHA512_K);
		src += length;
		ADD(ctx, length / SHA512_BLOCK_SIZE);
	}
	/* save final partial (possibly empty) input block */
	memcpy(ctx->block, src, left);
	ctx->index = left;
}

static inline void
_sha512_digest(struct sha512_ctx *ctx, uint8_t *dst, size_t length) {
	/* Pad blocks */
	assert (0 <= ctx->index && ctx->index < SHA512_BLOCK_SIZE);
	uint64_t high, low;
	unsigned i = ctx->index;
	/* There are 128*8 = 1024 = 2^10 bits in one block */
	high = (ctx->count_high << 10) | (ctx->count_low >> 54);
	low = (ctx->count_low << 10) | (ctx->index << 3);

	/* Set first byte of padding to 0x80 */
	ctx->block[i++] = 0x80;
	if (i > SHA512_BLOCK_SIZE - 16) {	/* Need 16 bytes for the length tag */
		/* No room in this block for the length tag. Zero-pad and process it */
		memset(ctx->block + i, 0, SHA512_BLOCK_SIZE - i);
		_sha512_compress(ctx->state, ctx->block, SHA512_BLOCK_SIZE, SHA512_K);
		i = 0;
	}
	/* Zero-pad such that we are 16 bytes short of a full block */
	memset(ctx->block + i, 0, SHA512_BLOCK_SIZE - 16 - i);

	/* Append bit count */
	BE_WRITE_UINT64(ctx->block + (SHA512_BLOCK_SIZE - 16), high);
	BE_WRITE_UINT64(ctx->block + (SHA512_BLOCK_SIZE -  8), low);
	_sha512_compress(ctx->state, ctx->block, SHA512_BLOCK_SIZE, SHA512_K);

	memwrite_be64(dst, ctx->state, length);
}

void
sha384_digest(struct sha384_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA384_DIGEST_SIZE);
	_sha512_digest(ctx, digest, length);
	sha384_init(ctx);
}

void
sha512_digest(struct sha512_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA512_DIGEST_SIZE);
	_sha512_digest(ctx, digest, length);
	sha512_init(ctx);
}

void
sha512_224_digest(struct sha512_224_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA512_224_DIGEST_SIZE);
	_sha512_digest(ctx, digest, length);
	sha512_224_init(ctx);
}

void
sha512_256_digest(struct sha512_256_ctx *ctx, uint8_t *digest, size_t length) {
	assert (0 <= length && length <= SHA512_256_DIGEST_SIZE);
	_sha512_digest(ctx, digest, length);
	sha512_256_init(ctx);
}

