/*
 * cbc.h
 *
 *  Created on: Mar 5, 2019, 2:40:26 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CBC_H_
#define CBC_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define cbc_init crypto_cbc_init
#define cbc_free crypto_cbc_free
#define cbc_set_encrypt_key crypto_cbc_set_encrypt_key
#define cbc_set_decrypt_key crypto_cbc_set_decrypt_key
#define cbc_encrypt crypto_cbc_encrypt
#define cbc_decrypt crypto_cbc_decrypt

#define cbc_get_block_size crypto_cbc_get_block_size
#define cbc_get_key_size crypto_cbc_get_key_size

struct cbc_ctx {
	unsigned block_size;
	unsigned key_size;
	unsigned cipher_size;
	void *cipher_ctx;
	uint8_t *iv;

	crypto_set_key_func *set_encrypt_key;
	crypto_set_key_func *set_decrypt_key;

	crypto_cipher_func *encrypt;
	crypto_cipher_func *decrypt;
};


__CRYPTO_DECLSPEC void
cbc_init(struct cbc_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cbc_free(struct cbc_ctx *ctx);

__CRYPTO_DECLSPEC void
cbc_set_encrypt_key(struct cbc_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
cbc_set_decrypt_key(struct cbc_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
cbc_encrypt(struct cbc_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
cbc_decrypt(struct cbc_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC unsigned
cbc_get_block_size(const struct cbc_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
cbc_get_key_size(const struct cbc_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* CBC_H_ */
