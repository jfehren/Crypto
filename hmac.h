/*
 * hmac.h
 *
 *  Created on: Sep 3, 2019, 2:15:49 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef HMAC_H_
#define HMAC_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* name mangling */
#define hmac_init crypto_hmac_init
#define hmac_free crypto_hmac_free

#define hmac_set_key crypto_hmac_set_key
#define hmac_update crypto_hmac_update
#define hmac_digest crypto_hmac_digest

#define hmac_get_digest_size crypto_hmac_get_digest_size
#define hmac_get_block_size crypto_hmac_get_block_size

struct hmac_ctx {
	unsigned digest_size;
	unsigned block_size;
	unsigned context_size;
	void *outer;
	void *inner;
	void *state;

	crypto_hash_init_func *initialize;
	crypto_hash_update_func *update;
	crypto_hash_digest_func *finalize;
};

__CRYPTO_DECLSPEC void
hmac_init(struct hmac_ctx *ctx, const struct crypto_hash_meta *hash_meta);

__CRYPTO_DECLSPEC void
hmac_free(struct hmac_ctx *ctx);

__CRYPTO_DECLSPEC void
hmac_set_key(struct hmac_ctx *ctx, const uint8_t *key, size_t key_length);

__CRYPTO_DECLSPEC void
hmac_update(struct hmac_ctx *ctx, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
hmac_digest(struct hmac_ctx *ctx, uint8_t *dst, size_t length);

__CRYPTO_DECLSPEC unsigned
hmac_get_digest_size(const struct hmac_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
hmac_get_block_size(const struct hmac_ctx *ctx) ATTRIBUTE(pure);

#ifdef __cplusplus
}
#endif

#endif /* HMAC_H_ */
