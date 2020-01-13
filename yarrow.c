/*
 * yarrow.c
 *
 *  Created on: Oct 22, 2019, 12:46:59 PM
 *      Author: Joshua Fehrenbach
 *
 * Based on GNU Nettle implementation
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "yarrow.h"
#include "macros.h"

/* Parameters */

/* An upper limit on the entropy (in bits) in one octet of sample data. */
#define YARROW_MULTIPLIER 4

/* Entropy threshold for reseeding from the fast pool */
#define YARROW_FAST_THRESHOLD 100

/* Entropy threshold for reseeding from the slow pool */
#define YARROW_SLOW_THRESHOLD 160

/* Number of sources that must exceed the threshold for slow reseed */
#define YARROW_SLOW_K 2

/* The number of iterations when reseeding, P_t in the yarrow paper.
 * Should be chosen so that reseeding takes on the order of 0.1-1
 * seconds. */
#define YARROW_RESEED_ITERATIONS 1500

/* Entropy estimates sticks to this value, it is treated as infinity
 * in calculations. It should fit comfortably in an uint32_t, to avoid
 * overflows. */
#define YARROW_MAX_ENTROPY 0x100000


static void yarrow_gate(struct yarrow256_ctx *ctx);

void
yarrow256_init(struct yarrow256_ctx *ctx, struct yarrow_source *sources, unsigned nsources) {
	unsigned i;

	sha256_init(&ctx->pools[0]);
	sha256_init(&ctx->pools[1]);

	ctx->seeded = 0;

	memset(ctx->counter, 0, sizeof(ctx->counter));

	ctx->nsources = nsources;
	ctx->sources = sources;

	for (i = 0; i < nsources; i++) {
		ctx->sources[i].estimate[YARROW_FAST] = 0;
		ctx->sources[i].estimate[YARROW_SLOW] = 0;
		ctx->sources[i].next = YARROW_FAST;
	}
}

void
yarrow256_seed(struct yarrow256_ctx *ctx, const uint8_t *seed_file, size_t length) {
	assert (length > 0);
	sha256_update(&ctx->pools[YARROW_FAST], seed_file, length);
	yarrow256_fast_reseed(ctx);
}

static void
yarrow_generate_block(struct yarrow256_ctx *ctx, uint8_t *block) {
	aes256_encrypt(&ctx->key, block, ctx->counter, sizeof(ctx->counter));
	/* Increment counter, treating it as a big-endian number. */
	INCREMENT(sizeof(ctx->counter), ctx->counter);
}

static void
yarrow_iterate(uint8_t *digest) {
	uint8_t v0[SHA256_DIGEST_SIZE], count[4];
	struct sha256_ctx hash;
	unsigned i;
	memcpy(v0, digest, SHA256_DIGEST_SIZE);
	sha256_init(&hash);
	for (i = 0; ++i < YARROW_RESEED_ITERATIONS; ) {
		/* sha256_digest(v_i | v_0 | i) */
		BE_WRITE_UINT32(count, i);

		sha256_update(&hash, digest, SHA256_DIGEST_SIZE);
		sha256_update(&hash, v0, sizeof(v0));
		sha256_update(&hash, count, sizeof(count));

		sha256_digest(&hash, digest, SHA256_DIGEST_SIZE);
	}
}

void
yarrow256_fast_reseed(struct yarrow256_ctx *ctx) {
	uint8_t digest[SHA256_DIGEST_SIZE];
	unsigned i;

	/* We feed two block of output using the current key into the pool
	 * before emptying it. */
	if (ctx->seeded) {
		uint8_t blocks[YARROW256_SEED_FILE_SIZE];

		yarrow_generate_block(ctx, blocks);
		yarrow_generate_block(ctx, blocks + AES_BLOCK_SIZE);
		sha256_update(&ctx->pools[YARROW_FAST], blocks, sizeof(blocks));
	}

	sha256_digest(&ctx->pools[YARROW_FAST], digest, sizeof(digest));

	/* Iterate */
	yarrow_iterate(digest);

	aes256_set_encrypt_key(&ctx->key, digest, sizeof(digest));
	ctx->seeded = 1;

	/* Derive new counter */
	memset(ctx->counter, 0, sizeof(ctx->counter));
	aes256_encrypt(&ctx->key, ctx->counter, ctx->counter, sizeof(ctx->counter));

	/* Reset estimates */
	for (i = 0; i < ctx->nsources; i++) {
		ctx->sources[i].estimate[YARROW_FAST] = 0;
	}
}

void
yarrow256_slow_reseed(struct yarrow256_ctx *ctx) {
	uint8_t digest[SHA256_DIGEST_SIZE];
	unsigned i;

	/* Get the digest of the slow pool */
	sha256_digest(&ctx->pools[YARROW_SLOW], digest, sizeof(digest));
	/* Feed it into the fast pool */
	sha256_update(&ctx->pools[YARROW_FAST], digest, sizeof(digest));

	yarrow256_fast_reseed(ctx);

	/* Reset estimates */
	for (i = 0; i < ctx->nsources; i++) {
		ctx->sources[i].estimate[YARROW_SLOW] = 0;
	}
}

int
yarrow256_update(struct yarrow256_ctx *ctx, unsigned src_idx, unsigned entropy,
		const uint8_t *data, size_t length) {
	enum yarrow_pool_id current;
	struct yarrow_source *source;

	assert (src_idx < ctx->nsources);

	if (!length) {
		/* Do nothing */
		return 0;
	}

	source = &ctx->sources[src_idx];

	if (!ctx->seeded) {
		/* while seeding, use slow pool */
		current = YARROW_SLOW;
	} else {
		current = source->next;
		source->next = !source->next;
	}

	sha256_update(&ctx->pools[current], data, length);

	/* NOTE: We should be careful to avoid overflows in the estimates. */
	if (source->estimate[current] < YARROW_MAX_ENTROPY) {
		if (entropy > YARROW_MAX_ENTROPY) {
			entropy = YARROW_MAX_ENTROPY;
		}

		if (length < (YARROW_MAX_ENTROPY / YARROW_MULTIPLIER) &&
				entropy > (YARROW_MULTIPLIER * length)) {
			entropy = YARROW_MULTIPLIER * length;
		}

		entropy += source->estimate[current];
		if (entropy > YARROW_MAX_ENTROPY) {
			entropy = YARROW_MAX_ENTROPY;
		}

		source->estimate[current] = entropy;
	}

	/* Check for seed/reseed */
	switch (current) {
		case YARROW_FAST:
			if (source->estimate[YARROW_FAST] >= YARROW_FAST_THRESHOLD) {
				yarrow256_fast_reseed(ctx);
				return 1;
			} else {
				return 0;
			}

		case YARROW_SLOW:
			if (!yarrow256_needed_sources(ctx)) {
				yarrow256_slow_reseed(ctx);
				return 1;
			} else {
				return 0;
			}

		default:	/* corrupted/invalid state */
			abort();
	}
}

static void
yarrow_gate(struct yarrow256_ctx *ctx) {
	uint8_t key[AES256_KEY_SIZE];
	unsigned i;

	for (i = 0; i < sizeof(key); i += AES_BLOCK_SIZE) {
		yarrow_generate_block(ctx, key + i);
	}

	aes256_set_encrypt_key(&ctx->key, key, sizeof(key));
}

void
yarrow256_random(struct yarrow256_ctx *ctx, uint8_t *dst, size_t length) {
	assert(ctx->seeded);

	while (length >= AES_BLOCK_SIZE) {
		yarrow_generate_block(ctx, dst);
		dst += AES_BLOCK_SIZE;
		length -= AES_BLOCK_SIZE;
	}
	if (length) {
		uint8_t buffer[AES_BLOCK_SIZE];

		assert(length < AES_BLOCK_SIZE);
		yarrow_generate_block(ctx, buffer);
		memcpy(dst, buffer, length);
	}
	yarrow_gate(ctx);
}

int
yarrow256_is_seeded(struct yarrow256_ctx *ctx) {
	return ctx->seeded;
}

unsigned
yarrow256_needed_sources(struct yarrow256_ctx *ctx) {
	unsigned k, i;
	for (i = k = 0; i < ctx->nsources; i++) {
		if (ctx->sources[i].estimate[YARROW_SLOW] >= YARROW_SLOW_THRESHOLD) {
			k++;
		}
	}
	return (k < YARROW_SLOW_K) ? (YARROW_SLOW_K - k) : 0;
}


void
yarrow_key_event_init(struct yarrow_key_event_ctx *ctx) {
	unsigned i;

	ctx->index = 0;
	ctx->previous = 0;

	for (i = 0; i < YARROW_KEY_EVENT_BUFFER; i++) {
		ctx->chars[i] = 0;
	}
}

unsigned
yarrow_key_event_estimate(struct yarrow_key_event_ctx *ctx, unsigned key, unsigned time) {
	unsigned entropy = 0;
	unsigned i;

	/* Look at timing first. */
	if (ctx->previous && (time > ctx->previous) ) {
		if ((time - ctx->previous) >= 256) {
			entropy++;
		}
	}
	ctx->previous = time;

	if (!key) {
		return entropy;
	}

	for (i = 0; i < YARROW_KEY_EVENT_BUFFER; i++) {
		if (key == ctx->chars[i]) {
			/* This is a recent character. Ignore it. */
			return entropy;
		}
	}

	/* Count one bit of entropy, unless this was one of the initial 16
	 * characters. */
	if (ctx->chars[ctx->index]) {
		entropy++;
	}

	/* Remember the character. */

	ctx->chars[ctx->index] = key;
	ctx->index = (ctx->index + 1) % YARROW_KEY_EVENT_BUFFER;

	return entropy;
}

