/*
 * sha2.h
 *
 *  Created on: Apr 24, 2019, 10:02:46 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA2_H_
#define SHA2_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define sha256_init		crypto_sha256_init
#define sha512_init		crypto_sha512_init
#define sha256_update	crypto_sha256_update
#define sha512_update	crypto_sha512_update
#define sha256_digest	crypto_sha256_digest
#define sha512_digest	crypto_sha512_digest

#define sha224_init		crypto_sha224_init
#define sha384_init		crypto_sha384_init
#define sha224_digest	crypto_sha224_digest
#define sha384_digest	crypto_sha384_digest

#define sha512_224_init		crypto_sha512_224_init
#define sha512_256_init		crypto_sha512_256_init
#define sha512_224_digest	crypto_sha512_224_digest
#define sha512_256_digest	crypto_sha512_256_digest

/* SHA256 */

#define SHA256_DIGEST_SIZE 32
#define SHA256_BLOCK_SIZE 64

/* Digest is kept internally as 8 32-bit words. */
#define _SHA256_DIGEST_LENGTH 8

struct sha256_ctx {
	uint64_t count;                           /* 64-bit block count */
	unsigned int index;                       /* index into buffer */
	uint32_t state[_SHA256_DIGEST_LENGTH];    /* State variables */
	uint8_t block[SHA256_BLOCK_SIZE];         /* SHA256 data buffer */
};

__CRYPTO_DECLSPEC void
sha256_init(struct sha256_ctx *ctx);

__CRYPTO_DECLSPEC void
sha256_update(struct sha256_ctx *ctx, const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
sha256_digest(struct sha256_ctx *ctx, uint8_t *digest, size_t length);


/* SHA224, a truncated SHA256 with different initial state. */

#define SHA224_DIGEST_SIZE 28
#define SHA224_BLOCK_SIZE SHA256_BLOCK_SIZE
#define sha224_ctx sha256_ctx

__CRYPTO_DECLSPEC void
sha224_init(struct sha256_ctx *ctx);

#define sha224_update sha256_update

__CRYPTO_DECLSPEC void
sha224_digest(struct sha256_ctx *ctx, uint8_t *digest, size_t length);


/* SHA512 */

#define SHA512_DIGEST_SIZE 64
#define SHA512_BLOCK_SIZE 128

/* Digest is kept internally as 8 64-bit words. */
#define _SHA512_DIGEST_LENGTH 8

struct sha512_ctx {
	uint64_t count_low, count_high;           /* 128-bit block count */
	unsigned int index;                       /* index into buffer */
	uint64_t state[_SHA512_DIGEST_LENGTH];    /* State variables */
	uint8_t block[SHA512_BLOCK_SIZE];         /* SHA512 data buffer */
};

__CRYPTO_DECLSPEC void
sha512_init(struct sha512_ctx *ctx);

__CRYPTO_DECLSPEC void
sha512_update(struct sha512_ctx *ctx, const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
sha512_digest(struct sha512_ctx *ctx, uint8_t *digest, size_t length);


/* SHA384, a truncated SHA512 with different initial state. */

#define SHA384_DIGEST_SIZE 48
#define SHA384_BLOCK_SIZE SHA512_BLOCK_SIZE
#define sha384_ctx sha512_ctx

__CRYPTO_DECLSPEC void
sha384_init(struct sha512_ctx *ctx);

#define sha384_update sha512_update

__CRYPTO_DECLSPEC void
sha384_digest(struct sha512_ctx *ctx, uint8_t *digest, size_t length);


/* SHA512_224 and SHA512_256, two truncated versions of SHA512
   with different initial states. */

#define SHA512_224_DIGEST_SIZE 28
#define SHA512_224_BLOCK_SIZE SHA512_BLOCK_SIZE
#define sha512_224_ctx sha512_ctx

__CRYPTO_DECLSPEC void
sha512_224_init(struct sha512_224_ctx *ctx);

#define sha512_224_update sha512_update

__CRYPTO_DECLSPEC void
sha512_224_digest(struct sha512_224_ctx *ctx, uint8_t *digest, size_t length);


#define SHA512_256_DIGEST_SIZE 32
#define SHA512_256_BLOCK_SIZE SHA512_BLOCK_SIZE
#define sha512_256_ctx sha512_ctx

__CRYPTO_DECLSPEC void
sha512_256_init(struct sha512_256_ctx *ctx);

#define sha512_256_update sha512_update

__CRYPTO_DECLSPEC void
sha512_256_digest(struct sha512_256_ctx *ctx, uint8_t *digest, size_t length);


#ifdef __cplusplus
}
#endif

#endif /* SHA2_H_ */
