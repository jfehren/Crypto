/*
 * memxor-internal.h
 *
 *  Created on: Mar 6, 2019, 2:21:45 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MEMXOR_INTERNAL_H_
#define MEMXOR_INTERNAL_H_

#include <stdint.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__arch64__)
typedef uint64_t word_t;
#else
typedef unsigned long int word_t;
#endif

#define ALIGN_OFFSET(p) ((uintptr_t) (p) % sizeof(word_t))

#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
#define MERGE(w0, s0, w1, s1) (((w0) >> (s0)) | ((w1) << (s1)))
#define READ_PARTIAL(r,p,n) do { \
	word_t __rp_x; \
	unsigned __rp_i; \
	for (__rp_i = (n), __rp_x = (p)[--__rp_i]; __rp_i > 0;) { \
		__rp_x = (__rp_x << CHAR_BIT) | (p)[--__rp_i]; \
	} \
	(r) = __rp_x; \
} while (0)
#else
#define MERGE(w0, s0, w1, s1) (((w0) << (s0)) | ((w1) >> (s1)))
#define READ_PARTIAL(r,p,n) do { \
	word_t __rp_x; \
	unsigned __rp_i; \
	for (__rp_x = (p)[0], __rp_i = 1; __rp_i < (n); __rp_i++) { \
		__rp_x = (__rp_x << CHAR_BIT) | (p)[__rp_i]; \
	} \
	(r) = __rp_x; \
} while (0)
#endif

#endif /* MEMXOR_INTERNAL_H_ */
