/*
 * md6.h
 *
 *  Created on: Aug 27, 2019, 9:14:29 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD6_H_
#define MD6_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define md6_init		crypto_md6_init
#define md6_init_full	crypto_md6_init_full
#define md6_update		crypto_md6_update
#define md6_digest		crypto_md6_digest

#define md6_224_init	crypto_md6_224_init
#define md6_256_init	crypto_md6_256_init
#define md6_384_init	crypto_md6_384_init
#define md6_512_init	crypto_md6_512_init

/* Define constants */
#define MD6_W		64		/* Word Size (in bits) */
#define MD6_N		89		/* Size of Compression Input Block (in words) */
#define MD6_C		16		/* Size of Compression Output (in words), called a "chunk" */
#define MD6_MAX_R	255		/* Maximum Number of Compression Rounds */
#define MD6_MAX_L	255		/* Maximum Number of Parallel Passes */

#define MD6_K		8		/* key words per compression block (>=0) */
#define MD6_B		64		/* data words per compression block (>0) */

#define MD6_DEFAULT_L	64	/* large so that MD6 is fully hierarchical */
#define MD6_MAX_HEIGHT	28


#define MD6_BLOCK_SIZE	(MD6_B*8)	/* bytes per compression block */
#define MD6_DIGEST_SIZE	(512/8)		/* maximum digest size */

#define MD6_224_BLOCK_SIZE	MD6_BLOCK_SIZE
#define MD6_224_DIGEST_SIZE	28

#define MD6_256_BLOCK_SIZE	MD6_BLOCK_SIZE
#define MD6_256_DIGEST_SIZE	32

#define MD6_384_BLOCK_SIZE	MD6_BLOCK_SIZE
#define MD6_384_DIGEST_SIZE	48

#define MD6_512_BLOCK_SIZE	MD6_BLOCK_SIZE
#define MD6_512_DIGEST_SIZE	64

struct md6_ctx {
	uint64_t B[MD6_MAX_HEIGHT][MD6_B];	/* uint64_t B[28][64]
										 * stack of 28 64-word partial blocks waiting
										 * to be completed and compressed.
										 * B[0] is for compressing text data (input);
										 * B[ell] corresponds to node at
										 *    level ell in the tree.
										 */

	unsigned bytes[MD6_MAX_HEIGHT];	/* bytes[ell] =
									 *    number of bytes already placed in B[ell]
									 *    for 0 <= ell < max_stack_height
									 * 0 <= bytes[ell] <= b*w/8
									 */

	uint64_t i_level[MD6_MAX_HEIGHT];	/* i_level[ell] =
										 *    index of the node B[ell] on this
										 *    level (0,1,...) when it is output
										 */

	uint64_t K[MD6_K];		/* 8 word key (aka "salt") for this instance of md6 */

	int d;					/* desired hash bit length. 1 <= d <= 512. */
	int keylen;				/* number of bytes in key K. 0<=keylen<=k*(w/8) */

	int L;				/* md6 mode specification parameter. 0 <= L <= 255
						 * L == 0 means purely sequential (Merkle-Damgaard)
						 * L >= 29 means purely tree-based
						 * Default is MD6_DEFAULT_L = 64 (hierarchical)
						 */

	int r;				/* Number of rounds. 0 <= r <= 255 */
	int top;			/* index of block corresponding to top of stack */
};

__CRYPTO_DECLSPEC void
md6_init(struct md6_ctx *ctx, int d);

__CRYPTO_DECLSPEC void
md6_224_init(struct md6_ctx *ctx);

__CRYPTO_DECLSPEC void
md6_256_init(struct md6_ctx *ctx);

__CRYPTO_DECLSPEC void
md6_384_init(struct md6_ctx *ctx);

__CRYPTO_DECLSPEC void
md6_512_init(struct md6_ctx *ctx);

__CRYPTO_DECLSPEC void
md6_init_full(struct md6_ctx *ctx, int d, const uint8_t *K, size_t keylen, int L, int r);

__CRYPTO_DECLSPEC void
md6_update(struct md6_ctx *ctx, const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
md6_digest(struct md6_ctx *ctx, uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MD6_H_ */
