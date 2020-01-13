/*
 * aes.h
 *
 *  Created on: Feb 11, 2019, 12:54:09 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef AES_H_
#define AES_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define aes_set_encrypt_key crypto_aes_set_encrypt_key
#define aes_set_decrypt_key crypto_aes_set_decrypt_key
#define aes_invert_key crypto_aes_invert_key
#define aes_encrypt crypto_aes_encrypt
#define aes_decrypt crypto_aes_decrypt

#define aes128_set_encrypt_key crypto_aes128_set_encrypt_key
#define aes128_set_decrypt_key crypto_aes128_set_decrypt_key
#define aes128_invert_key crypto_aes128_invert_key
#define aes128_encrypt crypto_aes128_encrypt
#define aes128_decrypt crypto_aes128_decrypt

#define aes192_set_encrypt_key crypto_aes192_set_encrypt_key
#define aes192_set_decrypt_key crypto_aes192_set_decrypt_key
#define aes192_invert_key crypto_aes192_invert_key
#define aes192_encrypt crypto_aes192_encrypt
#define aes192_decrypt crypto_aes192_decrypt

#define aes256_set_encrypt_key crypto_aes256_set_encrypt_key
#define aes256_set_decrypt_key crypto_aes256_set_decrypt_key
#define aes256_invert_key crypto_aes256_invert_key
#define aes256_encrypt crypto_aes256_encrypt
#define aes256_decrypt crypto_aes256_decrypt

#define AES_BLOCK_SIZE 16

#define AES128_KEY_SIZE 16
#define AES192_KEY_SIZE 24
#define AES256_KEY_SIZE 32
#define _AES128_ROUNDS 10
#define _AES192_ROUNDS 12
#define _AES256_ROUNDS 14

/* Variable key size between 128 and 256 bits. But the only valid
 * values are 16 (128 bits), 24 (192 bits) and 32 (256 bits). */
#define AES_MIN_KEY_SIZE AES128_KEY_SIZE
#define AES_MAX_KEY_SIZE AES256_KEY_SIZE

#define AES_KEY_SIZE 32

struct aes_ctx {
	uint32_t keys[4*(_AES256_ROUNDS + 1)] ATTRIBUTE(aligned(16));
	unsigned rounds;
};

__CRYPTO_DECLSPEC void
aes_set_encrypt_key(struct aes_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes_set_decrypt_key(struct aes_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes_invert_key(struct aes_ctx *dst, const struct aes_ctx *src);

__CRYPTO_DECLSPEC void
aes_encrypt(const struct aes_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
aes_decrypt(const struct aes_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);


struct aes128_ctx {
	uint32_t keys[4*(_AES128_ROUNDS + 1)] ATTRIBUTE(aligned(16));
};

__CRYPTO_DECLSPEC void
aes128_set_encrypt_key(struct aes128_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes128_set_decrypt_key(struct aes128_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes128_invert_key(struct aes128_ctx *dst, const struct aes128_ctx *src);

__CRYPTO_DECLSPEC void
aes128_encrypt(const struct aes128_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
aes128_decrypt(const struct aes128_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);


struct aes192_ctx {
	uint32_t keys[4*(_AES192_ROUNDS + 1)] ATTRIBUTE(aligned(16));
};

__CRYPTO_DECLSPEC void
aes192_set_encrypt_key(struct aes192_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes192_set_decrypt_key(struct aes192_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes192_invert_key(struct aes192_ctx *dst, const struct aes192_ctx *src);

__CRYPTO_DECLSPEC void
aes192_encrypt(const struct aes192_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
aes192_decrypt(const struct aes192_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);


struct aes256_ctx {
	uint32_t keys[4*(_AES256_ROUNDS + 1)] ATTRIBUTE(aligned(16));
};

__CRYPTO_DECLSPEC void
aes256_set_encrypt_key(struct aes256_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes256_set_decrypt_key(struct aes256_ctx *ctx, const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
aes256_invert_key(struct aes256_ctx *dst, const struct aes256_ctx *src);

__CRYPTO_DECLSPEC void
aes256_encrypt(const struct aes256_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
aes256_decrypt(const struct aes256_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* AES_H_ */
