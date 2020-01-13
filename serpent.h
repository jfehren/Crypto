/*
 * serpent.h
 *
 *  Created on: Feb 18, 2019, 8:38:13 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef SERPENT_H_
#define SERPENT_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define serpent_set_key crypto_serpent_set_key
#define serpent128_set_key crypto_serpent128_set_key
#define serpent192_set_key crypto_serpent192_set_key
#define serpent256_set_key crypto_serpent256_set_key
#define serpent_encrypt crypto_serpent_encrypt
#define serpent_decrypt crypto_serpent_decrypt

#define SERPENT_BLOCK_SIZE 16

/* Other key lengths are possible, but the design of Serpent makes
 * smaller key lengths quite pointless; they cheated with the AES
 * requirements, using a 256-bit key length exclusively and just
 * padding it out if the desired key length was less, so there really
 * is no advantage to using key lengths less than 256 bits. */
#define SERPENT_KEY_SIZE 32

/* Allow keys of size 128 <= bits <= 256 */

#define SERPENT_MIN_KEY_SIZE 16
#define SERPENT_MAX_KEY_SIZE 32

#define SERPENT128_KEY_SIZE 16
#define SERPENT192_KEY_SIZE 24
#define SERPENT256_KEY_SIZE 32

struct serpent_ctx
{
  uint32_t keys[33][4] ATTRIBUTE(aligned(16));  /* key schedule */
};

__CRYPTO_DECLSPEC void
serpent_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
serpent128_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
serpent192_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
serpent256_set_key(struct serpent_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
serpent_encrypt(const struct serpent_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
serpent_decrypt(const struct serpent_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SERPENT_H_ */
