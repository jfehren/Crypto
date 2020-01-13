/*
 * blowfish.c
 *
 *  Created on: Feb 18, 2019, 1:04:35 PM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <string.h>	/* for memcpy */
#include "blowfish-internal.h"
#include "macros.h"

#define S0(x,s)	(s)[0][((x)>>24) & 0xFF]
#define S1(x,s)	(s)[1][((x)>>16) & 0xFF]
#define S2(x,s)	(s)[2][((x)>> 8) & 0xFF]
#define S3(x,s)	(s)[3][((x)>> 0) & 0xFF]

#define F(x,s)	(((S0(x,s) + S1(x,s)) ^ S2(x,s)) + S3(x,s))
#define R(l,r,i,p,s)	(l) ^= (p)[i]; (r) ^= F(l,s)

#define ENCRYPT(l, r, p, s) \
do { \
	R(l,r,  0, p,s); R(r,l,  1, p,s); R(l,r,  2, p,s); R(r,l,  3, p,s); \
	R(l,r,  4, p,s); R(r,l,  5, p,s); R(l,r,  6, p,s); R(r,l,  7, p,s); \
	R(l,r,  8, p,s); R(r,l,  9, p,s); R(l,r, 10, p,s); R(r,l, 11, p,s); \
	R(l,r, 12, p,s); R(r,l, 13, p,s); R(l,r, 14, p,s); R(r,l, 15, p,s); \
	(l) ^= (p)[16]; (r) ^= (p)[17]; \
} while (0)

#define DECRYPT(l, r, p, s) \
do { \
	R(l,r, 17, p,s); R(r,l, 16, p,s); R(l,r, 15, p,s); R(r,l, 14, p,s); \
	R(l,r, 13, p,s); R(r,l, 12, p,s); R(l,r, 11, p,s); R(r,l, 10, p,s); \
	R(l,r,  9, p,s); R(r,l,  8, p,s); R(l,r,  7, p,s); R(r,l,  6, p,s); \
	R(l,r,  5, p,s); R(r,l,  4, p,s); R(l,r,  3, p,s); R(r,l,  2, p,s); \
	(l) ^= (p)[1]; (r) ^= (p)[0]; \
} while (0)

void
blowfish_encrypt(const struct blowfish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register const uint32_t (*stable)[256];
	register const uint32_t *ptable;
	register uint32_t d1, d2;
	stable = ctx->s;
	ptable = ctx->p;

	while (length >= 8) {
		d1 = BE_READ_UINT32(src + 0);
		d2 = BE_READ_UINT32(src + 4);
		ENCRYPT(d1, d2, ptable, stable);
		BE_WRITE_UINT32(dst + 0, d1);
		BE_WRITE_UINT32(dst + 4, d2);
		src += 8;
		dst += 8;
		length -= 8;
	}
}

void
blowfish_decrypt(const struct blowfish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register const uint32_t (*stable)[256];
	register const uint32_t *ptable;
	register uint32_t d1, d2;
	stable = ctx->s;
	ptable = ctx->p;

	while (length >= 8) {
		d1 = BE_READ_UINT32(src + 0);
		d2 = BE_READ_UINT32(src + 4);
		DECRYPT(d1, d2, ptable, stable);
		BE_WRITE_UINT32(dst + 0, d1);
		BE_WRITE_UINT32(dst + 4, d2);
		src += 8;
		dst += 8;
		length -= 8;
	}
}

int
blowfish_set_key(struct blowfish_ctx *ctx, const uint8_t *key, size_t length) {
	assert (BLOWFISH_MIN_KEY_SIZE <= length && length <= BLOWFISH_MAX_KEY_SIZE);
	unsigned i, j;
	uint32_t x, xl, xr;
	memcpy(ctx, &_blowfish_init_ctx, sizeof(struct blowfish_ctx));
	/* Define special cases where key_len is a multiple of 2 or 4
	   to avoid excessive usage of the mod operator */
	if (length % 4 == 0) {
		/* Simplest case, where key_len is a multiple of 4 */
		for (i = j = 0; i < _BLOWFISH_ROUNDS+2; i++) {
			x = BE_READ_UINT32(key + j);
			ctx->p[i] ^= x;
			if ((j += 4) == length) { /* Assignment intentional */
				j = 0;
			}
		}
	} else if (length % 4 == 2) {
		/* Less simple case, where key_len is a multiple of 2,
		   but not a multiple of 4 (i.e. 10, 14, 18) */
		i = 0;
		while (i < _BLOWFISH_ROUNDS+2) {
			/* Start by iterating until j+2 == key_len */
			for (j = 0; i < _BLOWFISH_ROUNDS+2 && j+2 < length; i++, j += 4) {
				x = BE_READ_UINT32(key + j);
				ctx->p[i] ^= x;
			}
			if (i >= _BLOWFISH_ROUNDS+2) {
				break;
			}
			/* Then handle the word that contains bytes wrapping
			   around to the start of the input key */
			x = (((uint32_t)BE_READ_UINT16(key+length-2)) << 16) |
				(((uint32_t)BE_READ_UINT16(key))           <<  0);
			ctx->p[i] ^= x;
			i++;
			/* Finish by iterating until j == key_len, starting
			   with j = 2 and incrementing by 4 */
			for (j = 2; i < _BLOWFISH_ROUNDS+2 && j < length; i++, j += 4) {
				x = (key[j+0] << 24) | (key[j+1] << 16) |
					(key[j+2] <<  8) | (key[j+3] <<  0);
				ctx->p[i] ^= x;
			}
			/* Repeat until i == BLOWFISH_PTABLE_LENGTH */
		}
	} else {
		/* Generic case, where key_len is not a multiple of 2 or 4 */
		for (i = j = 0; i < _BLOWFISH_ROUNDS+2; i++, j += 4) {
			x = (key[(j+0) % length] << 24) | (key[(j+1) % length] << 16) |
				(key[(j+2) % length] <<  8) | (key[(j+3) % length] <<  0);
			ctx->p[i] ^= x;
		}
	}
	xl = xr = 0;
	for (i = 0; i < _BLOWFISH_ROUNDS+2; i += 2) {
		ENCRYPT(xl, xr, ctx->p, ctx->s);
		ctx->p[i+0] = xl;
		ctx->p[i+1] = xr;
	}
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 256; j += 2) {
			ENCRYPT(xl, xr, ctx->p, ctx->s);
			ctx->s[i][j+0] = xl;
			ctx->s[i][j+1] = xr;
		}
	}

	/* Check for weak key. A weak key is a key in which a value in
	 * the P-array (here s) occurs more than once per table.  */
	for (i = 0; i < 255; i++) {
		for (j = i + 1; j < 256; j++) {
			if ((ctx->s[0][i] == ctx->s[0][j]) || (ctx->s[1][i] == ctx->s[1][j]) ||
				(ctx->s[2][i] == ctx->s[2][j]) || (ctx->s[3][i] == ctx->s[3][j]))
				return 0;
		}
	}

	return 1;
}

int
blowfish128_set_key(struct blowfish_ctx *ctx, const uint8_t *key) {
	return blowfish_set_key(ctx, key, BLOWFISH128_KEY_SIZE);
}

