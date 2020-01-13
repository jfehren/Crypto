/*
 * crypto-realloc.h
 *
 *  Created on: Oct 24, 2019, 11:07:01 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_REALLOC_H_
#define CRYPTO_REALLOC_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

__CRYPTO_DECLSPEC void*
crypto_realloc(void *ctx, void *p, size_t length) ATTRIBUTE(warn_unused_result,alloc_size(3));

__CRYPTO_DECLSPEC void*
crypto_xrealloc(void *ctx, void *p, size_t length) ATTRIBUTE(warn_unused_result,alloc_size(3));

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_REALLOC_H_ */
