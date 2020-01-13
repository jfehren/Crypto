/*
 * sha256-internal.h
 *
 *  Created on: Apr 24, 2019, 11:05:41 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA256_INTERNAL_H_
#define SHA256_INTERNAL_H_

#include "sha2.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

#define _sha256_compress		(*_crypto_sha256_compress_fat)
#define _sha256_compress_gen	_crypto_sha256_compress_gen
#define _sha256_compress_x86	_crypto_sha256_compress_x86
#define _sha256_compress_x64	_crypto_sha256_compress_x64
#define _sha256_compress_sha	_crypto_sha256_compress_sha

void
_sha256_compress_x86(uint32_t *state, const uint8_t *data, size_t length, const uint32_t *K);

void
_sha256_compress_x64(uint32_t *state, const uint8_t *data, size_t length, const uint32_t *K);

void
_sha256_compress_sha(uint32_t *state, const uint8_t *data, size_t length, const uint32_t *K);

extern void
_sha256_compress(uint32_t *state, const uint8_t *data, size_t length, const uint32_t *K);

#else

#define _sha256_compress	_crypto_sha256_compress
#define _sha256_compress_gen	_sha256_compress

#endif

void
_sha256_compress_gen(uint32_t *state, const uint8_t *data, size_t length, const uint32_t *K);

#ifdef __cplusplus
}
#endif

#endif /* SHA256_INTERNAL_H_ */
