/*
 * memops.h
 *
 *  Created on: Mar 7, 2019, 6:10:21 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MEMOPS_H_
#define MEMOPS_H_

#include "memxor.h"
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define memeql_sec crypto_memeql_sec

__CRYPTO_DECLSPEC int
memeql_sec(const void *a, const void *b, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* MEMOPS_H_ */
