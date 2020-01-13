/*
 * realloc.c
 *
 *  Created on: Oct 24, 2019, 11:08:34 AM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>
#include <stdio.h>

#include "crypto-realloc.h"

void* ATTRIBUTE(warn_unused_result,alloc_size(3))
crypto_realloc(void *ctx ATTRIBUTE(unused), void *p, size_t length) {
	if (length > 0) {
		return realloc(p, length);
	} else {
		free(p);
		return NULL;
	}
}

void* ATTRIBUTE(warn_unused_result,alloc_size(3))
crypto_xrealloc(void *ctx ATTRIBUTE(unused), void *p, size_t length) {
	if (length > 0) {
		void *n = realloc(p, length);
		if (!n) {
			fprintf(stderr, "Memory Exhausted.\n");
			abort();
		}
		return n;
	} else {
		free(p);
		return NULL;
	}
}
