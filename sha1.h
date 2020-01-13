/*
 * sha1.h
 *
 *  Created on: Apr 15, 2019, 3:08:17 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA1_H_
#define SHA1_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define sha1_init crypto_sha1_init
#define sha1_update crypto_sha1_update
#define sha1_digest crypto_sha1_digest

#define SHA1_DIGEST_SIZE 20
#define SHA1_BLOCK_SIZE 64

/* Digest is kept internally as 5 32-bit words. */
#define _SHA1_DIGEST_LENGTH 5

struct sha1_ctx {
	uint64_t count;                      /* 64-bit block count */
	unsigned index;                      /* index into buffer */
	uint32_t state[_SHA1_DIGEST_LENGTH]; /* State variables */
	uint8_t  block[SHA1_BLOCK_SIZE];     /* SHA1 data buffer */
};

__CRYPTO_DECLSPEC void
sha1_init(struct sha1_ctx *ctx);

__CRYPTO_DECLSPEC void
sha1_update(struct sha1_ctx *ctx, const uint8_t *src, size_t length);

__CRYPTO_DECLSPEC void
sha1_digest(struct sha1_ctx *ctx, uint8_t *dst, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_H_ */
