/*
 * gcm.h
 *
 *  Created on: Mar 8, 2019, 9:32:13 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef GCM_H_
#define GCM_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define gcm_init crypto_gcm_init
#define gcm_free crypto_gcm_free
#define gcm_set_key_default crypto_gcm_set_key_default
#define gcm_set_key crypto_gcm_set_key
#define gcm_update crypto_gcm_update
#define gcm_digest crypto_gcm_digest
#define gcm_encrypt crypto_gcm_encrypt
#define gcm_decrypt crypto_gcm_decrypt
#define gcm_encrypt_full crypto_gcm_encrypt_full
#define gcm_decrypt_full crypto_gcm_decrypt_full

#define gcm_get_block_size crypto_gcm_get_block_size
#define gcm_get_key_size crypto_gcm_get_key_size
#define gcm_get_nonce_size crypto_gcm_get_nonce_size
#define gcm_get_digest_size crypto_gcm_get_digest_size

#define GCM_BLOCK_SIZE 16
#define GCM_TABLE_BITS 8
#define GCM_IV_SIZE (GCM_BLOCK_SIZE - 4)
#define GCM_MAX_DIGEST_SIZE GCM_BLOCK_SIZE
#define GCM_MIN_DIGEST_SIZE (GCM_BLOCK_SIZE - 4)

/* Per-message state, depending on the iv */
struct gcm_ctx {
	/* Original counter block */
	union crypto_block16 iv;
	/* Updated for each block. */
	union crypto_block16 ctr;
	/* Hashing buffer */
	uint8_t buf[GCM_BLOCK_SIZE];
	/* Hashing state */
	union crypto_block16 *hash;
	unsigned blength;	/* number of bytes in x not yet processed */
	uint64_t auth_size;
	uint64_t data_size;
	unsigned digest_size;
	unsigned cipher_size;
	unsigned key_size;
	void *cipher_ctx;

	crypto_set_key_func *set_key;
	crypto_cipher_func *encrypt;
};

__CRYPTO_DECLSPEC void
gcm_init(struct gcm_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
gcm_free(struct gcm_ctx *ctx);

__CRYPTO_DECLSPEC void
gcm_set_key_default(struct gcm_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *nonce, size_t nonce_length);

__CRYPTO_DECLSPEC void
gcm_set_key(struct gcm_ctx *ctx, const uint8_t *key, size_t key_length, const uint8_t *nonce,
		size_t nonce_length, size_t aad_length, size_t digest_length, size_t message_length);

__CRYPTO_DECLSPEC void
gcm_update(struct gcm_ctx *ctx, const uint8_t *aad, size_t length);

__CRYPTO_DECLSPEC void
gcm_digest(struct gcm_ctx *ctx, const uint8_t *digest);

__CRYPTO_DECLSPEC void
gcm_encrypt(struct gcm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
gcm_decrypt(struct gcm_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC int
gcm_encrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length);

__CRYPTO_DECLSPEC int
gcm_decrypt_full(const struct crypto_cipher_meta *cipher_meta, const uint8_t *key,
		size_t key_length, uint8_t *dst, const uint8_t *src, size_t msg_length,
		size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length);


__CRYPTO_DECLSPEC unsigned
gcm_get_block_size(const struct gcm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
gcm_get_key_size(const struct gcm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
gcm_get_nonce_size(const struct gcm_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
gcm_get_digest_size(const struct gcm_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* GCM_H_ */
