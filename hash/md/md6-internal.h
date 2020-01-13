/*
 * md6-internal.h
 *
 *  Created on: Aug 27, 2019, 11:50:57 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD_MD6_INTERNAL_H_
#define MD_MD6_INTERNAL_H_

#include "md6.h"

#ifdef __cplusplus
extern "C" {
#endif

#define _md6_process	_crypto_md6_process
#define _md6_final		_crypto_md6_final

#define MD6_Q		15		/* Q words in compression block (>=0) */
#define MD6_U		1		/* words for unique node ID (64/MD6_W) */
#define MD6_V		1		/* words for control word (64/MD6_W) */

#undef max
#undef min

#define max(a,b)	((a)>(b)?(a):(b))
#define min(a,b)	((a)<(b)?(a):(b))

#define MD6_DEFAULT_R(d,k)	((k) > 0 ? max(80, 40 + ((d)/4)) : (40 + ((d)/4)))

#define MD6_CHUNK_SIZE	(MD6_C*MD6_W/8)

void
_md6_process(struct md6_ctx *ctx);

void
_md6_final(struct md6_ctx *ctx, int ell, uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* MD_MD6_INTERNAL_H_ */
