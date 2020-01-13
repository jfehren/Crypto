/*
 * sha1-internal.h
 *
 *  Created on: Apr 19, 2019, 10:08:22 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef SHA1_INTERNAL_H_
#define SHA1_INTERNAL_H_

#include "sha1.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

#define _sha1_compress		(*_crypto_sha1_compress_fat)
#define _sha1_compress_gen	_crypto_sha1_compress_gen
#define _sha1_compress_x86	_crypto_sha1_compress_x86
#define _sha1_compress_x64	_crypto_sha1_compress_x64
#define _sha1_compress_sha	_crypto_sha1_compress_sha

void
_sha1_compress_x86(uint32_t* state, const uint8_t* data, size_t length);

void
_sha1_compress_x64(uint32_t* state, const uint8_t* data, size_t length);

void
_sha1_compress_sha(uint32_t* state, const uint8_t* data, size_t length);

extern void _sha1_compress(uint32_t* state, const uint8_t* data, size_t length);

#else

#define _sha1_compress	_crypto_sha1_compress
#define _sha1_compress_gen	_sha1_compress

#endif

void
_sha1_compress_gen(uint32_t* state, const uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* SHA1_INTERNAL_H_ */
