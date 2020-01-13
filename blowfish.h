/*
 * blowfish.h
 *
 *  Created on: Feb 18, 2019, 12:58:11 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef BLOWFISH_H_
#define BLOWFISH_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define blowfish_set_key crypto_blowfish_set_key
#define blowfish128_set_key crypto_blowfish128_set_key
#define blowfish_encrypt crypto_blowfish_encrypt
#define blowfish_decrypt crypto_blowfish_decrypt

#define BLOWFISH_BLOCK_SIZE 8

/* Variable key size between 64 and 448 bits. */
#define BLOWFISH_MIN_KEY_SIZE 8
#define BLOWFISH_MAX_KEY_SIZE 56

/* Default to 128 bits */
#define BLOWFISH_KEY_SIZE 16

#define BLOWFISH128_KEY_SIZE 16

#define _BLOWFISH_ROUNDS 16

struct blowfish_ctx {
	uint32_t s[4][256];
	uint32_t p[_BLOWFISH_ROUNDS+2];
};

/* Returns 0 for weak keys, 1 otherwise */
__CRYPTO_DECLSPEC int
blowfish_set_key(struct blowfish_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC int
blowfish128_set_key(struct blowfish_ctx *ctx, const uint8_t *key);

__CRYPTO_DECLSPEC void
blowfish_encrypt(const struct blowfish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
blowfish_decrypt(const struct blowfish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* BLOWFISH_H_ */
