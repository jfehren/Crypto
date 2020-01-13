/*
 * md6.c
 *
 *  Created on: Aug 27, 2019, 12:08:31 PM
 *      Author: Joshua Fehrenbach
 */

#include "md6-internal.h"
#include "macros.h"
#include <assert.h>
#include <string.h>

static void
_read_words_be(uint64_t *dst, const uint8_t *src, size_t length, unsigned off) {
	size_t words, bytes;
	if (off != 0) {
		/* Fill partial word */
		assert (off < 8);
		uint64_t t = *dst;
		off = min(length, 8-off);
		length -= off;
		t <<= 8*off;
		do {
			t |= (((uint64_t)*src++) & 0xFF) << (8*(--off));
		} while (off != 0);
		*dst++ = t;
	}
	words = length / 8;
	bytes = length % 8;
	/* Copy full words */
	if (words != 0) {
		do {
			*dst++ = BE_READ_UINT64(src);
			src += 8;
		} while (--words);
	}
	if (bytes) {
		/* Copy remaining bytes into partial word */
		*dst = ((uint64_t)*src++) & 0xFF;
		while (--bytes) {
			*dst <<= 8;
			*dst |= ((uint64_t)*src++) & 0xFF;
		}
	}
}

void
md6_224_init(struct md6_ctx *ctx) {
	md6_init_full(ctx, 224, NULL, 0, MD6_DEFAULT_L, 96);	/* 40 + (224/4) = 96 */
}

void
md6_256_init(struct md6_ctx *ctx) {
	md6_init_full(ctx, 256, NULL, 0, MD6_DEFAULT_L, 104);	/* 40 + (256/4) = 104 */
}

void
md6_384_init(struct md6_ctx *ctx) {
	md6_init_full(ctx, 384, NULL, 0, MD6_DEFAULT_L, 136);	/* 40 + (384/4) = 136 */
}

void
md6_512_init(struct md6_ctx *ctx) {
	md6_init_full(ctx, 512, NULL, 0, MD6_DEFAULT_L, 168);	/* 40 + (512/4) = 168 */
}

void
md6_init(struct md6_ctx *ctx, int d) {
	assert (1 <= d && d <= 512);
	md6_init_full(ctx, d, NULL, 0, MD6_DEFAULT_L, MD6_DEFAULT_R(d,0));
}

void
md6_init_full(struct md6_ctx *ctx, int d, const uint8_t *K, size_t keylen, int L, int r) {
	assert (1 <= d && d <= 512);
	/* Zero state */
	memset(ctx, 0, sizeof(struct md6_ctx));
	/* Save hash output length */
	ctx->d = d;
	/* Save the key and key length */
	if (K != NULL && keylen != 0) {
		assert (0 <= keylen && keylen <= 64);
		ctx->keylen = keylen;
		_read_words_be(ctx->K, K, keylen, 0);
		/* Fix last key word, if necessary */
		if (keylen % 8 != 0) {
			ctx->K[keylen/8] <<= 8*(8 - (keylen % 8));
		}
	} else {
		ctx->keylen = 0;
	}
	/* Save mode parameter, reverting to default in the case of invalid input */
	ctx->L = (L < 0 || L > MD6_MAX_L) ? MD6_DEFAULT_L : L;
	/* Save round number, reverting to default in the case of invalid input */
	ctx->r = (r <= 0 || r > MD6_MAX_R) ? MD6_DEFAULT_R(d, ctx->keylen) : r;
	/* Set the top of the stack */
	ctx->top = 0;
	/* if SEQ mode for level 1, then use IV=0. Zero bits already there from
	 * memset, we just need to set ctx->bytes[0] */
	if (L == 0) {
		ctx->bytes[0] = MD6_CHUNK_SIZE;
	}
}


void
md6_update(struct md6_ctx *ctx, const uint8_t *data, size_t length) {
	unsigned portion;
	/* Process input blocks */
	if (ctx->bytes[0] % 8 != 0) {
		/* If there is a partial word, handle that first */
		portion = min(length, 8 - (ctx->bytes[0] % 8));
		_read_words_be(ctx->B[0] + (ctx->bytes[0] / 8),
				data, portion, ctx->bytes[0] % 8);
		data += portion;
		ctx->bytes[0] += portion;
		length -= portion;
		assert ((ctx->bytes[0] % 8) == 0 || length == 0);	/* sanity check */
	}
	while (length != 0) {
		if (ctx->bytes[0] == MD6_BLOCK_SIZE) {
			/* Only process a block if we still have input */
			_md6_process(ctx);
		}
		portion = min(length, MD6_BLOCK_SIZE - ctx->bytes[0]);
		_read_words_be(ctx->B[0] + (ctx->bytes[0] / 8), data, portion, 0);
		data += portion;
		ctx->bytes[0] += portion;
		length -= portion;
	}
}

void
md6_digest(struct md6_ctx *ctx, uint8_t *digest, size_t length) {
	int ell, k;
	uint64_t K[MD6_K];
	/* compute final processing */
	if (ctx->top == 0) {
		ell = 0;
	} else {
		for (ell = 0; ell <= ctx->top; ell++) {
			if (ctx->bytes[ell] > 0) {
				break;
			}
		}
	}
	_md6_final(ctx, ell, digest, length);
	/* Reset the state with the same parameters */
	k = ctx->keylen;
	memcpy(K, ctx->K, sizeof(ctx->K));	/* Save key in temporary buffer */
	md6_init_full(ctx, ctx->d, NULL, 0, ctx->L, ctx->r);
	memcpy(ctx->K, K, sizeof(ctx->K));	/* Restore key */
	ctx->keylen = k;
	memset(K, 0, sizeof(K));	/* Remove sensitive info from temporary memory */
}

