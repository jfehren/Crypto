/*
 * twofish.h
 *
 *  Created on: Mar 4, 2019, 2:24:41 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef TWOFISH_H_
#define TWOFISH_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define twofish_set_key crypto_twofish_set_key
#define twofish128_set_key crypto_twofish128_set_key
#define twofish192_set_key crypto_twofish192_set_key
#define twofish256_set_key crypto_twofish256_set_key
#define twofish_encrypt crypto_twofish_encrypt
#define twofish_decrypt crypto_twofish_decrypt

#define TWOFISH_BLOCK_SIZE 16

/* Variable key size between 128 and 256 bits. But the only valid
 * values are 16 (128 bits), 24 (192 bits) and 32 (256 bits). */
#define TWOFISH_MIN_KEY_SIZE 16
#define TWOFISH_MAX_KEY_SIZE 32

#define TWOFISH_KEY_SIZE 32
#define TWOFISH128_KEY_SIZE 16
#define TWOFISH192_KEY_SIZE 24
#define TWOFISH256_KEY_SIZE 32

struct twofish_ctx
{
	uint32_t keys[40];
	uint32_t s_box[4][256];
};

__CRYPTO_DECLSPEC void
twofish_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
twofish128_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
twofish192_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
twofish256_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
twofish_encrypt(const struct twofish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
twofish_decrypt(const struct twofish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* TWOFISH_H_ */
