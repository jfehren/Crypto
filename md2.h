/*
 * md2.h
 *
 *  Created on: Aug 26, 2019, 10:38:31 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD2_H_
#define MD2_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define md2_init	crypto_md2_init
#define md2_update	crypto_md2_update
#define md2_digest	crypto_md2_digest

#define MD2_DIGEST_SIZE 16
#define MD2_BLOCK_SIZE 16

struct md2_ctx {
	uint8_t C[MD2_BLOCK_SIZE];
	uint8_t X[3 * MD2_BLOCK_SIZE];
	uint8_t block[MD2_BLOCK_SIZE]; /* Block buffer */
	unsigned index;               /* Into buffer */
};

__CRYPTO_DECLSPEC void
md2_init(struct md2_ctx *ctx);

__CRYPTO_DECLSPEC void
md2_update(struct md2_ctx *ctx, const uint8_t *data, size_t length);

__CRYPTO_DECLSPEC void
md2_digest(struct md2_ctx *ctx, uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MD2_H_ */
