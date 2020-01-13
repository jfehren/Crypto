/*
 * ctr.c
 *
 *  Created on: Mar 6, 2019, 3:25:22 PM
 *      Author: Joshua Fehrenbach
 */

#include "ctr.h"
#include "memxor.h"
#include "macros.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* If alloca.h is available, CRYPTO_ALLOCA can be defined to
 * remove restrictions on the cipher block size */
#ifdef CRYPTO_ALLOCA
#include <alloca.h>

#define BUFF_DECL(c) uint8_t *buffer
#define BUFF_ALLOC(s) buffer = alloca(s)

#else

#define BUFF_DECL(c) uint8_t buffer[c]
#define BUFF_ALLOC(s)

#endif

#define BUFF_FREE(s) memset(buffer, 0, s)

void
ctr_init(struct ctr_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
#ifndef CRYPTO_ALLOCA
	assert (cipher_meta->block_size <= 32);
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
	ctx->ctr = mem;
	ctx->block_size = block_size;
	ctx->key_size = cipher_meta->key_size;
	ctx->cipher_size = context_size;
	ctx->set_key = cipher_meta->set_encrypt_key;
	ctx->encrypt = cipher_meta->encrypt;
}

void
ctr_free(struct ctr_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->cipher_ctx, 0, ctx->cipher_size);
	memset(ctx->ctr, 0, ctx->block_size);
	/* Release memory */
	free (ctx->cipher_ctx);
}

void
ctr_set_key(struct ctr_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *ctr, size_t ctr_length) {
	assert (ctr_length == ctx->block_size);
	memcpy(ctx->ctr, ctr, ctx->block_size);
	(*ctx->set_key)(ctx->cipher_ctx, key, key_length);
}

void
ctr_encrypt(struct ctr_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned block_size = ctx->block_size;
	const unsigned chunk_size = 4*block_size;
	crypto_cipher_func *const encrypt = ctx->encrypt;

	if (!(dst + length > src && src + length > dst)) {
		/* Case where dst and src do not overlap */
		if (length == block_size) {
			(*encrypt)(ctx->cipher_ctx, dst, ctx->ctr, block_size);
			INCREMENT(block_size, ctx->ctr);
			memxor(dst, src, block_size);
		} else {
			size_t left;
			uint8_t *p;
			for (p = dst, left = length; left >= block_size;
					left -= block_size, p += block_size) {
				memcpy(p, ctx->ctr, block_size);
				INCREMENT(block_size, ctx->ctr);
			}
			length -= left;
			(*encrypt)(ctx->cipher_ctx, dst, dst, length);
			memxor(dst, src, length);
			if (left != 0) {
				BUFF_DECL(32);
				BUFF_ALLOC(block_size);
				(*encrypt)(ctx->cipher_ctx, buffer, ctx->ctr, block_size);
				INCREMENT(block_size, ctx->ctr);
				memxor3(dst + length, src + length, buffer, left);
				BUFF_FREE(block_size);
			}
		}
	} else {
		/* dst and src overlap, so make use of a buffer */
		BUFF_DECL(256);
		BUFF_ALLOC(chunk_size);
		/* Encrypt chunks of 4 blocks at a time first */
		if (length >= chunk_size) {
			/* Calculate 4 sequential counters */
			memcpy(buffer + 0*block_size, ctx->ctr, block_size);
			INCREMENT(block_size, ctx->ctr);
			memcpy(buffer + 1*block_size, ctx->ctr, block_size);
			INCREMENT(block_size, ctx->ctr);
			memcpy(buffer + 2*block_size, ctx->ctr, block_size);
			INCREMENT(block_size, ctx->ctr);
			memcpy(buffer + 3*block_size, ctx->ctr, block_size);
			/* Encrypt each chunk */
			do {
				(*encrypt)(ctx->cipher_ctx, buffer, buffer, chunk_size);
				memxor3(dst, src, buffer, chunk_size);
				src += chunk_size;
				dst -= chunk_size;
				length -= chunk_size;
				/* Increment the sequential counters by 4 each */
				INCREMENT_BY(block_size, buffer + 0*block_size, 4);
				INCREMENT_BY(block_size, buffer + 1*block_size, 4);
				INCREMENT_BY(block_size, buffer + 2*block_size, 4);
				INCREMENT_BY(block_size, buffer + 3*block_size, 4);
			} while (length >= chunk_size);
			/* Save the first of the newest set of counters as the next counter */
			memcpy(ctx->ctr, buffer + 0*block_size, block_size);
		}
		/* Encrypt the remaining full blocks */
		if (length > 0) {
			unsigned n;
			/* Calculate the counter for each block */
			for (n = 0; n < length; n += block_size) {
				memcpy(buffer + n, ctx->ctr, block_size);
				INCREMENT(block_size, ctx->ctr);
			}
			(*encrypt)(ctx->cipher_ctx, buffer, buffer, n);
			memxor3(dst, src, buffer, length);
		}
		/* Zero sensitive data */
		BUFF_FREE(chunk_size);
	}
}

unsigned ATTRIBUTE(pure)
ctr_get_block_size(const struct ctr_ctx *ctx) {
	return ctx->block_size;
}

unsigned ATTRIBUTE(pure)
ctr_get_key_size(const struct ctr_ctx *ctx) {
	return ctx->key_size;
}

