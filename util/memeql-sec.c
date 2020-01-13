/*
 * memeql-sec.c
 *
 *  Created on: Mar 7, 2019, 6:11:23 PM
 *      Author: Joshua Fehrenbach
 */

#include "memops.h"

int
memeql_sec(const void *a, const void *b, size_t n) {
	volatile const unsigned char *ap = (volatile const unsigned char*)a;
	volatile const unsigned char *bp = (volatile const unsigned char*)b;
	volatile unsigned char diff;
	size_t i;

	for (i = 0, diff = 0; i < n; i++) {
		diff |= ap[i] ^ bp[i];
	}

	return diff == 0;
}

