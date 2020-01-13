/*
 * md4.h
 *
 *  Created on: Aug 26, 2019, 10:54:32 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD4_H_
#define MD4_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define md4_init	crypto_md4_init
#define md4_update	crypto_md4_update
#define md4_digest	crypto_md4_digest

#define MD4_DIGEST_SIZE 16
#define MD4_BLOCK_SIZE 64

/* Digest is kept internally as 4 32-bit words. */
#define _MD4_DIGEST_LENGTH 4

/* FIXME: Identical to md5_ctx */
struct md4_ctx {
	uint32_t state[_MD4_DIGEST_LENGTH];
	uint64_t count;			/* Block count */
	uint8_t block[MD4_BLOCK_SIZE];	/* Block buffer */
	unsigned index;			/* Into buffer */
};

__CRYPTO_DECLSPEC void
md4_init(struct md4_ctx *ctx);

__CRYPTO_DECLSPEC void
md4_update(struct md4_ctx *ctx, const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
md4_digest(struct md4_ctx *ctx, uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MD4_H_ */
