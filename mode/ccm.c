/*
 * ccm.c
 *
 *  Created on: Mar 7, 2019, 6:08:54 PM
 *      Author: Joshua Fehrenbach
 */

#include "ccm.h"
#include "ctr.h"
#include "memops.h"
#include "macros.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>

void
ccm_init(struct ccm_ctx *ctx, const struct crypto_cipher_meta *cipher_meta) {
	assert (cipher_meta->block_size == CCM_BLOCK_SIZE);
	ctr_init(&ctx->ctr_ctx, cipher_meta);
	ctx->blength = 0;
	ctx->digest_size = CCM_MAX_DIGEST_SIZE;
}

void
ccm_free(struct ccm_ctx *ctx) {
	/* Zero all sensitive data */
	memset(ctx->tag, 0, sizeof(union crypto_block16));
	/* Release memory */
	ctr_free(&ctx->ctr_ctx);
}

void
ccm_set_key_default(struct ccm_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *nonce, size_t nonce_length) {
	fprintf(stderr, "Cannot set key for CCM mode without knowing the lengths of the Message, "
			"Associated Data, Nonce, and Authentication Tag");
	abort ();
}

/*
 * The format of the CCM IV (for both CTR and CBC-MAC) is: flags | nonce | count
 *  flags = 1 octet
 *  nonce = N octets
 *  count <= 15 - N octets
 *
 * such that:
 *  sizeof(flags) + sizeof(nonce) + sizeof(count) == 1 block
 */
#define CCM_FLAG_Q          0x07
#define CCM_FLAG_T          0x38
#define CCM_FLAG_ADATA      0x40
#define CCM_FLAG_RESERVED   0x80
#define CCM_FLAG_GET_Q(x) (((x) & CCM_FLAG_Q) + 1)
#define CCM_FLAG_SET_Q(x) (((x) - 1) & CCM_FLAG_Q)
#define CCM_FLAG_SET_T(x) (((((x) - 2) >> 1) << 3) & CCM_FLAG_T)

#define CCM_OFFSET_FLAGS    0
#define CCM_OFFSET_NONCE    1
#define CCM_Q_SIZE(nlen)    (CCM_BLOCK_SIZE - CCM_OFFSET_NONCE - (nlen))

#define CBC_MAC(ctx) \
	(*(ctx)->ctr_ctx.encrypt)((ctx)->ctr_ctx.cipher_ctx, \
			(ctx)->tag.b, (ctx)->tag.b, CCM_BLOCK_SIZE)

#define CRYPT(ctx,dst,src,len) ctr_encrypt(&(ctx)->ctr_ctx, (dst), (src), (len))

#define CTR(ctx) ((ctx)->ctr_ctx.ctr)

static void
ccm_build_iv(uint8_t *iv, const uint8_t *nonce, size_t nlen, uint8_t flags, size_t count) {
	unsigned int i;

	iv[CCM_OFFSET_FLAGS] = flags | CCM_FLAG_SET_Q(CCM_Q_SIZE(nlen));
	memcpy(&iv[CCM_OFFSET_NONCE], nonce, nlen);
	for (i = CCM_BLOCK_SIZE-1; i >= CCM_OFFSET_NONCE + nlen; --i) {
		iv[i] = count & 0xff;
		count >>= 8;
	}
	assert (count == 0);
}

inline static void
ccm_pad(struct ccm_ctx *ctx) {
	if (ctx->blength != 0) {
		CBC_MAC(ctx);
	}
	ctx->blength = 0;
}

void
ccm_set_key(struct ccm_ctx *ctx, const uint8_t *key, size_t klen, const uint8_t *nonce,
		size_t nlen, size_t tlen, size_t plen, size_t alen) {
	assert (4 <= tlen && tlen <= CCM_BLOCK_SIZE && (tlen & 1) == 0);
	assert (CCM_MIN_NONCE_SIZE <= nlen && nlen <= CCM_MAX_NONCE_SIZE);

	(*ctx->ctr_ctx.set_key)(ctx->ctr_ctx.cipher_ctx, key, klen);
	/* Generate IV for CTR and CBC-MAC */
	ctx->blength = 0;
	ctx->digest_size = tlen;
	ccm_build_iv(ctx->tag.b, nonce, nlen, CCM_FLAG_SET_T(tlen), plen);
	ccm_build_iv(CTR(ctx), nonce, nlen, 0, 1);

	/* If there is no AAD, encrypt B0 and return */
	if (alen == 0) {
		CBC_MAC(ctx);
		return;
	}

	/* Set AAD flag in B0 and encrypt it */
	ctx->tag.b[CCM_OFFSET_FLAGS] |= CCM_FLAG_ADATA;
	CBC_MAC(ctx);
	/* XOR the encoded length of the AAD into the tag */
#if PTRDIFF_MAX > 0x7fffffff
	if (alen >= (0x01ULL << 32)) {
		/* We'll assume that ptrdiff_t having a size greater than
		 * 32 bits means we have a 64-bit size_t */
		ctx->tag.b[ctx->blength++] ^= 0xff;
		ctx->tag.b[ctx->blength++] ^= 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 56) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 48) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 40) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 32) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 24) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 16) & 0xff;
	} else
#endif
	if (alen >= ((0x1ULL << 16) - (0x1ULL << 8))) {
		ctx->tag.b[ctx->blength++] ^= 0xff;
		ctx->tag.b[ctx->blength++] ^= 0xfe;
		ctx->tag.b[ctx->blength++] ^= (alen >> 24) & 0xff;
		ctx->tag.b[ctx->blength++] ^= (alen >> 16) & 0xff;
	}
	ctx->tag.b[ctx->blength++] ^= (alen >> 8) & 0xff;
	ctx->tag.b[ctx->blength++] ^= (alen >> 0) & 0xff;
}

void
ccm_update(struct ccm_ctx *ctx, const uint8_t *aad, size_t length) {
	if (ctx->blength + length < CCM_BLOCK_SIZE) {
		/* We don't have enough data to process, so save it for later */
		memxor(&ctx->tag.b[ctx->blength], aad, length);
		ctx->blength += length;
		return;
	}
	if (ctx->blength != 0) {
		/* We have data saved, so append AAD and process it */
		memxor(&ctx->tag.b[ctx->blength], aad, CCM_BLOCK_SIZE - ctx->blength);
		aad += CCM_BLOCK_SIZE - ctx->blength;
		length -= CCM_BLOCK_SIZE - ctx->blength;
		CBC_MAC(ctx);
	}
	/* Process full blocks */
	while (length >= CCM_BLOCK_SIZE) {
		memxor(ctx->tag.b, aad, CCM_BLOCK_SIZE);
		aad += CCM_BLOCK_SIZE;
		length -= CCM_BLOCK_SIZE;
		CBC_MAC(ctx);
	}
	/* Save any remaining data */
	ctx->blength = length;
	if (ctx->blength != 0) {
		memxor(ctx->tag.b, aad, ctx->blength);
	}
}

void
ccm_encrypt(struct ccm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	ccm_pad(ctx);
	ccm_update(ctx, src, length);
	CRYPT(ctx, dst, src, length);
}

void
ccm_decrypt(struct ccm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	CRYPT(ctx, dst, src, length);
	ccm_pad(ctx);
	ccm_update(ctx, dst, length);
}

void
ccm_digest(struct ccm_ctx *ctx, const uint8_t *digest) {
	int n = CCM_BLOCK_SIZE - CCM_FLAG_GET_Q(CTR(ctx)[CCM_OFFSET_FLAGS]);
	while (n < CCM_BLOCK_SIZE) {
		CTR(ctx)[n++] = 0;
	}
	ccm_pad(ctx);
	CRYPT(ctx, digest, ctx->tag.b, ctx->digest_size);
}

int
ccm_encrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length) {
	struct ccm_ctx ctx;
	ccm_init(&ctx, cipher_meta);
	ccm_set_key(&ctx, key, key_length, nonce, nonce_length,
			digest_length, msg_length, aad_length);

	ccm_update(&ctx, aad, aad_length);
	ccm_encrypt(&ctx, dst, src, msg_length);
	ccm_digest(&ctx, dst + msg_length);

	ccm_free(&ctx);
	return 1;
}

int
ccm_decrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length) {
	struct ccm_ctx ctx;
	uint8_t tag[CCM_BLOCK_SIZE];
	ccm_init(&ctx, cipher_meta);
	ccm_set_key(&ctx, key, key_length, nonce, nonce_length,
			digest_length, msg_length, aad_length);

	ccm_update(&ctx, aad, aad_length);
	ccm_decrypt(&ctx, dst, src, msg_length);
	ccm_digest(&ctx, tag);

	ccm_free(&ctx);

	return memeql_sec(tag, src + msg_length, digest_length);
}

unsigned ATTRIBUTE(pure)
ccm_get_block_size(const struct ccm_ctx *ctx) {
	return CCM_BLOCK_SIZE;
}

unsigned ATTRIBUTE(pure)
ccm_get_key_size(const struct ccm_ctx *ctx) {
	return ctx->ctr_ctx.key_size;
}

unsigned ATTRIBUTE(pure)
ccm_get_nonce_size(const struct ccm_ctx *ctx) {
	return CCM_Q_SIZE(CCM_FLAG_GET_Q(&ctx->tag.b[CCM_OFFSET_FLAGS]));
}

unsigned ATTRIBUTE(pure)
ccm_get_digest_size(const struct ccm_ctx *ctx) {
	return ctx->digest_size;
}

