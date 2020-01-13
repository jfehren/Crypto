/*
 * hmac.c
 *
 *  Created on: Sep 3, 2019, 2:33:02 PM
 *      Author: Joshua Fehrenbach
 */

#include "hmac.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
hmac_init(struct hmac_ctx *ctx, const struct crypto_hash_meta *hash_meta) {
	const unsigned context_size = hash_meta->context_size;
	const unsigned offset = (context_size % 16 == 0) ? 0
			: 16 - (context_size % 16);
	uint8_t *mem = aligned_alloc(16, 3*context_size + 3*offset);
	ctx->outer = mem;
	mem += context_size + offset;
	ctx->inner = mem;
	mem += context_size + offset;
	ctx->state = mem;
	ctx->digest_size = hash_meta->digest_size;
	ctx->block_size = hash_meta->block_size;
	ctx->context_size = context_size;
	ctx->initialize = hash_meta->initialize;
	ctx->update = hash_meta->update;
	ctx->finalize = hash_meta->finalize;
}

void
hmac_free(struct hmac_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->outer, 0, ctx->context_size);
	memset(ctx->inner, 0, ctx->context_size);
	memset(ctx->state, 0, ctx->context_size);
	/* Release memory */
	free(ctx->outer);
}

void
hmac_set_key(struct hmac_ctx *ctx, const uint8_t *key, size_t length) {
	uint8_t ko[144], ki[144];	/* 144 bytes is the current largest hash block length */
	unsigned i;
	(*ctx->initialize)(ctx->outer);
	(*ctx->initialize)(ctx->inner);
	if (length > ctx->block_size) {
		/* Hash key to reduce length */
		(*ctx->update)(ctx->outer, key, length);
		(*ctx->finalize)(ctx->outer, ko, ctx->digest_size);
		for (i = 0; i < ctx->digest_size; i++) {
			ki[i] = ko[i] ^ 0x36;
			ko[i] = ko[i] ^ 0x5C;
		}
		memset(ki + ctx->digest_size, 0x36, ctx->block_size - ctx->digest_size);
		memset(ko + ctx->digest_size, 0x5C, ctx->block_size - ctx->digest_size);
	} else {
		for (i = 0; i < length; i++) {
			ki[i] = key[i] ^ 0x36;
			ko[i] = key[i] ^ 0x5C;
		}
		memset(ki + length, 0x36, ctx->block_size - length);
		memset(ko + length, 0x5C, ctx->block_size - length);
	}
	/* Initialize inner and outer hash states with respective keys */
	(*ctx->update)(ctx->outer, ko, ctx->block_size);
	(*ctx->update)(ctx->inner, ki, ctx->block_size);
	/* clear stored key data */
	memset(ki, 0, ctx->block_size);
	memset(ko, 0, ctx->block_size);
	/* Load inner hash into state */
	memcpy(ctx->state, ctx->inner, ctx->context_size);
}

void
hmac_update(struct hmac_ctx *ctx, const uint8_t *src, size_t length) {
	(*ctx->update)(ctx->state, src, length);
}

void
hmac_digest(struct hmac_ctx *ctx, uint8_t *dst, size_t length) {
	uint8_t digest[64];		/* 64 bytes is the current largest hash output size */
	/* Finalize inner hash */
	(*ctx->finalize)(ctx->state, digest, ctx->digest_size);
	/* Load outer hash into state */
	memcpy(ctx->state, ctx->outer, ctx->context_size);
	/* Compute HMAC */
	(*ctx->update)(ctx->state, digest, ctx->digest_size);
	(*ctx->finalize)(ctx->state, dst, length);
	/* Reload inner hash into state */
	memcpy(ctx->state, ctx->inner, ctx->context_size);
}

unsigned ATTRIBUTE(pure)
hmac_get_digest_size(const struct hmac_ctx *ctx) {
	return ctx->digest_size;
}

unsigned ATTRIBUTE(pure)
hmac_get_block_size(const struct hmac_ctx *ctx) {
	return ctx->block_size;
}

