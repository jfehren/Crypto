/*
 * memwrite.h
 *
 *  Created on: Apr 19, 2019, 2:33:26 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MEMWRITE_H_
#define MEMWRITE_H_

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

__CRYPTO_DECLSPEC void
crypto_memwrite_be16(uint8_t* dst, const uint16_t* src, size_t length);
__CRYPTO_DECLSPEC void
crypto_memwrite_be32(uint8_t* dst, const uint32_t* src, size_t length);
__CRYPTO_DECLSPEC void
crypto_memwrite_be64(uint8_t* dst, const uint64_t* src, size_t length);

__CRYPTO_DECLSPEC void
crypto_memwrite_le16(uint8_t* dst, const uint16_t* src, size_t length);
__CRYPTO_DECLSPEC void
crypto_memwrite_le32(uint8_t* dst, const uint32_t* src, size_t length);
__CRYPTO_DECLSPEC void
crypto_memwrite_le64(uint8_t* dst, const uint64_t* src, size_t length);

#define WORDS_BIGENDIAN 	(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define WORDS_LITTLEENDIAN	(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#if WORDS_BIGENDIAN

#define memwrite_le16(dst,src,len)	crypto_memwrite_le16(dst, src, len)
#define memwrite_le32(dst,src,len)	crypto_memwrite_le32(dst, src, len)
#define memwrite_le64(dst,src,len)	crypto_memwrite_le64(dst, src, len)

#define memwrite_be16(dst,src,len)	memcpy(dst, src, len)
#define memwrite_be32(dst,src,len)	memcpy(dst, src, len)
#define memwrite_be64(dst,src,len)	memcpy(dst, src, len)

#elif WORDS_LITTLEENDIAN

#define memwrite_le16(dst,src,len)	memcpy(dst, src, len)
#define memwrite_le32(dst,src,len)	memcpy(dst, src, len)
#define memwrite_le64(dst,src,len)	memcpy(dst, src, len)

#define memwrite_be16(dst,src,len)	crypto_memwrite_be16(dst, src, len)
#define memwrite_be32(dst,src,len)	crypto_memwrite_be32(dst, src, len)
#define memwrite_be64(dst,src,len)	crypto_memwrite_be64(dst, src, len)

#else

#define memwrite_le16(dst,src,len)	crypto_memwrite_le16(dst, src, len)
#define memwrite_le32(dst,src,len)	crypto_memwrite_le32(dst, src, len)
#define memwrite_le64(dst,src,len)	crypto_memwrite_le64(dst, src, len)

#define memwrite_be16(dst,src,len)	crypto_memwrite_be16(dst, src, len)
#define memwrite_be32(dst,src,len)	crypto_memwrite_be32(dst, src, len)
#define memwrite_be64(dst,src,len)	crypto_memwrite_be64(dst, src, len)

#endif

#ifdef __cplusplus
}
#endif

#endif /* MEMWRITE_H_ */
