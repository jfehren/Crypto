/*
 * cmac.h
 *
 *  Created on: Sep 25, 2019, 11:08:22 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef CMAC_H_
#define CMAC_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* name mangling */
#define cmac_init crypto_cmac_init
#define cmac_free crypto_cmac_free

#define cmac_set_key crypto_cmac_set_key
#define cmac_update crypto_cmac_update
#define cmac_digest crypto_cmac_digest

#define cmac_get_digest_size crypto_cmac_get_digest_size
#define cmac_get_block_size crypto_cmac_get_block_size
#define cmac_get_key_size crypto_cmac_get_key_size

struct cmac_ctx {
	unsigned block_size;	/* Input block length and Output digest length */
	unsigned key_size;		/* Cipher key size */
	unsigned cipher_size;	/* Cipher context size */
	unsigned index;			/* index into buffer */

	void *ctx;		/* Cipher context */
	uint8_t *buf;	/* CMAC internal buffer */
	uint8_t *k1;	/* K1, generated from cipher key K */
	uint8_t *k2;	/* K2, generated from cipher key K */

	crypto_set_key_func *set_key;	/* Function to set cipher key */
	crypto_cipher_func *crypt;		/* Cipher's encryption function */
};

__CRYPTO_DECLSPEC void
cmac_init(struct cmac_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cmac_free(struct cmac_ctx *ctx);

__CRYPTO_DECLSPEC void
cmac_set_key(struct cmac_ctx *ctx, const uint8_t *key, size_t key_length);

__CRYPTO_DECLSPEC void
cmac_update(struct cmac_ctx *ctx, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
cmac_digest(struct cmac_ctx *ctx, uint8_t *dst, size_t length);

__CRYPTO_DECLSPEC unsigned
cmac_get_digest_size(const struct hmac_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
cmac_get_block_size(const struct hmac_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
cmac_get_key_size(const struct cmac_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* CMAC_H_ */
