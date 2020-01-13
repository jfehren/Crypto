/*
 * sha3-internal.h
 *
 *  Created on: Aug 19, 2019, 12:11:03 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA3_SHA3_INTERNAL_H_
#define SHA3_SHA3_INTERNAL_H_

#include "sha3.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

#define sha3_permute		(*crypto_sha3_permute_fat)
#define sha3_permute_gen	crypto_sha3_permute_gen
#define sha3_permute_avx	crypto_sha3_permute_avx

void
sha3_permute_avx(uint64_t *A);

extern void
sha3_permute(uint64_t *A);

#else

#define sha3_permute	crypto_sha3_permute
#define sha3_permute_gen sha3_permute

#endif

void
sha3_permute_gen(uint64_t *A);

#ifdef __cplusplus
}
#endif

#endif /* SHA3_SHA3_INTERNAL_H_ */
