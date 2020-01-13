/*
 * ecb.c
 *
 *  Created on: Sep 3, 2019, 12:29:50 PM
 *      Author: Joshua Fehrenbach
 */

#include "ecb.h"
#include <stdlib.h>
#include <assert.h>

void
ecb_init(struct ecb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	const unsigned off = (cipher_meta->context_size % cipher_meta->context_align == 0) ? 0
			: cipher_meta->context_align - (cipher_meta->context_size % cipher_meta->context_align);
	ctx->cipher_ctx = aligned_alloc(cipher_meta->context_align,
			cipher_meta->context_size + off);
	ctx->block_size = cipher_meta->block_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = cipher_meta->context_size;
	ctx->set_encrypt_key = cipher_meta->set_encrypt_key;
	ctx->set_decrypt_key = cipher_meta->set_decrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
	ctx->decrypt = cipher_meta->decrypt;
}

void
ecb_free(struct ecb_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->cipher_ctx, 0, ctx->cipher_size);
	/* Release memory */
	free(ctx->cipher_ctx);
}

void
ecb_set_encrypt_key(struct ecb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == 0);
	(*ctx->set_encrypt_key)(ctx->cipher_ctx, key, key_length);
}

void
ecb_set_decrypt_key(struct ecb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == 0);
	(*ctx->set_decrypt_key)(ctx->cipher_ctx, key, key_length);
}

void
ecb_encrypt(struct ecb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	(*ctx->encrypt)(ctx->cipher_ctx, dst, src, length);
}

void
ecb_decrypt(struct ecb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	(*ctx->decrypt)(ctx->cipher_ctx, dst, src, length);
}

unsigned ATTRIBUTE(pure)
ecb_get_block_size(const struct ecb_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
ecb_get_key_size(const struct ecb_ctx *ctx) {
	return ctx->key_size;
}

unsigned ATTRIBUTE(const)
ecb_get_iv_size(const struct ecb_ctx *ctx) {
	return 0;
}

