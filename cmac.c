/*
 * cmac.c
 *
 *  Created on: Sep 25, 2019, 11:20:51 AM
 *      Author: Joshua Fehrenbach
 */

#include "cmac.h"
#include "memxor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static const uint8_t R64 = 0x1B;

static const uint8_t R128 = 0x87;

void
cmac_init(struct cmac_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	const unsigned ctx_size = cipher_meta->context_size;
	const unsigned ctx_align = cipher_meta->context_align;
	const unsigned blk_size = cipher_meta->block_size;
	const unsigned off1 = (ctx_size % ctx_align == 0) ? 0
			: ctx_align - (ctx_size % ctx_align);
	const unsigned off2 = (blk_size % ctx_align == 0) ? 0
			: ctx_align - (blk_size % ctx_align);
	/* Allocate all memory in a single block with enough extra space for everything
	 * to have its proper padding */
	uint8_t *mem = aligned_alloc(ctx_align, ctx_size + off1 + 3*blk_size + 3*off2);
	ctx->ctx = mem;
	mem += ctx_size + off1;
	ctx->buf = mem;
	mem += blk_size + off2;
	ctx->k1 = mem;
	mem += blk_size + off2;
	ctx->k2 = mem;
	ctx->block_size = blk_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = ctx_size;
	ctx->set_key = cipher_meta->set_encrypt_key;
	ctx->crypt = cipher_meta->encrypt;
}

void
cmac_free(struct cmac_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->ctx, 0, ctx->cipher_size);
	memset(ctx->buf, 0, ctx->block_size);
	memset(ctx->k1, 0, ctx->block_size);
	memset(ctx->k2, 0, ctx->block_size);
	/* Release memory */
	free(ctx->ctx);
}

void
cmac_set_key(struct cmac_ctx *ctx, const uint8_t *key, size_t key_length) {
	int i;
	const uint8_t R;
	assert (ctx->block_size == 8 || ctx->block_size == 16);
	R = (ctx->block_size == 16 ? R128 : R64);
	(*ctx->set_key)(ctx->ctx, key, key_length);
	/* L = CIPH_K(0) */
	memset(ctx->buf, 0, ctx->block_size);
	(*ctx->crypt)(ctx->ctx, ctx->buf, ctx->buf, ctx->block_size);
	/* K1 = (MSB(L) == 0 ? L << 1 : (L << 1) ^ R) */
	for (i = 0; i < ctx->block_size-1; i++) {
		ctx->k1[i] = (ctx->buf[i] << 1) | (ctx->buf[i+1] >> 7);
	}
	ctx->k1[i] = (ctx->buf[0] & 0x80) ? ((ctx->buf[i] << 1) ^ R)
			: (ctx->buf[i] << 1);
	/* K2 = (MSB(K1) == 0 ? K1 << 1 : (K1 << 1) ^ R) */
	for (i = 0; i < ctx->block_size-1; i++) {
		ctx->k2[i] = (ctx->k1[i] << 1) | (ctx->k1[i+1] >> 7);
	}
	ctx->k2[i] = (ctx->k1[0] & 0x80) ? ((ctx->k1[i] << 1) ^ R)
			: (ctx->k1[i] << 1);
	/* Initial chaining block is the zero-string */
	memset(ctx->buf, 0, ctx->block_size);
	ctx->index = 0;
}

void
cmac_update(struct cmac_ctx *ctx, const uint8_t *src, size_t length) {
	/* Always maintain at least 1 (possibly full) block  */
	if (length == 0) {
		/* Nothing to be done */
		return;
	} else if (ctx->index < ctx->block_size) {
		/* fill partial block first */
		unsigned left = ctx->block_size - ctx->index;
		if (length <= left) {
			/* At most enough to fill partial block */
			memxor(ctx->buf + ctx->index, src, length);
			ctx->index += length;
			return;
		}
		/* Enough to fill partial block with input left over */
		memxor(ctx->buf + ctx->index, src, left);
		src += left;
		length -= left;
	}
	/* Process block already in the buffer */
	(*ctx->crypt)(ctx->ctx, ctx->buf, ctx->buf, ctx->block_size);
	/* Process all but 1 (possibly full) block */
	while (length > ctx->block_size) {
		memxor(ctx->buf, src, ctx->block_size);
		(*ctx->crypt)(ctx->ctx, ctx->buf, ctx->buf, ctx->block_size);

		src += ctx->block_size;
		length -= ctx->block_size;
	}
	/* Save last (possibly full) block */
	memxor(ctx->buf, src, length);
	ctx->index = length;
}

void
cmac_digest(struct cmac_ctx *ctx, uint8_t *dst, size_t length) {
	assert (0 <= length && length <= ctx->block_size);
	if (ctx->index == ctx->block_size) {
		/* Last block was a full block, so use K1 */
		memxor(ctx->buf, ctx->k1, ctx->block_size);
	} else {
		/* Last block was partial, so pad and use K2 */
		ctx->buf[ctx->block_size] ^= 0x80;
		memxor(ctx->buf, ctx->k2, ctx->block_size);
	}
	/* Process final block */
	(*ctx->crypt)(ctx->ctx, ctx->buf, ctx->buf, ctx->block_size);
	/* Output digest */
	memcpy(dst, ctx->buf, length);
}

unsigned ATTRIBUTE(pure)
cmac_get_digest_size(const struct cmac_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
cmac_get_block_size(const struct cmac_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
cmac_get_key_size(const struct cmac_ctx *ctx) {
	return ctx->key_size;
}

