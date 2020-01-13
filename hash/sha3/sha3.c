/*
 * sha3.c
 *
 *  Created on: Aug 19, 2019, 1:39:24 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha3-internal.h"
#include "memwrite.h"
#include "macros.h"
#include <stddef.h>
#include <string.h>
#include <assert.h>

void
sha3_224_init(struct sha3_224_ctx *ctx) {
	memset(ctx, 0, sizeof(struct sha3_224_ctx));
}

void
sha3_256_init(struct sha3_256_ctx *ctx) {
	memset(ctx, 0, sizeof(struct sha3_256_ctx));
}

void
sha3_384_init(struct sha3_384_ctx *ctx) {
	memset(ctx, 0, sizeof(struct sha3_384_ctx));
}

void
sha3_512_init(struct sha3_512_ctx *ctx) {
	memset(ctx, 0, sizeof(struct sha3_512_ctx));
}


#if WORDS_BIGENDIAN
#define ABSORB(A,data,length) do { \
	assert ( (length & 7) == 0); \
	unsigned __absorb_len = length; \
	const uint8_t *__absorb_data = data; \
	for (uint64_t *p = A; __absorb_len > 0; p++, __absorb_len -= 8, __absorb_data += 8) { \
		*p = LE_READ_UINT64(__absorb_data); \
	} \
	sha3_permute(A); \
} while (0)
#else
#include "memxor.h"
#define ABSORB(A,data,length) do { \
	assert ( (length & 7) == 0); \
	memxor(A, data, length); \
	sha3_permute(A); \
} while (0)
#endif

#define UPDATE(A,block,size,pos,data,length) do { \
	if (pos > 0) { \
		unsigned left = size - pos; \
		if (length < left) { \
			memcpy(block + pos, data, length); \
			pos += length; \
		} else { \
			memcpy(block + pos, data, left); \
			data += left; \
			length -= left; \
			ABSORB(A, block, size); \
		} \
	} \
	FOR_BLOCKS_INPLACE(length, data, size) { \
		ABSORB(A, data, size); \
	} \
	pos = length; \
	memcpy(block, data, length); \
} while (0)

#define PAD(A,block,size,pos) do { \
	assert(pos < size); \
	/* pad message with 011000...0001 */ \
	block[pos++] = 6; \
	memset(block + pos, 0, size - pos); \
	block[size - 1] |= 0x80; \
	ABSORB(A, block, size); \
} while (0)


void
sha3_224_update(struct sha3_224_ctx *ctx, const uint8_t *data, size_t length) {
	UPDATE(ctx->state.a, ctx->block, SHA3_224_BLOCK_SIZE, ctx->index, data, length);
}

void
sha3_256_update(struct sha3_256_ctx *ctx, const uint8_t *data, size_t length) {
	UPDATE(ctx->state.a, ctx->block, SHA3_256_BLOCK_SIZE, ctx->index, data, length);
}

void
sha3_384_update(struct sha3_384_ctx *ctx, const uint8_t *data, size_t length) {
	UPDATE(ctx->state.a, ctx->block, SHA3_384_BLOCK_SIZE, ctx->index, data, length);
}

void
sha3_512_update(struct sha3_512_ctx *ctx, const uint8_t *data, size_t length) {
	UPDATE(ctx->state.a, ctx->block, SHA3_512_BLOCK_SIZE, ctx->index, data, length);
}


void
sha3_224_digest(struct sha3_224_ctx *ctx, uint8_t *digest, size_t length) {
	assert (length <= SHA3_224_DIGEST_SIZE);
	PAD(ctx->state.a, ctx->block, SHA3_224_BLOCK_SIZE, ctx->index);
	memwrite_le64(digest, ctx->state.a, length);
	sha3_224_init(ctx);
}

void
sha3_256_digest(struct sha3_256_ctx *ctx, uint8_t *digest, size_t length) {
	assert (length <= SHA3_256_DIGEST_SIZE);
	PAD(ctx->state.a, ctx->block, SHA3_256_BLOCK_SIZE, ctx->index);
	memwrite_le64(digest, ctx->state.a, length);
	sha3_256_init(ctx);
}

void
sha3_384_digest(struct sha3_384_ctx *ctx, uint8_t *digest, size_t length) {
	assert (length <= SHA3_384_DIGEST_SIZE);
	PAD(ctx->state.a, ctx->block, SHA3_384_BLOCK_SIZE, ctx->index);
	memwrite_le64(digest, ctx->state.a, length);
	sha3_384_init(ctx);
}

void
sha3_512_digest(struct sha3_512_ctx *ctx, uint8_t *digest, size_t length) {
	assert (length <= SHA3_512_DIGEST_SIZE);
	PAD(ctx->state.a, ctx->block, SHA3_512_BLOCK_SIZE, ctx->index);
	memwrite_le64(digest, ctx->state.a, length);
	sha3_512_init(ctx);
}

