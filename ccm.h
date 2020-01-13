/*
 * ccm.h
 *
 *  Created on: Mar 7, 2019, 1:26:55 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CCM_H_
#define CCM_H_

#include "crypto.h"
#include "ctr.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ccm_init crypto_ccm_init
#define ccm_free crypto_ccm_free
#define ccm_set_key_default crypto_ccm_set_key_default
#define ccm_set_key crypto_ccm_set_key
#define ccm_update crypto_ccm_update
#define ccm_digest crypto_ccm_digest
#define ccm_encrypt crypto_ccm_encrypt
#define ccm_decrypt crypto_ccm_decrypt
#define ccm_encrypt_full crypto_ccm_encrypt_full
#define ccm_decrypt_full crypto_ccm_decrypt_full

#define ccm_get_block_size crypto_ccm_get_block_size
#define ccm_get_key_size crypto_ccm_get_key_size
#define ccm_get_nonce_size crypto_ccm_get_nonce_size
#define ccm_get_digest_size crypto_ccm_get_digest_size

#define CCM_BLOCK_SIZE 16
#define CCM_MAX_DIGEST_SIZE 16
#define CCM_MIN_NONCE_SIZE 7
#define CCM_MAX_NONCE_SIZE 13

/* Maximum plaintext message size, as a function of the nonce size n.
 * The length field is q octets, with q = 15 - n, and then the maximum
 * size M = 2^{8q} - 1. */
#define CCM_MAX_MSG_SIZE(n)	\
	((sizeof(size_t) + (n) <= 15) ? ~(size_t) 0 : ((size_t) 1 << (8*(15 - n))) - 1)

struct ccm_ctx {
	struct ctr_ctx ctr_ctx;
	union crypto_block16 tag;
	unsigned blength;
	unsigned digest_size;
};

__CRYPTO_DECLSPEC void
ccm_init(struct ccm_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
ccm_free(struct ccm_ctx *ctx);

__CRYPTO_DECLSPEC void
ccm_set_key_default(struct ccm_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *nonce, size_t nonce_length);

__CRYPTO_DECLSPEC void
ccm_set_key(struct ccm_ctx *ctx, const uint8_t *key, size_t key_length, const uint8_t *nonce,
		size_t nonce_length, size_t digest_length, size_t message_length, size_t aad_length);

__CRYPTO_DECLSPEC void
ccm_update(struct ccm_ctx *ctx, const uint8_t *aad, size_t length);

__CRYPTO_DECLSPEC void
ccm_digest(struct ccm_ctx *ctx, const uint8_t *digest);

__CRYPTO_DECLSPEC void
ccm_encrypt(struct ccm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
ccm_decrypt(struct ccm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC int
ccm_encrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length);

__CRYPTO_DECLSPEC int
ccm_decrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length);


__CRYPTO_DECLSPEC unsigned
ccm_get_block_size(const struct ccm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ccm_get_key_size(const struct ccm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ccm_get_nonce_size(const struct ccm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
ccm_get_digest_size(const struct ccm_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* CCM_H_ */
