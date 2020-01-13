/*
 * cbc.c
 *
 *  Created on: Mar 5, 2019, 3:12:31 PM
 *      Author: Joshua Fehrenbach
 */

#include "cbc.h"
#include "memxor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void
cbc_init(struct cbc_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	const unsigned block_size = cipher_meta->block_size;
	const unsigned context_size = cipher_meta->context_size;
	const unsigned context_align = cipher_meta->context_align;
	const unsigned off1 = (context_size % context_align == 0) ? 0
			: context_align - (context_size % context_align);
	const unsigned off2 = (block_size % context_align == 0) ? 0
			: context_align - (block_size % context_align);
	uint8_t *mem = aligned_alloc(context_align,
			context_size + off1 + block_size + off2);
	ctx->cipher_ctx = mem;
	mem += context_size + off1;
	ctx->iv = mem;
	ctx->block_size = block_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = context_size;
	ctx->set_encrypt_key = cipher_meta->set_encrypt_key;
	ctx->set_decrypt_key = cipher_meta->set_decrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
	ctx->decrypt = cipher_meta->decrypt;
}

void
cbc_free(struct cbc_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->cipher_ctx, 0, ctx->cipher_size);
	memset(ctx->iv, 0, ctx->block_size);
	/* Release memory */
	free(ctx->cipher_ctx);
}

void
cbc_set_encrypt_key(struct cbc_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == ctx->block_size);
	memcpy(ctx->iv, iv, ctx->block_size);
	(*ctx->set_encrypt_key)(ctx->cipher_ctx, key, key_length);
}

void
cbc_set_decrypt_key(struct cbc_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == ctx->block_size);
	memcpy(ctx->iv, iv, ctx->block_size);
	(*ctx->set_decrypt_key)(ctx->cipher_ctx, key, key_length);
}

void
cbc_encrypt(struct cbc_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	crypto_cipher_func *const encrypt = ctx->encrypt;
	while (length >= block_size) {
		memxor(ctx->iv, src, block_size);
		(*encrypt)(ctx->cipher_ctx, dst, ctx->iv, block_size);
		memcpy(ctx->iv, dst, block_size);
		src += block_size;
		dst += block_size;
		length -= block_size;
	}
}

void
cbc_decrypt(struct cbc_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	if (length < block_size) {
		return;
	}
	crypto_cipher_func *const decrypt = ctx->decrypt;
	length -= length % block_size;
	if (!(dst + length > src && src + length > dst)) {
		/* Case where dst and src don't overlap */
		(*decrypt)(ctx->cipher_ctx, dst, src, length);
		memxor(dst, ctx->iv, block_size);
		memxor(dst + block_size, src, length - block_size);
		memcpy(ctx->iv, src + length - block_size, block_size);
	} else {
		/* For in-place CBC, decrypt into a temporary buffer of size at most 512 bytes,
		 * and process data in chunks of that size.
		 *
		 * Note that it is assumed that block_size <= 512, and that memxor3 operates on
		 * data starting from the end, allowing for some overlap in the operands
		 */
		uint8_t buffer[512];
		uint8_t init_iv[16];
		size_t buffer_size;

		if (length <= 512) {
			buffer_size = length;
		} else {
			buffer_size = 512 - (512 % block_size);
		}

		while (length > buffer_size) {
			(*decrypt)(ctx->cipher_ctx, buffer, src, buffer_size);
			memcpy(init_iv, ctx->iv, block_size);
			memcpy(ctx->iv, src + buffer_size - block_size, block_size);
			memxor3(dst, buffer, init_iv, block_size);
			memxor3(dst + block_size, buffer + block_size, src, buffer_size - block_size);
			src += buffer_size;
			dst += buffer_size;
			length -= buffer_size;
		}
		/* Decrypt last (partial) chunk */
		(*decrypt)(ctx->cipher_ctx, buffer, src, length);
		memcpy(init_iv, ctx->iv, block_size);
		memcpy(ctx->iv, src + length - block_size, block_size);
		memxor3(dst, buffer, init_iv, block_size);
		memxor3(dst + block_size, buffer + block_size, src, length - block_size);
		/* Zero sensitive data */
		memset(buffer, 0, sizeof(buffer));
		memset(init_iv, 0, sizeof(init_iv));
	}
}

unsigned ATTRIBUTE(pure)
cbc_get_block_size(const struct cbc_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
cbc_get_key_size(const struct cbc_ctx *ctx) {
	return ctx->key_size;
}

