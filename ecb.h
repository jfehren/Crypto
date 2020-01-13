/*
 * ecb.h
 *
 *  Created on: Sep 3, 2019, 12:24:26 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef ECB_H_
#define ECB_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ecb_init crypto_ecb_init
#define ecb_free crypto_ecb_free
#define ecb_set_encrypt_key crypto_ecb_set_encrypt_key
#define ecb_set_decrypt_key crypto_ecb_set_decrypt_key
#define ecb_encrypt crypto_ecb_encrypt
#define ecb_decrypt crypto_ecb_decrypt

#define ecb_get_block_size crypto_ecb_get_block_size
#define ecb_get_key_size crypto_ecb_get_key_size
#define ecb_get_iv_size crypto_ecb_get_iv_size

struct ecb_ctx {
	unsigned block_size;
	unsigned key_size;
	unsigned cipher_size;
	void *cipher_ctx;

	crypto_set_key_func *set_encrypt_key;
	crypto_set_key_func *set_decrypt_key;

	crypto_cipher_func *encrypt;
	crypto_cipher_func *decrypt;
};


__CRYPTO_DECLSPEC void
ecb_init(struct ecb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
ecb_free(struct ecb_ctx *ctx);

__CRYPTO_DECLSPEC void
ecb_set_encrypt_key(struct ecb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
ecb_set_decrypt_key(struct ecb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
ecb_encrypt(struct ecb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
ecb_decrypt(struct ecb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC unsigned
ecb_get_block_size(const struct ecb_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ecb_get_key_size(const struct ecb_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ecb_get_iv_size(const struct ecb_ctx *ctx) ATTRIBUTE(const);

#ifdef __cplusplus
}
#endif

#endif /* ECB_H_ */
