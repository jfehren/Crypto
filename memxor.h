/*
 * memxor.h
 *
 *  Created on: Mar 5, 2019, 3:29:29 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MEMXOR_H_
#define MEMXOR_H_

#include <stdlib.h>
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define memxor  crypto_memxor
#define memxor3 crypto_memxor3

__CRYPTO_DECLSPEC void *memxor(void *dst, const void *src, size_t n);
__CRYPTO_DECLSPEC void *memxor3(void *dst, const void *a, const void *b, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* MEMXOR_H_ */
