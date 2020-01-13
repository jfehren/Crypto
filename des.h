/*
 * des.h
 *
 *  Created on: Feb 18, 2019, 4:37:23 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef DES_H_
#define DES_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Namespace mangling */
#define des_set_key crypto_des_set_key
#define des_encrypt crypto_des_encrypt
#define des_decrypt crypto_des_decrypt
#define des_check_parity crypto_des_check_parity
#define des_fix_parity crypto_des_fix_parity

#define des3_set_key crypto_des3_set_key
#define des3_encrypt crypto_des3_encrypt
#define des3_decrypt crypto_des3_decrypt

#define DES_KEY_SIZE 8
#define DES_BLOCK_SIZE 8

/* Expanded key length */
#define _DES_KEY_LENGTH 32

struct des_ctx {
	uint32_t key[_DES_KEY_LENGTH];
};

/* Returns 1 for good keys and 0 for weak keys. */
__CRYPTO_DECLSPEC int
des_set_key(struct des_ctx *ctx, const uint8_t *key, size_t key_length);

__CRYPTO_DECLSPEC void
des_encrypt(const struct des_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
des_decrypt(const struct des_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC int
des_check_parity(const uint8_t *key, size_t length);

__CRYPTO_DECLSPEC void
des_fix_parity(uint8_t *dst, const uint8_t *src, size_t length);

#define DES3_KEY_SIZE 24
#define DES3_BLOCK_SIZE DES_BLOCK_SIZE

struct des3_ctx {
	struct des_ctx des[3];
};


/* Returns 1 for good keys and 0 for weak keys. */
__CRYPTO_DECLSPEC int
des3_set_key(struct des3_ctx *ctx, const uint8_t *key, size_t key_length);

__CRYPTO_DECLSPEC void
des3_encrypt(const struct des3_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
des3_decrypt(const struct des3_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);


#ifdef __cplusplus
}
#endif

#endif /* DES_H_ */
