/*
 * cfb.c
 *
 *  Created on: Mar 6, 2019, 10:11:35 AM
 *      Author: Joshua Fehrenbach
 */

#include "cfb.h"
#include "memxor.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* If alloca.h is available, CRYPTO_ALLOCA can be defined to
 * remove restrictions on the cipher block size */
#ifdef CRYPTO_ALLOCA
#include <tmp-alloc.h>

#define BUFF_DECL(c) TMP_DECL(buffer, uint8_t)
#define BUFF_ALLOC(s) TMP_ALLOC(buffer, s)
#define BUFF_FREE(s) do { \
	memset(buffer, 0, s); \
	TMP_FREE(buffer); \
} while (0)

#define CFB_INIT(s) void \
cfb##s##_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) { \
	assert (cipher_meta->block_size >= s/8); \
	const unsigned block_size = cipher_meta->block_size; \
	const unsigned context_size = cipher_meta->context_size; \
	const unsigned context_align = cipher_meta->context_align; \
	const unsigned off1 = (context_size % context_align == 0) ? 0 \
			: context_align - (context_size % context_align); \
	const unsigned off2 = (context_size % context_align == 0) ? 0 \
			: context_align - (block_size % context_align); \
	uint8_t *mem = aligned_alloc(context_align, \
			context_size + off1 + block_size + off2); \
	ctx->cipher_ctx = mem; \
	mem += context_size + off1; \
	ctx->iv = mem; \
	ctx->cipher_block_size = cipher_meta->block_size; \
	ctx->block_size = s/8; \
	ctx->key_size = cipher_meta->key_size; \
	ctx->cipher_size = context_size; \
	ctx->set_key = cipher_meta->set_encrypt_key; \
	ctx->encrypt = cipher_meta->encrypt; \
}

#else

#define BUFF_DECL(c) uint8_t buffer[c]
#define BUFF_ALLOC(s)
#define BUFF_FREE(s) memset(buffer, 0, s)

#define CFB_INIT(s) void \
cfb##s##_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) { \
	assert (cipher_meta->block_size <= 32);  \
	assert (cipher_meta->block_size >= s/8); \
	const unsigned block_size = cipher_meta->block_size; \
	const unsigned context_size = cipher_meta->context_size; \
	const unsigned context_align = cipher_meta->context_align; \
	const unsigned off1 = (context_size % context_align == 0) ? 0 \
			: context_align - (context_size % context_align); \
	const unsigned off2 = (context_size % context_align == 0) ? 0 \
			: context_align - (block_size % context_align); \
	uint8_t *mem = aligned_alloc(context_align, \
			context_size + off1 + block_size + off2); \
	ctx->cipher_ctx = mem; \
	mem += context_size + off1; \
	ctx->iv = mem; \
	ctx->cipher_block_size = cipher_meta->block_size; \
	ctx->block_size = s/8; \
	ctx->key_size = cipher_meta->key_size; \
	ctx->cipher_size = context_size; \
	ctx->set_key = cipher_meta->set_encrypt_key; \
	ctx->encrypt = cipher_meta->encrypt; \
}

#endif

void
cfb_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
#ifndef CRYPTO_ALLOCA
	assert (cipher_meta->block_size <= 32);		/* restricted to to cfb_encrypt fixed buffer size */
#endif
	const unsigned block_size = cipher_meta->block_size;
	const unsigned context_size = cipher_meta->context_size;
	const unsigned context_align = cipher_meta->context_align;
	const unsigned off1 = (context_size % context_align == 0) ? 0
			: context_align - (context_size % context_align);
	const unsigned off2 = (block_size % context_align == 0) ? 0
			: context_align - (block_size % context_align);
	uint8_t *mem = aligned_alloc(cipher_meta->context_align,
			context_size + off1 + block_size + off2);
	ctx->cipher_ctx = mem;
	mem += context_size + off1;
	ctx->iv = mem;
	ctx->cipher_block_size = cipher_meta->block_size;
	ctx->block_size = cipher_meta->block_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = context_size;
	ctx->set_key = cipher_meta->set_encrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
}


CFB_INIT(8)
CFB_INIT(16)
CFB_INIT(24)
CFB_INIT(32)
CFB_INIT(40)
CFB_INIT(48)
CFB_INIT(56)
CFB_INIT(64)
CFB_INIT(72)
CFB_INIT(80)
CFB_INIT(88)
CFB_INIT(96)
CFB_INIT(104)
CFB_INIT(112)
CFB_INIT(120)
CFB_INIT(128)

/* These are theoretical and currently have no use. */
/* Attempting to use them will result in an assertion failure for any cipher
 * with a block size less than the feedback size of the associated CFB version */
CFB_INIT(136)
CFB_INIT(144)
CFB_INIT(152)
CFB_INIT(160)
CFB_INIT(168)
CFB_INIT(176)
CFB_INIT(184)
CFB_INIT(192)
CFB_INIT(200)
CFB_INIT(208)
CFB_INIT(216)
CFB_INIT(224)
CFB_INIT(232)
CFB_INIT(240)
CFB_INIT(248)
CFB_INIT(256)


void
cfb_free(struct cfb_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->cipher_ctx, 0, ctx->cipher_size);
	memset(ctx->iv, 0, ctx->block_size);
	/* Release memory */
	free (ctx->cipher_ctx);
}

void
cfb_set_key(struct cfb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length) {
	assert (iv_length == ctx->cipher_block_size);
	memcpy(ctx->iv, iv, ctx->cipher_block_size);
	(*ctx->set_key)(ctx->cipher_ctx, key, key_length);
}

void
cfb_encrypt(struct cfb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	const unsigned cipher_block_size = ctx->cipher_block_size;
	crypto_cipher_func *const encrypt = ctx->encrypt;
	BUFF_DECL(32);
	uint8_t *p;

	BUFF_ALLOC(cipher_block_size);

	p = ctx->iv;
	if (block_size == cipher_block_size) {
		/* Case where the feedback size is the same as the cipher's block size */
		if (!(dst + block_size > src && src + block_size > dst)) {
			/* src and dst are different by more than block_size */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, dst, p, cipher_block_size);
				memxor(dst, src, block_size);
				p = dst;
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		} else {
			/* src and dst overlap by at most block_size bytes, so treat as in-place */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, buffer, p, cipher_block_size);
				memxor3(dst, buffer, src, block_size);
				p = dst;
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		}
		/* Update iv */
		if (p != ctx->iv) {
			memcpy(ctx->iv, p, block_size);
		}
	} else {
		/* Case where the feedback size is less than the cipher's block size */
		if (!(dst + block_size > src && src + block_size > dst)) {
			/* src and dst are different by more than block_size */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, dst, p, cipher_block_size);
				memmove(p, p + block_size, cipher_block_size - block_size);
				memxor(dst, src, block_size);
				memcpy(p + block_size, dst, block_size);
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		} else {
			/* src and dst overlap by at most block_size bytes, so treat as in-place */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, buffer, p, cipher_block_size);
				memmove(p, p + block_size, cipher_block_size - block_size);
				memxor3(dst, buffer, src, block_size);
				memcpy(p + block_size, dst, block_size);
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		}
	}
	/* Check for a remaining partial block */
	if (length != 0) {
		(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, cipher_block_size);
		memxor3(dst, buffer, src, length);
		/* Do not update iv, since this is the last block in the message sequence */
	}
	/* Zero sensitive data */
	BUFF_FREE(cipher_block_size);
}

void
cfb_decrypt(struct cfb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	const unsigned cipher_block_size = ctx->cipher_block_size;
	crypto_cipher_func *const encrypt = ctx->encrypt;

	if (block_size == cipher_block_size) {
		/* Case where the feedback size is the same as the cipher's block size */
		if (!(dst + length > src && src + length > dst)) {
			/* src and dst do not overlap */
			size_t left = length % block_size;
			length -= left;
			if (length > 0) {
				(*encrypt)(ctx->cipher_ctx, dst, ctx->iv, block_size);
				(*encrypt)(ctx->cipher_ctx, dst + block_size, src, length - block_size);
				memcpy(ctx->iv, src + length - block_size, block_size);
				memxor(dst, src, length);
			}
			if (left > 0) {
				BUFF_DECL(32);
				BUFF_ALLOC(block_size);
				(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, block_size);
				memxor3(dst + length, src + length, buffer, left);
				BUFF_FREE(block_size);
			}
		} else {
			/* For in-place CFB, decrypt into a temporary buffer of size at most 512 bytes,
			 * and process data in chunks of that size.
			 *
			 * Note that it is assumed that memxor3 operates on data starting
			 * from the end, allowing for some overlap in the operands
			 */
			BUFF_DECL(512);
			size_t buffer_size, left, part;

			left = length % block_size;
			length -= left;

			buffer_size = 512 - (512 % block_size);
			if (length < buffer_size) {
				buffer_size = (length > 0) ? length : block_size;
			}

			BUFF_ALLOC(buffer_size);

			while (length > 0) {
				part = (length > buffer_size) ? buffer_size : length;
				(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, block_size);
				(*encrypt)(ctx->cipher_ctx, buffer + block_size, src, part - block_size);
				memcpy(ctx->iv, src + part - block_size, block_size);
				memxor(dst, buffer, part);
				src += part;
				dst += part;
				length -= part;
			}
			if (left > 0) {
				(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, block_size);
				memxor3(dst + length, src + length, buffer, left);
			}
			/* Zero sensitive data */
			BUFF_FREE(buffer_size);
			memset(buffer, 0, buffer_size);
		}
	} else {
		/* Case where the feedback size is less than the cipher's block size */
		BUFF_DECL(32);
		BUFF_ALLOC(block_size);
		if (!(dst + block_size > src && src + block_size > dst)) {
			/* src and dst are different by more than block_size */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, dst, ctx->iv, cipher_block_size);
				memmove(ctx->iv, ctx->iv + block_size, cipher_block_size - block_size);
				memcpy(ctx->iv + block_size, src, block_size);
				memxor(dst, src, block_size);
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		} else {
			/* src and dst overlap by at most block_size bytes, so treat as in-place */
			while (length >= block_size) {
				(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, cipher_block_size);
				memmove(ctx->iv, ctx->iv + block_size, cipher_block_size - block_size);
				memcpy(ctx->iv + block_size, src, block_size);
				memxor3(dst, buffer, src, block_size);
				src += block_size;
				dst += block_size;
				length -= block_size;
			}
		}
		/* Check for a remaining partial block */
		if (length > 0) {
			(*encrypt)(ctx->cipher_ctx, buffer, ctx->iv, block_size);
			memxor3(dst + length, src + length, buffer, length);
		}
		/* Zero sensitive data */
		BUFF_FREE(block_size);
	}
}

unsigned ATTRIBUTE(pure)
cfb_get_block_size(const struct cfb_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
cfb_get_key_size(const struct cfb_ctx *ctx) {
	return ctx->key_size;
}

unsigned ATTRIBUTE(pure)
cfb_get_iv_size(const struct cfb_ctx *ctx) {
	return ctx->cipher_block_size;
}

#undef CFB_INIT
#undef BUFF_FREE
#undef BUFF_ALLOC
#undef BUFF_DECL
#undef EXPECT

