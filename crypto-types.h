/*
 * crypto-types.h
 *
 *  Created on: Sep 11, 2019, 3:06:52 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_TYPES_H_
#define CRYPTO_TYPES_H_

#include <stdint.h>
#include <stdlib.h>

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

union crypto_block16 {
	uint_least8_t  b[16 / sizeof(uint_least8_t)]  ATTRIBUTE(aligned(16));
	uint_least16_t w[16 / sizeof(uint_least16_t)] ATTRIBUTE(aligned(16));
	uint_least32_t d[16 / sizeof(uint_least32_t)] ATTRIBUTE(aligned(16));
	uint_least64_t q[16 / sizeof(uint_least64_t)] ATTRIBUTE(aligned(16));
};

/* Random bit generator function */
typedef void crypto_random_func(void *ctx, uint8_t *dst, size_t length);

/* Progress report function, mainly for key generation. */
typedef void crypto_progress_func(void *ctx, int c);

/* Realloc function, used by struct crypto_buffer. Length of 0 means free memory */
typedef void *crypto_realloc_func(void *ctx, void *p, size_t length);

/* Cipher key initialization function */
typedef void crypto_set_key_func(void *ctx, const uint8_t *key, size_t length);

/* Cipher encryption/decryption function with constant state */
typedef void crypto_cipher_func(const void *ctx, uint8_t *dst, const uint8_t *src, size_t length);

/* Cipher encryption/decryption function with an internal state
 * that changes during operation, such as a cipher mode function */
typedef void crypto_crypt_func(void *ctx, uint8_t *dst, const uint8_t *src, size_t length);

struct crypto_cipher_meta {
	const char *name;		/* Name of this cipher */
	unsigned context_size;	/* Size of context */
	unsigned context_align;	/* Alignment of context */
	unsigned block_size;	/* Size of input block - zero for stream ciphers */
	unsigned key_size;		/* Suggested key size; other sizes may be possible */

	crypto_set_key_func *set_encrypt_key;	/* Initialize cipher for encryption */
	crypto_set_key_func *set_decrypt_key;	/* Initialize cipher for decryption */

	crypto_cipher_func *encrypt;	/* Encrypt input block(s) */
	crypto_cipher_func *decrypt;	/* Decrypt input block(s) */
};


/* Cipher mode context initialization function */
typedef void crypto_mode_init_func(void *mode_ctx,
		const struct crypto_cipher_meta *cipher_meta);

/* Cipher mode context free function */
typedef void crypto_mode_free_func(void *mode_ctx);

/* Cipher mode key initial vector initialization function */
typedef void crypto_mode_set_key_func(void *mode_ctx, const uint8_t *key,
		size_t key_length, const uint8_t *iv, size_t iv_length);

/* Cipher mode member value retrieval function */
typedef unsigned crypto_mode_get_func(const void *mode_ctx);

struct crypto_cipher_mode_meta {
	const char *name;	/* Name of this cipher mode */
	unsigned context_size;	/* Size of this cipher mode's context */

	crypto_mode_init_func *init_mode_ctx;	/* Function to initialize the
											 * mode's context according to
											 * the given cipher's meta data */
	crypto_mode_free_func *free_mode_ctx;	/* Function to free the internal
											 * data allocated to the cipher
											 * mode's context */

	crypto_mode_set_key_func *set_encrypt_key;	/* Initialize cipher mode for encryption */
	crypto_mode_set_key_func *set_decrypt_key;	/* Initialize cipher mode for decryption */

	crypto_crypt_func *encrypt;		/* Encrypt input block(s) */
	crypto_crypt_func *decrypt;		/* Decrypt input block(s) */

	crypto_mode_get_func *get_block_size;	/* Get this cipher mode's input block size */
	crypto_mode_get_func *get_key_size;		/* Get this cipher mode's suggested key size */
	crypto_mode_get_func *get_iv_size;		/* Get this cipher mode's IV size */
};


/* AEAD Cipher mode key and nonce initialization function */
typedef void crypto_aead_mode_set_key_func(void *mode_ctx, const uint8_t *key,
		size_t key_length, const uint8_t *nonce, size_t nonce_length, size_t digest_length,
		size_t message_length, size_t aad_length);

/* AEAD Cipher mode update function */
typedef void crypto_aead_mode_update_func(void *mode_ctx, const uint8_t *aad, size_t length);

/* AEAD Cipher mode digest function */
typedef void crypto_aead_mode_digest_func(void *mode_ctx, uint8_t *dst);

/* AEAD Cipher mode full encryption/decryption function; return value is undefined for
 * encryption, and for decryption a nonzero return value indicates that the digest produced
 * by decryption matched the input digest, with a zero return value otherwise. */
typedef int crypto_aead_mode_full_crypt_func(const struct crypto_cipher_meta *cipher_meta,
		const uint8_t *key, size_t key_length, uint8_t *dst, const uint8_t *src,
		size_t msg_length, size_t digest_length, const uint8_t *nonce, size_t nonce_length,
		const uint8_t *aad, size_t aad_length);

struct crypto_aead_cipher_mode_meta {
	struct crypto_cipher_mode_meta mode_meta;	/* Info shared with non-aead cipher mode's */

	crypto_aead_mode_set_key_func *aead_set_encrypt_key;	/* Initialize AEAD for encryption */
	crypto_aead_mode_set_key_func *aead_set_decrypt_key;	/* Initialize AEAD for decryption */

	crypto_aead_mode_update_func *aad_update;	/* Update the Additional Authentication Data */

	crypto_aead_mode_digest_func *digest;	/* Output the verification tag after encryption */

	crypto_aead_mode_full_crypt_func *encrypt_full;	/* Perform a full encryption */
	crypto_aead_mode_full_crypt_func *decrypt_full;	/* Perform a full decryption */

	crypto_mode_get_func *get_digest_length;	/* Get verification tag length */
};


/* Hash algorithm initialization function */
typedef void crypto_hash_init_func(void *ctx);

/* Hash algorithm update function */
typedef void crypto_hash_update_func(void *ctx, const uint8_t *src, size_t length);

/* Hash algorithm finalization and output function */
typedef void crypto_hash_digest_func(void *ctx, uint8_t *dst, size_t length);

struct crypto_hash_meta {
	const char *name;	/* Name of this hash */
	unsigned context_size;	/* Size of this hash's context */
	unsigned digest_size;	/* Length of this hash's output */
	unsigned block_size;	/* Size of this hash's input block */

	crypto_hash_init_func *initialize;	/* Initialize hash */
	crypto_hash_update_func *update;	/* Update hash */
	crypto_hash_digest_func *finalize;	/* Finalize hash and output message digest */
};


enum sexp_type { SEXP_ATOM, SEXP_LIST, SEXP_END };

/* Parsing sexp objects */
struct sexp_iterator {
	size_t length;
	const uint8_t *buffer;

	/* Points at the start of the current sub expression. */
	size_t start;
	/* If type is SEXP_LIST, pos points at the start of the current
	 * element. Otherwise, it points at the end. */
	size_t pos;
	unsigned level;

	enum sexp_type type;

	size_t display_length;
	const uint8_t *display;

	size_t atom_length;
	const uint8_t *atom;
};


#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_TYPES_H_ */
