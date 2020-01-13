/*
 * md5-internal.h
 *
 *  Created on: Aug 26, 2019, 1:50:02 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MD5_INTERNAL_H_
#define MD5_INTERNAL_H_

#include "md5.h"
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

#define _md5_compress	(*_crypto_md5_compress_fat)

#define _md5_compress_gen	_crypto_md5_compress_gen
#define _md5_compress_x86	_crypto_md5_compress_x86
#define _md5_compress_x64	_crypto_md5_compress_x64

void
_md5_compress_x86(uint32_t *state, const uint8_t *data);

void
_md5_compress_x64(uint32_t *state, const uint8_t *data);

extern void
_md5_compress(uint32_t *state, const uint8_t *data);

#else

#define _md5_compress		_crypto_md5_compress
#define _md5_compress_gen	_md5_compress

#endif

void
_md5_compress_gen(uint32_t *state, const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif /* MD5_INTERNAL_H_ */
