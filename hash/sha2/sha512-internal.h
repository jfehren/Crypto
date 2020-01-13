/*
 * sha512-internal.h
 *
 *  Created on: Apr 27, 2019, 6:13:50 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA2_SHA512_INTERNAL_H_
#define SHA2_SHA512_INTERNAL_H_

#include "sha2.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

#define _sha512_compress		(*_crypto_sha512_compress_fat)
#define _sha512_compress_gen	_crypto_sha512_compress_gen
#define _sha512_compress_x64	_crypto_sha512_compress_x64

void
_sha512_compress_x64(uint64_t *state, const uint8_t *data, size_t length, const uint64_t *K);

extern void
_sha512_compress(uint64_t *state, const uint8_t *data, size_t length, const uint64_t *K);

#else

#define _sha512_compress	_crypto_sha512_compress
#define _sha512_compress_gen	_sha512_compress

#endif

void
_sha512_compress_gen(uint64_t *state, const uint8_t *data, size_t length, const uint64_t *K);

#ifdef __cplusplus
}
#endif

#endif /* SHA2_SHA512_INTERNAL_H_ */
