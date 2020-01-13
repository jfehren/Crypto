/*
 * gcm-gen-tables.c
 *
 *  Created on: Mar 8, 2019, 2:27:02 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define GHASH_POLYNOMIAL 0xE1

#define WORD_FORMAT "W(%02X,%02X)"
#define WORD_COLUMNS 8

static unsigned
reduce(unsigned x) {
	unsigned p = GHASH_POLYNOMIAL << 1;
	unsigned y = 0;
	assert (0 <= x && x < 0x10000);
	for (; x != 0; x >>= 1, p <<= 1) {
		if ((x & 1) != 0) {
			y ^= p;
		}
	}
	assert (0 <= y && y < 0x10000);
	return y;
}

static void
display_table(unsigned bits) {
	unsigned i, j, x, shift, length;
	shift = 8 - bits;
	length = 1 << bits;
	printf("static const uint16_t shift_table[0%03o] = {", length);
	if (length < WORD_COLUMNS) {
		x = reduce(0);
		printf("\n\t" WORD_FORMAT, x >> 8, x & 0xff);
		for (i = 1; i < length; i++) {
			x = reduce(i << shift);
			printf("," WORD_FORMAT, x >> 8, x & 0xff);
		}
	} else {
		for (i = 0; i < length; i += WORD_COLUMNS) {
			x = reduce(i << shift);
			printf("\n\t" WORD_FORMAT, x >> 8, x & 0xff);
			for (j = 1; j < WORD_COLUMNS; j++) {
				x = reduce((i + j) << shift);
				printf("," WORD_FORMAT, x >> 8, x & 0xff);
			}
			if (i < length - WORD_COLUMNS) {
				printf(",");
			}
		}
	}
	printf("\n};\n");
}

int
main(void) {
	printf(	"/*\n"
			" * Precomputed Multiplication Tables for the GHASH\n"
			" * primitive of the GCM mode of operation.\n"
			" */\n"
			"\n"
			"#if WORDS_BIGENDIAN\n"
			"#define W(left,right) (0x##left##right)\n"
			"#else\n"
			"#define W(left,right) (0x##right##left)\n"
			"#endif\n"
			"\n"
			"#  if GCM_TABLE_BITS == 8\n"
			"\n");
	display_table(8);
	printf(	"\n"
			"#elif GCM_TABLE_BITS == 4\n"
			"\n");
	display_table(4);
	printf(	"\n"
			"#elif GCM_TABLE_BITS == 2\n"
			"\n");
	display_table(2);
	printf(	"\n"
			"#endif\n"
			"\n"
			"#undef W\n"
			"\n"
			"\n");
	return EXIT_SUCCESS;
}

