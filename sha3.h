/*
 * sha3.h
 *
 *  Created on: Aug 19, 2019, 11:58:33 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA3_H_
#define SHA3_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define sha3_224_init		crypto_sha3_224_init
#define sha3_256_init		crypto_sha3_256_init
#define sha3_384_init		crypto_sha3_384_init
#define sha3_512_init		crypto_sha3_512_init

#define sha3_224_update		crypto_sha3_224_update
#define sha3_256_update		crypto_sha3_256_update
#define sha3_384_update		crypto_sha3_384_update
#define sha3_512_update		crypto_sha3_512_update

#define sha3_224_digest		crypto_sha3_224_digest
#define sha3_256_digest		crypto_sha3_256_digest
#define sha3_384_digest		crypto_sha3_384_digest
#define sha3_512_digest		crypto_sha3_512_digest

/* The sha3 state is a 5x5 matrix of 64-bit words. In the notation of
 * Keccak description, S[x,y] is element x + 5*y, so if x is
 * interpreted as the row index and y the column index, it is stored
 * in column-major order. */
#define SHA3_STATE_LENGTH 25

/* The "width" is 1600 bits or 200 octets */
struct sha3_state {
	uint64_t a[SHA3_STATE_LENGTH];
};

/* The "capacity" is set to 2*(digest size), 512 bits or 64 octets.
 * The "rate" is the width - capacity, or width - 2 * (digest
 * size). */

#define SHA3_224_DIGEST_SIZE	28
#define SHA3_224_BLOCK_SIZE		144

#define SHA3_256_DIGEST_SIZE	32
#define SHA3_256_BLOCK_SIZE		136

#define SHA3_384_DIGEST_SIZE	48
#define SHA3_384_BLOCK_SIZE		104

#define SHA3_512_DIGEST_SIZE	64
#define SHA3_512_BLOCK_SIZE		72


struct sha3_224_ctx {
	struct sha3_state state;
	unsigned index;
	uint8_t block[SHA3_224_BLOCK_SIZE];
};

void
sha3_224_init(struct sha3_224_ctx *ctx);
void
sha3_224_update(struct sha3_224_ctx *ctx, const uint8_t *data, size_t length);
void
sha3_224_digest(struct sha3_224_ctx *ctx, uint8_t *digest, size_t length);

struct sha3_256_ctx {
	struct sha3_state state;
	unsigned index;
	uint8_t block[SHA3_256_BLOCK_SIZE];
};

void
sha3_256_init(struct sha3_256_ctx *ctx);
void
sha3_256_update(struct sha3_256_ctx *ctx, const uint8_t *data, size_t length);
void
sha3_256_digest(struct sha3_256_ctx *ctx, uint8_t *digest, size_t length);

struct sha3_384_ctx {
	struct sha3_state state;
	unsigned index;
	uint8_t block[SHA3_384_BLOCK_SIZE];
};

void
sha3_384_init(struct sha3_384_ctx *ctx);
void
sha3_384_update(struct sha3_384_ctx *ctx, const uint8_t *data, size_t length);
void
sha3_384_digest(struct sha3_384_ctx *ctx, uint8_t *digest, size_t length);

struct sha3_512_ctx {
	struct sha3_state state;
	unsigned index;
	uint8_t block[SHA3_512_BLOCK_SIZE];
};

void
sha3_512_init(struct sha3_512_ctx *ctx);
void
sha3_512_update(struct sha3_512_ctx *ctx, const uint8_t *data, size_t length);
void
sha3_512_digest(struct sha3_512_ctx *ctx, uint8_t *digest, size_t length);


#ifdef __cplusplus
}
#endif

#endif /* SHA3_H_ */
