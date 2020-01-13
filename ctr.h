/*
 * ctr.h
 *
 *  Created on: Mar 6, 2019, 3:16:03 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CTR_H_
#define CTR_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define ctr_init crypto_ctr_init
#define ctr_free crypto_ctr_free
#define ctr_set_key crypto_ctr_set_key
#define ctr_encrypt crypto_ctr_encrypt

#define ctr_get_block_size crypto_ctr_get_block_size
#define ctr_get_key_size crypto_ctr_get_key_size

struct ctr_ctx {
	unsigned block_size;
	unsigned key_size;
	unsigned cipher_size;
	void *cipher_ctx;
	uint8_t *ctr;

	crypto_set_key_func *set_key;
	crypto_cipher_func *encrypt;
};

__CRYPTO_DECLSPEC void
ctr_init(struct ctr_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
ctr_free(struct ctr_ctx *ctx);

__CRYPTO_DECLSPEC void
ctr_set_key(struct ctr_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *ctr, size_t ctr_length);

__CRYPTO_DECLSPEC void
ctr_encrypt(struct ctr_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC unsigned
ctr_get_block_size(const struct ctr_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ctr_get_key_size(const struct ctr_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* CTR_H_ */
