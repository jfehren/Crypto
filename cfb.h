/*
 * cfb.h
 *
 *  Created on: Mar 6, 2019, 9:45:44 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef CFB_H_
#define CFB_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define cfb_init crypto_cfb_init
#define cfb_free crypto_cfb_free
#define cfb_set_key crypto_cfb_set_key
#define cfb_encrypt crypto_cfb_encrypt
#define cfb_decrypt crypto_cfb_decrypt

#define cfb_get_block_size crypto_cfb_get_block_size
#define cfb_get_key_size crypto_cfb_get_key_size
#define cfb_get_iv_size crypto_cfb_get_iv_size


#define cfb8_init	crypto_cfb8_init
#define cfb16_init	crypto_cfb16_init
#define cfb24_init	crypto_cfb24_init
#define cfb32_init	crypto_cfb32_init
#define cfb40_init	crypto_cfb40_init
#define cfb48_init	crypto_cfb48_init
#define cfb56_init	crypto_cfb56_init
#define cfb64_init	crypto_cfb64_init
#define cfb72_init	crypto_cfb72_init
#define cfb80_init	crypto_cfb80_init
#define cfb88_init	crypto_cfb88_init
#define cfb96_init	crypto_cfb96_init
#define cfb104_init	crypto_cfb104_init
#define cfb112_init	crypto_cfb112_init
#define cfb120_init	crypto_cfb120_init
#define cfb128_init	crypto_cfb128_init

/* These are theoretical and currently have no use. */
/* Attempting to use them will result in an assertion failure for any cipher
 * with a block size less than the feedback size of the associated CFB version */
#define cfb136_init	crypto_cfb136_init
#define cfb144_init	crypto_cfb144_init
#define cfb152_init	crypto_cfb152_init
#define cfb160_init	crypto_cfb160_init
#define cfb168_init	crypto_cfb168_init
#define cfb176_init	crypto_cfb176_init
#define cfb184_init	crypto_cfb184_init
#define cfb192_init	crypto_cfb192_init
#define cfb200_init	crypto_cfb200_init
#define cfb208_init	crypto_cfb208_init
#define cfb216_init	crypto_cfb216_init
#define cfb224_init	crypto_cfb224_init
#define cfb232_init	crypto_cfb232_init
#define cfb240_init	crypto_cfb240_init
#define cfb248_init	crypto_cfb248_init
#define cfb256_init	crypto_cfb256_init

struct cfb_ctx {
	unsigned cipher_block_size;
	unsigned block_size;
	unsigned key_size;
	unsigned cipher_size;
	void *cipher_ctx;
	uint8_t *iv;

	crypto_set_key_func *set_key;
	crypto_cipher_func *encrypt;
};


__CRYPTO_DECLSPEC void
cfb_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb_free(struct cfb_ctx *ctx);

__CRYPTO_DECLSPEC void
cfb_set_key(struct cfb_ctx *ctx, const uint8_t *key, size_t key_length,
		const uint8_t *iv, size_t iv_length);

__CRYPTO_DECLSPEC void
cfb_encrypt(struct cfb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
cfb_decrypt(struct cfb_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC unsigned
cfb_get_block_size(const struct cfb_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
cfb_get_key_size(const struct cfb_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
cfb_get_iv_size(const struct cfb_ctx *ctx) ATTRIBUTE(pure);


__CRYPTO_DECLSPEC void
cfb8_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb16_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb24_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb32_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb40_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb48_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb56_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb64_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb72_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb80_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb88_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb96_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb104_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb112_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb120_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb128_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);


/* These are theoretical and currently have no use. */
/* Attempting to use them will result in an assertion failure for any cipher
 * with a block size less than the feedback size of the associated CFB version */
__CRYPTO_DECLSPEC void
cfb136_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb144_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb152_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb160_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb168_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb176_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb184_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb192_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb200_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb208_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb216_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb224_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb232_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb240_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb248_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);

__CRYPTO_DECLSPEC void
cfb256_init(struct cfb_ctx *ctx, const struct crypto_cipher_meta *cipher_meta);


#ifdef __cplusplus
}
#endif

#endif /* CFB_H_ */
