/*
 * md5.h
 *
 *  Created on: Aug 26, 2019, 1:47:30 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD5_H_
#define MD5_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define md5_init	crypto_md5_init
#define md5_update	crypto_md5_update
#define md5_digest	crypto_md5_digest

#define MD5_DIGEST_SIZE 16
#define MD5_BLOCK_SIZE 64

/* Digest is kept internally as 4 32-bit words. */
#define _MD5_DIGEST_LENGTH 4

struct md5_ctx {
	uint32_t state[_MD5_DIGEST_LENGTH];
	uint64_t count;               /* Block count */
	uint8_t block[MD5_BLOCK_SIZE]; /* Block buffer */
	unsigned index;               /* Into buffer */
};

void
md5_init(struct md5_ctx *ctx);

void
md5_update(struct md5_ctx *ctx, const uint8_t *data, size_t length);

void
md5_digest(struct md5_ctx *ctx, uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MD5_H_ */
