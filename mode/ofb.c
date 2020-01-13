/*
 * ofb.c
 *
 *  Created on: Mar 6, 2019, 1:43:43 PM
 *      Author: Joshua Fehrenbach
 */

#include "ofb.h"
#include "memxor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
ofb_init(struct ofb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	const unsigned block_size = cipher_meta->block_size;
	const unsigned context_size = cipher_meta->context_size;
	const unsigned offset = (context_size % block_size == 0) ? 0
			: block_size - (context_size % block_size);
	uint8_t *mem = aligned_alloc(cipher_meta->context_align,
			context_size + block_size + offset);
	ctx->cipher_ctx = mem;
	mem += context_size + offset;
	ctx->iv = mem;
	ctx->block_size = block_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = context_size;
	ctx->set_key = cipher_meta->set_encrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
}

void
ofb_free(struct ofb_ctx *ctx) {
	/* Zero all sensitive data */
	unsigned size = ctx->cipher_size;
	size += (ctx->cipher_size % ctx->block_size == 0) ? ctx->block_size
			: 2*ctx->block_size - (ctx->cipher_size % ctx->block_size);
	memset(ctx->cipher_ctx, 0, size);
	free (ctx->cipher_ctx);
}

void
ofb_set_key(struct ofb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == ctx->block_size);
	memcpy(ctx->iv, iv, ctx->block_size);
	(*ctx->set_key)(ctx->cipher_ctx, key, key_length);
}

void
ofb_encrypt(struct ofb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	crypto_cipher_func *const encrypt = ctx->encrypt;
	while (length >= block_size) {
		(*encrypt)(ctx->cipher_ctx, ctx->iv, ctx->iv, block_size);
		memxor3(dst, src, ctx->iv, block_size);
		src += block_size;
		dst += block_size;
		length -= block_size;
	}
	if (length > 0) {
		(*encrypt)(ctx->cipher_ctx, ctx->iv, ctx->iv, block_size);
		memxor3(dst, src, ctx->iv, length);
	}
}

unsigned ATTRIBUTE(pure)
ofb_get_block_size(const struct ofb_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
ofb_get_key_size(const struct ofb_ctx *ctx) {
	return ctx->key_size;
}

