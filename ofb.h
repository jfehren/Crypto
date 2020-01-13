/*
 * ofb.h
 *
 *  Created on: Mar 6, 2019, 1:39:16 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef OFB_H_
#define OFB_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define ofb_init crypto_ofb_init
#define ofb_free crypto_ofb_free
#define ofb_set_key crypto_ofb_set_key
#define ofb_encrypt crypto_ofb_encrypt

#define ofb_get_block_size crypto_ofb_get_block_size
#define ofb_get_key_size crypto_ofb_get_key_size

struct ofb_ctx {
	unsigned block_size;
	unsigned key_size;
	unsigned cipher_size;
	void *cipher_ctx;
	uint8_t *iv;

	crypto_set_key_func *set_key;
	crypto_cipher_func *encrypt;
};

__CRYPTO_DECLSPEC void
ofb_init(struct ofb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
ofb_free(struct ofb_ctx *ctx);

__CRYPTO_DECLSPEC void
ofb_set_key(struct ofb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
ofb_encrypt(struct ofb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC unsigned
ofb_get_block_size(const struct ofb_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ofb_get_key_size(const struct ofb_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* OFB_H_ */
