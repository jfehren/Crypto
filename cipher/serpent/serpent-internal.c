/*
 * serpent-internal.c
 *
 *  Created on: Feb 25, 2019, 9:36:15 AM
 *      Author: Joshua Fehrenbach
 */

#include "serpent-internal.h"
#include "crypto-internal.h"
#include "macros.h"
#include <assert.h>

#define KBOX0(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t05, t06, t07, t08, t09; \
	uint32_t t11, t12, t13, t14, t15, t17, t01; \
	t01 = b   ^ c  ; \
	t02 = a   | d  ; \
	t03 = a   ^ b  ; \
	z   = t02 ^ t01; \
	t05 = c   | z  ; \
	t06 = a   ^ d  ; \
	t07 = b   | c  ; \
	t08 = d   & t05; \
	t09 = t03 & t07; \
	y   = t09 ^ t08; \
	t11 = t09 & y  ; \
	t12 = c   ^ d  ; \
	t13 = t07 ^ t11; \
	t14 = b   & t06; \
	t15 = t06 ^ t13; \
	w   =     ~ t15; \
	t17 = w   ^ t14; \
	x   = t12 ^ t17; \
} while (0)
#define KBOX1(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t04, t05, t06, t07, t08; \
	uint32_t t10, t11, t12, t13, t16, t17, t01; \
	t01 = a   | d  ; \
	t02 = c   ^ d  ; \
	t03 =     ~ b  ; \
	t04 = a   ^ c  ; \
	t05 = a   | t03; \
	t06 = d   & t04; \
	t07 = t01 & t02; \
	t08 = b   | t06; \
	y   = t02 ^ t05; \
	t10 = t07 ^ t08; \
	t11 = t01 ^ t10; \
	t12 = y   ^ t11; \
	t13 = b   & d  ; \
	z   =     ~ t10; \
	x   = t13 ^ t12; \
	t16 = t10 | x  ; \
	t17 = t05 & t16; \
	w   = c   ^ t17; \
} while (0)
#define KBOX2(a,b,c,d, w,x,y,z) \
do {					   \
	uint32_t t02, t03, t05, t06, t07, t08; \
	uint32_t t09, t10, t12, t13, t14, t01; \
	t01 = a   | c  ; \
	t02 = a   ^ b  ; \
	t03 = d   ^ t01; \
	w   = t02 ^ t03; \
	t05 = c   ^ w  ; \
	t06 = b   ^ t05; \
	t07 = b   | t05; \
	t08 = t01 & t06; \
	t09 = t03 ^ t07; \
	t10 = t02 | t09; \
	x   = t10 ^ t08; \
	t12 = a   | d  ; \
	t13 = t09 ^ x  ; \
	t14 = b   ^ t13; \
	z   =     ~ t09; \
	y   = t12 ^ t14; \
} while (0)
#define KBOX3(a,b,c,d, w,x,y,z) \
do {						\
	uint32_t t02, t03, t04, t05, t06, t07, t08; \
	uint32_t t09, t10, t11, t13, t14, t15, t01; \
	t01 = a   ^ c  ; \
	t02 = a   | d  ; \
	t03 = a   & d  ; \
	t04 = t01 & t02; \
	t05 = b   | t03; \
	t06 = a   & b  ; \
	t07 = d   ^ t04; \
	t08 = c   | t06; \
	t09 = b   ^ t07; \
	t10 = d   & t05; \
	t11 = t02 ^ t10; \
	z   = t08 ^ t09; \
	t13 = d   | z  ; \
	t14 = a   | t07; \
	t15 = b   & t13; \
	y   = t08 ^ t11; \
	w   = t14 ^ t15; \
	x   = t05 ^ t04; \
} while (0)
#define KBOX4(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t04, t05, t06, t08, t09; \
	uint32_t t10, t11, t12, t13, t14, t15, t16, t01; \
	t01 = a   | b  ; \
	t02 = b   | c  ; \
	t03 = a   ^ t02; \
	t04 = b   ^ d  ; \
	t05 = d   | t03; \
	t06 = d   & t01; \
	z   = t03 ^ t06; \
	t08 = z   & t04; \
	t09 = t04 & t05; \
	t10 = c   ^ t06; \
	t11 = b   & c  ; \
	t12 = t04 ^ t08; \
	t13 = t11 | t03; \
	t14 = t10 ^ t09; \
	t15 = a   & t05; \
	t16 = t11 | t12; \
	y   = t13 ^ t08; \
	x   = t15 ^ t16; \
	w   =     ~ t14; \
} while (0)
#define KBOX5(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t04, t05, t07, t08, t09; \
	uint32_t t10, t11, t12, t13, t14, t01; \
	t01 = b   ^ d  ; \
	t02 = b   | d  ; \
	t03 = a   & t01; \
	t04 = c   ^ t02; \
	t05 = t03 ^ t04; \
	w   =     ~ t05; \
	t07 = a   ^ t01; \
	t08 = d   | w  ; \
	t09 = b   | t05; \
	t10 = d   ^ t08; \
	t11 = b   | t07; \
	t12 = t03 | w  ; \
	t13 = t07 | t10; \
	t14 = t01 ^ t11; \
	y   = t09 ^ t13; \
	x   = t07 ^ t08; \
	z   = t12 ^ t14; \
} while (0)
#define KBOX6(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t04, t05, t07, t08, t09, t10;	\
	uint32_t t11, t12, t13, t15, t17, t18, t01; \
	t01 = a   & d  ; \
	t02 = b   ^ c  ; \
	t03 = a   ^ d  ; \
	t04 = t01 ^ t02; \
	t05 = b   | c  ; \
	x   =     ~ t04; \
	t07 = t03 & t05; \
	t08 = b   & x  ; \
	t09 = a   | c  ; \
	t10 = t07 ^ t08; \
	t11 = b   | d  ; \
	t12 = c   ^ t11; \
	t13 = t09 ^ t10; \
	y   =     ~ t13; \
	t15 = x   & t03; \
	z   = t12 ^ t07; \
	t17 = a   ^ b  ; \
	t18 = y   ^ t15; \
	w   = t17 ^ t18; \
} while (0)
#define KBOX7(a,b,c,d, w,x,y,z) \
do { \
	uint32_t t02, t03, t04, t05, t06, t08, t09, t10;	\
	uint32_t t11, t13, t14, t15, t16, t17, t01; \
	t01 = a   & c  ; \
	t02 =     ~ d  ; \
	t03 = a   & t02; \
	t04 = b   | t01; \
	t05 = a   & b  ; \
	t06 = c   ^ t04; \
	z   = t03 ^ t06; \
	t08 = c   | z  ; \
	t09 = d   | t05; \
	t10 = a   ^ t08; \
	t11 = t04 & z  ; \
	x   = t09 ^ t10; \
	t13 = b   ^ x  ; \
	t14 = t01 ^ x  ; \
	t15 = c   ^ t05; \
	t16 = t11 | t13; \
	t17 = t02 | t14; \
	w   = t15 ^ t17; \
	y   = a   ^ t16; \
} while (0)

#define KEY_RECURRENCE(w, k) \
do { \
	uint32_t __wn = w[((k)+0)&7] ^ w[((k)+3)&7] ^ \
					w[((k)+5)&7] ^ w[((k)+7)&7] ^ \
					(0x9e3779b9 ^ (k)); \
	w[i] = ROTL32(__wn, 11); \
} while (0)

#define KEY_EXPAND(keys, s, w, i) \
do { \
	KEY_RECURRENCE(w, ((i)*4)+0); \
	KEY_RECURRENCE(w, ((i)*4)+1); \
	KEY_RECURRENCE(w, ((i)*4)+2); \
	KEY_RECURRENCE(w, ((i)*4)+3); \
	KBOX##s(w[(((i)*4)+0)&7], w[(((i)*4)+1)&7], w[(((i)*4)+2)&7], w[(((i)*4)+3)&7], \
			keys[i][0], keys[i][1], keys[i][2], keys[i][3]); \
} while (0)

void
serpent_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length) {
	uint32_t w[8];
	unsigned i;
	assert (SERPENT_MIN_KEY_SIZE <= length && length <= SERPENT_MAX_KEY_SIZE);
	/* Read the key into w */
	for (i = 0; length >= 4; length -= 4, key += 4) {
		w[i++] = LE_READ_UINT32(key);
	}
	/* Pad the key, if necessary */
	if (i < 8) {
		uint32_t pad = 0x01;
		while (length > 0) {
			pad = (pad << 8) | ((uint32_t)key[--length]);
		}
		w[i++] = pad;
		while (i < 8) {
			w[i++] = 0;
		}
	}
	/* Expand the key */
	KEY_EXPAND(ctx->keys, 3, w,  0); KEY_EXPAND(ctx->keys, 2, w,  1);
	KEY_EXPAND(ctx->keys, 1, w,  2); KEY_EXPAND(ctx->keys, 0, w,  3);
	KEY_EXPAND(ctx->keys, 7, w,  4); KEY_EXPAND(ctx->keys, 6, w,  5);
	KEY_EXPAND(ctx->keys, 5, w,  6); KEY_EXPAND(ctx->keys, 4, w,  7);

	KEY_EXPAND(ctx->keys, 3, w,  8); KEY_EXPAND(ctx->keys, 2, w,  9);
	KEY_EXPAND(ctx->keys, 1, w, 10); KEY_EXPAND(ctx->keys, 0, w, 11);
	KEY_EXPAND(ctx->keys, 7, w, 12); KEY_EXPAND(ctx->keys, 6, w, 13);
	KEY_EXPAND(ctx->keys, 5, w, 14); KEY_EXPAND(ctx->keys, 4, w, 15);

	KEY_EXPAND(ctx->keys, 3, w, 16); KEY_EXPAND(ctx->keys, 2, w, 17);
	KEY_EXPAND(ctx->keys, 1, w, 18); KEY_EXPAND(ctx->keys, 0, w, 19);
	KEY_EXPAND(ctx->keys, 7, w, 20); KEY_EXPAND(ctx->keys, 6, w, 21);
	KEY_EXPAND(ctx->keys, 5, w, 22); KEY_EXPAND(ctx->keys, 4, w, 23);

	KEY_EXPAND(ctx->keys, 3, w, 24); KEY_EXPAND(ctx->keys, 2, w, 25);
	KEY_EXPAND(ctx->keys, 1, w, 26); KEY_EXPAND(ctx->keys, 0, w, 27);
	KEY_EXPAND(ctx->keys, 7, w, 28); KEY_EXPAND(ctx->keys, 6, w, 29);
	KEY_EXPAND(ctx->keys, 5, w, 30); KEY_EXPAND(ctx->keys, 4, w, 31);

	KEY_EXPAND(ctx->keys, 3, w, 32);
}

void
serpent128_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == SERPENT128_KEY_SIZE);
	serpent_set_key(ctx, key, length);
}

void
serpent192_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == SERPENT192_KEY_SIZE);
	serpent_set_key(ctx, key, length);
}

void
serpent256_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == SERPENT256_KEY_SIZE);
	serpent_set_key(ctx, key, length);
}

void
serpent_encrypt(const struct serpent_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	_serpent_encrypt(ctx->keys, dst, src, length);
}

void
serpent_decrypt(const struct serpent_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	_serpent_decrypt(ctx->keys, dst, src, length);
}

#if __IS_x86__ && CRYPTO_FAT

static void
_crypto_serpent_encrypt_default(const uint32_t (*keys)[4], uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_avx()) {
			_crypto_serpent_encrypt_fat = &_serpent_encrypt_avx;
		} else {
			_crypto_serpent_encrypt_fat = &_serpent_encrypt_x64;
		}
	} else {
		_crypto_serpent_encrypt_fat = &_serpent_encrypt_gen;
	}
	_serpent_encrypt(keys, dst, src, length);
}

static void
_crypto_serpent_decrypt_default(const uint32_t (*keys)[4], uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_avx()) {
			_crypto_serpent_decrypt_fat = &_serpent_decrypt_avx;
		} else {
			_crypto_serpent_decrypt_fat = &_serpent_decrypt_x64;
		}
	} else {
		_crypto_serpent_decrypt_fat = &_serpent_decrypt_gen;
	}
	_serpent_decrypt(keys, dst, src, length);
}

void _serpent_encrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t)
		= &_crypto_serpent_encrypt_default;
void _serpent_decrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t)
		= &_crypto_serpent_decrypt_default;

#else

void _serpent_encrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t)
		= &_serpent_encrypt_gen;
void _serpent_decrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t)
		= &_serpent_encrypt_gen;

#endif
