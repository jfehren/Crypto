/*
 * des.c
 *
 *  Created on: Feb 18, 2019, 4:44:32 PM
 *      Author: Joshua Fehrenbach
 */

#include "des-internal.h"
#include <assert.h>

#ifdef CRYPTO_DES_QUICK
static ENCRYPT(DesEncrypt, TEMPQUICK, LOADFIPS, KEYMAPQUICK, SAVEFIPS);
static DECRYPT(DesDecrypt, TEMPQUICK, LOADFIPS, KEYMAPQUICK, SAVEFIPS);
#else
static ENCRYPT(DesEncrypt, TEMPSMALL, LOADFIPS, KEYMAPSMALL, SAVEFIPS);
static DECRYPT(DesDecrypt, TEMPSMALL, LOADFIPS, KEYMAPSMALL, SAVEFIPS);
#endif

static const uint8_t rotors[] = {
#include "./rotors.h"
};

static const unsigned parity_16[16] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
#define PARITY(x) (parity_16[(x)&0xf] ^ parity_16[((x)>>4) & 0xf])

int
des_check_parity(const uint8_t *key, size_t length) {
	for (size_t i = 0; i < length; i++) {
		if (!PARITY(key[i])) {
			return 0;
		}
	}
	return 1;
}

void
des_fix_parity(uint8_t *dst, const uint8_t *src, size_t length) {
	for (size_t i = 0; i < length; i++) {
		dst[i] = src[i] ^ PARITY(src[i]) ^ 1;
	}
}

static int
des_weak_key(const uint8_t *key) {
	static const unsigned char asso_values[0x81] = {
		16,  9, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26,  6,  2, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
		26, 26,  3,  1, 26, 26, 26, 26, 26, 26,
		26, 26, 26, 26, 26, 26, 26,  0,  0
	};

	static const int8_t weak_key_hash[26][4] = {
	/*  0 */ {0x7f,0x7f, 0x7f,0x7f},
	/*  1 */ {0x7f,0x70, 0x7f,0x78},
	/*  2 */ {0x7f,0x0f, 0x7f,0x07},
	/*  3 */ {0x70,0x7f, 0x78,0x7f},
	/*  4 */ {0x70,0x70, 0x78,0x78},
	/*  5 */ {0x70,0x0f, 0x78,0x07},
	/*  6 */ {0x0f,0x7f, 0x07,0x7f},
	/*  7 */ {0x0f,0x70, 0x07,0x78},
	/*  8 */ {0x0f,0x0f, 0x07,0x07},
	/*  9 */ {0x7f,0x00, 0x7f,0x00},
	/* 10 */ {  -1,  -1,   -1,  -1},
	/* 11 */ {  -1,  -1,   -1,  -1},
	/* 12 */ {0x70,0x00, 0x78,0x00},
	/* 13 */ {  -1,  -1,   -1,  -1},
	/* 14 */ {  -1,  -1,   -1,  -1},
	/* 15 */ {0x0f,0x00, 0x07,0x00},
	/* 16 */ {0x00,0x7f, 0x00,0x7f},
	/* 17 */ {0x00,0x70, 0x00,0x78},
	/* 18 */ {0x00,0x0f, 0x00,0x07},
	/* 19 */ {  -1,  -1,   -1,  -1},
	/* 20 */ {  -1,  -1,   -1,  -1},
	/* 21 */ {  -1,  -1,   -1,  -1},
	/* 22 */ {  -1,  -1,   -1,  -1},
	/* 23 */ {  -1,  -1,   -1,  -1},
	/* 24 */ {  -1,  -1,   -1,  -1},
	/* 25 */ {0x00,0x00, 0x00,0x00}
	};

	int8_t k0 = key[0] >> 1;
	int8_t k1 = key[1] >> 1;

	unsigned hash = asso_values[k1 + 1] + asso_values[k0];
	const int8_t *candidate;

	if (hash > 25) {
		return 0;
	}

	candidate = weak_key_hash[hash];

	if (k0 != candidate[0] || k1 != candidate[1]) {
		return 0;
	}
	if ((key[2] >> 1) != k0 || (key[3] >> 1) != k1) {
		return 0;
	}

	k0 = key[4] >> 1;
	k1 = key[5] >> 1;
	if (k0 != candidate[2] || k1 != candidate[3]) {
		return 0;
	}
	if ((key[6] >> 1) != k0 || (key[7] >> 1) != k1) {
		return 0;
	}
	return 1;
}

int
des_set_key(const struct des_ctx *ctx, const uint8_t *key, size_t length) {
	register uint32_t n, w;
	register char * b0, * b1;
	char bits0[56], bits1[56];
	uint32_t *method;
	const uint8_t *k;
	assert (length == DES3_KEY_SIZE);

	/* explode the bits */
	n = 56;
	b0 = bits0;
	b1 = bits1;
	k = key;
	do {
		w = (256 | *k++) << 2;
		do {
			--n;
			b1[n] = 8 & w;
			w >>= 1;
			b0[n] = 4 & w;
		} while (w >= 16);
	} while (n);

	/* put the bits in the correct places */
	n = 16;
	k = rotors;
	method = ctx->key;

	do {
		w   = (b1[k[ 0   ]] | b0[k[ 1   ]]) << 28;
		w  |= (b1[k[ 2   ]] | b0[k[ 3   ]]) << 26;
		w  |= (b1[k[ 4   ]] | b0[k[ 5   ]]) << 24;
		w  |= (b1[k[ 6   ]] | b0[k[ 7   ]]) << 20;
		w  |= (b1[k[ 8   ]] | b0[k[ 9   ]]) << 18;
		w  |= (b1[k[10   ]] | b0[k[11   ]]) << 16;
		w  |= (b1[k[12   ]] | b0[k[13   ]]) << 12;
		w  |= (b1[k[14   ]] | b0[k[15   ]]) << 10;
		w  |= (b1[k[16   ]] | b0[k[17   ]]) <<  8;
		w  |= (b1[k[18   ]] | b0[k[19   ]]) <<  4;
		w  |= (b1[k[20   ]] | b0[k[21   ]]) <<  2;
		w  |= (b1[k[22   ]] | b0[k[23   ]]) <<  0;

		method[0] = w;

		w   = (b1[k[ 0+24]] | b0[k[ 1+24]]) << 24;
		w  |= (b1[k[ 2+24]] | b0[k[ 3+24]]) << 22;
		w  |= (b1[k[ 4+24]] | b0[k[ 5+24]]) << 20;
		w  |= (b1[k[ 6+24]] | b0[k[ 7+24]]) << 26;
		w  |= (b1[k[ 8+24]] | b0[k[ 9+24]]) << 14;
		w  |= (b1[k[10+24]] | b0[k[11+24]]) << 12;
		w  |= (b1[k[12+24]] | b0[k[13+24]]) <<  8;
		w  |= (b1[k[14+24]] | b0[k[15+24]]) <<  6;
		w  |= (b1[k[16+24]] | b0[k[17+24]]) <<  4;
		w  |= (b1[k[18+24]] | b0[k[19+24]]) <<  0;
		w  |= (b1[k[20+24]] | b0[k[21+24]]) >>  2;
		w  |= (b1[k[20+24]] | b0[k[21+24]]) << 30;
		w  |= (b1[k[22+24]] | b0[k[23+24]]) << 28;

		method[1] = w;

		k += 48;
		method += 2;
	} while (--n);

	return !des_weak_key(key);
}

void
des_encrypt(const struct des_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	register const uint32_t *key = ctx->key;
	while (length >= 8) {
		DesEncrypt(dst, key, src);
		dst += 8;
		src += 8;
		length -= 8;
	}
}

void
des_decrypt(const struct des_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	register const uint32_t *key = ctx->key;
	while (length >= 8) {
		DesDecrypt(dst, key, src);
		dst += 8;
		src += 8;
		length -= 8;
	}
}


int
des3_set_key(struct des3_ctx *ctx, const uint8_t *key, size_t length) {
	int is_bad;
	assert (length == DES3_KEY_SIZE);
	is_bad  = des_set_key(&ctx->des[0], key +  0, DES_KEY_SIZE);
	is_bad |= des_set_key(&ctx->des[1], key +  8, DES_KEY_SIZE);
	is_bad |= des_set_key(&ctx->des[2], key + 16, DES_KEY_SIZE);
	return !is_bad;
}

void
des3_encrypt(const struct des3_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const uint32_t *k1, *k2, *k3;
	k1 = ctx->des[0].key;
	k2 = ctx->des[1].key;
	k3 = ctx->des[2].key;
	while (length >= 8) {
		DesEncrypt(dst, k1, src);
		DesDecrypt(dst, k2, dst);
		DesEncrypt(dst, k3, dst);
		dst += 8;
		src += 8;
		length -= 8;
	}
}

void
des3_decrypt(const struct des3_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const uint32_t *k1, *k2, *k3;
	k1 = ctx->des[0].key;
	k2 = ctx->des[1].key;
	k3 = ctx->des[2].key;
	while (length >= 8) {
		DesDecrypt(dst, k3, src);
		DesEncrypt(dst, k2, dst);
		DesDecrypt(dst, k1, dst);
		dst += 8;
		src += 8;
		length -= 8;
	}
}


