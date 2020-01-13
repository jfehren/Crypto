/*
 * memwrite.c
 *
 *  Created on: Apr 19, 2019, 2:42:14 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>
#include "memwrite.h"
#include "macros.h"

void
crypto_memwrite_be16(uint8_t* dst, const uint16_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 2;
	left = length % 2;

	for (i = 0; i < words; i++) {
		BE_WRITE_UINT16(dst, src[i]);
		dst += 2;
	}
	if (left) {
		*dst = (src[i] >> 8) & 0xff;
	}
}

void
crypto_memwrite_be32(uint8_t* dst, const uint32_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 4;
	left = length % 4;

	for (i = 0; i < words; i++) {
		BE_WRITE_UINT32(dst, src[i]);
		dst += 4;
	}
	if (left) {
		uint32_t word = src[i];

		/* Use a jump table rather than a switch statement to avoid warnings about
		 * there being no break at the end of a case statement */

		static void *jumps[4] = { &&l0, &&l1, &&l2, &&l3 };

		goto *jumps[left];

l3:		dst[--left] = (word >> 8) & 0xff;
l2:		dst[--left] = (word >> 16) & 0xff;
l1:		dst[--left] = (word >> 24) & 0xff;
l0:		return;
	}
}

void
crypto_memwrite_be64(uint8_t* dst, const uint64_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 8;
	left = length % 8;

	for (i = 0; i < words; i++) {
		BE_WRITE_UINT64(dst, src[i]);
		dst += 8;
	}
	if (left) {
		uint64_t word = src[i];

		/* Use a jump table rather than a switch statement to avoid warnings about
		 * there being no break at the end of a case statement */

		static void *jumps[8] = { &&l0, &&l1, &&l2, &&l3, &&l4, &&l5, &&l6, &&l7 };

		goto *jumps[left];

l7:		dst[--left] = (word >> 8) & 0xff;
l6:		dst[--left] = (word >> 16) & 0xff;
l5:		dst[--left] = (word >> 24) & 0xff;
l4:		dst[--left] = (word >> 32) & 0xff;
l3:		dst[--left] = (word >> 40) & 0xff;
l2:		dst[--left] = (word >> 48) & 0xff;
l1:		dst[--left] = (word >> 56) & 0xff;
l0:		return;
	}
}


void
crypto_memwrite_le16(uint8_t* dst, const uint16_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 2;
	left = length % 2;

	for (i = 0; i < words; i++) {
		LE_WRITE_UINT16(dst, src[i]);
		dst += 2;
	}
	if (left) {
		*dst = src[i] & 0xff;
	}
}

void
crypto_memwrite_le32(uint8_t* dst, const uint32_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 4;
	left = length % 4;

	for (i = 0; i < words; i++) {
		LE_WRITE_UINT32(dst, src[i]);
		dst += 4;
	}
	if (left) {
		uint32_t word = src[i];

		do {
			*dst++ = word & 0xff;
			word >>= 8;
		} while (--left);
	}
}

void
crypto_memwrite_le64(uint8_t* dst, const uint64_t* src, size_t length) {
	size_t i, words;
	unsigned left;

	words = length / 8;
	left = length % 8;

	for (i = 0; i < words; i++) {
		LE_WRITE_UINT64(dst, src[i]);
		dst += 8;
	}
	if (left) {
		uint64_t word = src[i];

		do {
			*dst++ = word & 0xff;
			word >>= 8;
		} while (--left);
	}
}
