/*
 * yarrow.h
 *
 *  Created on: Oct 22, 2019, 12:40:17 PM
 *      Author: Joshua Fehrenbach
 *
 * Based on GNU Nettle implementation
 */

#ifndef YARROW_H_
#define YARROW_H_

#include "aes.h"
#include "sha2.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define yarrow256_init				crypto_yarrow256_init
#define yarrow256_seed				crypto_yarrow256_seed
#define yarrow256_update			crypto_yarrow256_update
#define yarrow256_random			crypto_yarrow256_random
#define yarrow256_is_seeded			crypto_yarrow256_is_seeded
#define yarrow256_needed_sources	crypto_yarrow256_needed_sources
#define yarrow256_fast_reseed		crypto_yarrow256_fast_reseed
#define yarrow256_slow_reseed		crypto_yarrow256_slow_reseed
#define yarrow_key_event_init		crypto_yarrow_key_event_init
#define yarrow_key_event_estimate	crypto_yarrow_key_event_estimate

enum yarrow_pool_id { YARROW_FAST = 0, YARROW_SLOW = 1 };

struct yarrow_source {
	uint32_t estimate[2];		/* Indexed by yarrow_pool_id */
	enum yarrow_pool_id next;	/* The pool next sample should go to. */
};

#define YARROW256_SEED_FILE_SIZE (2 * AES_BLOCK_SIZE)

/* Yarrow-256, based on SHA-256 and AES-256 */
struct yarrow256_ctx {
	/* Indexed by yarrow_pool_id */
	struct sha256_ctx pools[2];

	int seeded;

	/* The current key and counter block */
	struct aes256_ctx key;
	uint8_t counter[AES_BLOCK_SIZE];

	/* The entropy sources */
	unsigned nsources;
	struct yarrow_source *sources;
};

__CRYPTO_DECLSPEC void
yarrow256_init(struct yarrow256_ctx *ctx, struct yarrow_source *sources, unsigned nsources);

__CRYPTO_DECLSPEC void
yarrow256_seed(struct yarrow256_ctx *ctx, const uint8_t *seed_file, size_t length);

/* Returns 1 on reseed */
__CRYPTO_DECLSPEC int
yarrow256_update(struct yarrow256_ctx *ctx, unsigned source, unsigned entropy,
		const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
yarrow256_random(struct yarrow256_ctx *ctx, uint8_t *dst, size_t length);

__CRYPTO_DECLSPEC int
yarrow256_is_seeded(struct yarrow256_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC unsigned
yarrow256_needed_sources(struct yarrow256_ctx *ctx) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC void
yarrow256_fast_reseed(struct yarrow256_ctx *ctx);

__CRYPTO_DECLSPEC void
yarrow256_slow_reseed(struct yarrow256_ctx *ctx);

/* Keyboard event estimator */
#define YARROW_KEY_EVENT_BUFFER 16

struct yarrow_key_event_ctx {
	/* Counter for initial priming of the state */
	unsigned index;
	unsigned chars[YARROW_KEY_EVENT_BUFFER];
	unsigned previous;
};

__CRYPTO_DECLSPEC void
yarrow_key_event_init(struct yarrow_key_event_ctx *ctx);

__CRYPTO_DECLSPEC unsigned
yarrow_key_event_estimate(struct yarrow_key_event_ctx *ctx, unsigned key, unsigned time);

#ifdef __cplusplus
}
#endif

#endif /* YARROW_H_ */
