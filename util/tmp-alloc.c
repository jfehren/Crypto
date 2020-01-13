/*
 * tmp-alloc.c
 *
 *  Created on: Nov 5, 2019, 10:25:34 AM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <gmp.h>

#include "tmp-alloc.h"

typedef void *(*gmp_alloc_func)(size_t);
typedef void (*gmp_free_func)(void *, size_t);

void * ATTRIBUTE(malloc,alloc_size(1))
crypto_gmp_tmp_alloc(size_t n) {
	gmp_alloc_func gmp_alloc;
	gmp_free_func gmp_free;
	void *p;
	if (n == 0) {
		return NULL;
	}
	/* Get the allocation and de-allocation functions */
	mp_get_memory_functions(&gmp_alloc, NULL, &gmp_free);
	/* add storage for the de-allocation function pointer */
	n += sizeof(gmp_free_func*);
	/* Allocate the storage */
	p = (*gmp_alloc)(n);
	assert (p != NULL);
	/* Save the de-allocation function pointer */
	((gmp_free_func)p)[0] = gmp_free;
	p = &((gmp_free_func)p)[1];
	add_free_func(p, gmp_free);
	return p;
}

void
crypto_gmp_tmp_free(void *p, size_t n) {
	/* get the de-allocation function pointer */
	gmp_free_func gmp_free;
	p = &((gmp_free_func)p)[-1];
	gmp_free = p;
	/* de-allocate the temporary storage */
	(*gmp_free)(p, n);
}
