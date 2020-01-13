/*
 * crypto.h
 *
 *  Created on: Feb 11, 2019, 12:34:19 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_H_
#define CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#if defined (__CRYPTO_WITHIN_CONFIGURE)
#define __CRYPTO_LIBCRYPTO_DLL  0
#else
#ifndef __CRYPTO_LIBCRYPTO_DLL
/* Compile DLL by default */
#define __CRYPTO_LIBCRYPTO_DLL	1
#endif
#endif

#define __CRYPTO_DECLSPEC_EXPORT  __declspec(dllexport)
#define __CRYPTO_DECLSPEC_IMPORT  __declspec(dllimport)

#if __CRYPTO_LIBCRYPTO_DLL
#ifdef __CRYPTO_WITHIN_CRYPTO
/* compiling to go into a DLL libcrypto */
#define __CRYPTO_DECLSPEC  __CRYPTO_DECLSPEC_EXPORT
#else
/* compiling to go into an application which will link to DLL libcrypto */
#define __CRYPTO_DECLSPEC  __CRYPTO_DECLSPEC_IMPORT
#endif
#else
/* all other cases (i.e. building/linking a static library) */
#define __CRYPTO_DECLSPEC
#endif

#if defined(_M_X64) || defined(__x86_64__) || defined(__i386) || defined(_M_IX86) || \
	defined(__X86__) || defined(_X86_) || defined(__THW_INTEL__) || defined(__I86__) || \
	defined(__INTEL__) || defined(__386)
#define __IS_x86__ 1
#  if defined(_M_X64) || defined(__x86_64__)
#  define __IS_x64__ 1
#  else
#  define __IS_x64__ 0
#  endif
#else
#define __IS_x86__ 0
#define __IS_x64__ 0
#endif

#define WORDS_BIGENDIAN 	(__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#define WORDS_LITTLEENDIAN	(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)

#if LONG_MAX >= 0x7fffffffffffffffL
#define SIZEOF_LONG	8
#elif LONG_MAX >= 0x7fffffffffffL
#define SIZEOF_LONG 7
#elif LONG_MAX >= 0x7fffffffffL
#define SIZEOF_LONG 6
#elif LONG_MAX >= 0x7fffffffL
#define SIZEOF_LONG 5
#elif LONG_MAX >= 0x7fffffL
#define SIZEOF_LONG 4
#elif LONG_MAX >= 0x7fffL
#define SIZEOF_LONG 3
#elif LONG_MAX >= 0x7fL
#define SIZEOF_LONG 2
#else
#define SIZEOF_LONG 1
#endif

#if LLONG_MAX == LONG_MAX
#define SIZEOF_LLONG SIZEOF_LONG
#define LONG_IS_LLONG 1
#else
#define LONG_IS_LLONG 0
#  if LLONG_MAX >= 0x7fffffffffffffffLL
#  define SIZEOF_LLONG	8
#  elif LLONG_MAX >= 0x7fffffffffffLL
#  define SIZEOF_LLONG 7
#  elif LLONG_MAX >= 0x7fffffffffLL
#  define SIZEOF_LLONG 6
#  elif LLONG_MAX >= 0x7fffffffLL
#  define SIZEOF_LLONG 5
#  elif LLONG_MAX >= 0x7fffffLL
#  define SIZEOF_LLONG 4
#  elif LLONG_MAX >= 0x7fffLL
#  define SIZEOF_LLONG 3
#  elif LLONG_MAX >= 0x7fLL
#  define SIZEOF_LLONG 2
#  else
#  define SIZEOF_LLONG 1
#  endif
#endif

#if defined(__GNUC__) && defined(__GNUC_MINOR__)
#define __GNUC_PREREQ__(maj,min) \
	(((__GNUC__ << 16) + __GNUC_MINOR__) >= (((maj) << 16) + (min)))
#else
#define __GNUC_PREREQ__(maj,min) 0
#endif

#if __GNUC_PREREQ__(2,96)
#define ATTRIBUTE(attr, ...) __attribute__((attr,##__VA_ARGS__))
#else
#define ATTRIBUTE(attr, ...)
#endif

#if __GNUC_PREREQ__(3,0)
#define EXPECT(cond,exp)	__builtin_expect((cond), (exp))
#else
#define EXPECT(cond,exp)	(cond)
#endif
#define LIKELY(cond)		EXPECT((cond) != 0, 1)
#define UNLIKELY(cond)		EXPECT((cond) != 0, 0)

#include "crypto-types.h"

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_H_ */
