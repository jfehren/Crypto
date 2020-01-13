/*
 * tmp-alloc.h
 *
 *  Created on: Oct 31, 2019, 12:16:54 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef TMP_ALLOC_H_
#define TMP_ALLOC_H_

#include "crypto.h"

void *
crypto_gmp_tmp_alloc(size_t n) ATTRIBUTE(malloc,alloc_size(1));

void
crypto_gmp_tmp_free(void *p, size_t n);

#define TSIZ(name) tmp_##name##_size

#define TMP_DECL(name, type) type *name; size_t TSIZ(name)

#ifdef CRYPTO_ALLOCA
#include <alloca.h>
#define TMP_ALLOC(name, size) ( \
	TSIZ(name) = (size)*sizeof(*name), \
	name = LIKELY(TSIZ(name) <= 0x7f00) \
				? alloca(TSIZ(name)) \
				: crypto_gmp_tmp_alloc(TSIZ(name)) )
#define TMP_FREE(name) do { \
	if (UNLIKELY(TSIZ(name) > 0x7f00)) { \
		crypto_gmp_tmp_free(name, TSIZ(name)); \
	} \
} while (0)
#else
#define TMP_ALLOC(name,size) (name = crypto_gmp_tmp_alloc(TSIZ(name) = (size)*sizeof(*name)))
#define TMP_FREE(name) crypto_gmp_tmp_free(name, TSIZ(name))
#endif

#endif /* TMP_ALLOC_H_ */
