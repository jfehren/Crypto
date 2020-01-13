/*
 * twofish-gen-tables.c
 *
 *  Created on: Mar 4, 2019, 5:47:06 PM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if 1
# define BYTE_FORMAT "0x%02X"
# define BYTE_COLUMNS 8
#else
# define BYTE_FORMAT "%3d"
# define BYTE_COLUMNS 0x10
#endif

#define WORD_FORMAT "0x%08lX"
#define WORD_COLUMNS 4

static const uint8_t q0_t0[16] = {
	0x8,0x1,0x7,0xD,0x6,0xF,0x3,0x2,
	0x0,0xB,0x5,0x9,0xE,0xC,0xA,0x4
};
static const uint8_t q0_t1[16] = {
	0xE,0xC,0xB,0x8,0x1,0x2,0x3,0x5,
	0xF,0x4,0xA,0x6,0x7,0x0,0x9,0xD
};
static const uint8_t q0_t2[16] = {
	0xB,0xA,0x5,0xE,0x6,0xD,0x9,0x0,
	0xC,0x8,0xF,0x3,0x2,0x4,0x7,0x1
};
static const uint8_t q0_t3[16] = {
	0xD,0x7,0xF,0x4,0x1,0x2,0x6,0xE,
	0x9,0xB,0x3,0x0,0x8,0x5,0xC,0xA
};

static const uint8_t q1_t0[16] = {
	0x2,0x8,0xB,0xD,0xF,0x7,0x6,0xE,
	0x3,0x1,0x9,0x4,0x0,0xA,0xC,0x5
};
static const uint8_t q1_t1[16] = {
	0x1,0xE,0x2,0xB,0x4,0xC,0x3,0x7,
	0x6,0xD,0xA,0x5,0xF,0x9,0x0,0x8
};
static const uint8_t q1_t2[16] = {
	0x4,0xC,0x7,0x5,0x1,0x6,0x9,0xA,
	0x0,0xE,0xD,0x8,0x2,0xB,0x3,0xF
};
static const uint8_t q1_t3[16] = {
	0xB,0x9,0x5,0x1,0xC,0x3,0xD,0xE,
	0x6,0x4,0x7,0xF,0x2,0x0,0x8,0xA
};

#define GF_MUL2(x,poly) ((unsigned)((((x) & 0x80) != 0) ? (((x) << 1) ^ poly) : ((x) << 1)))
#define ROR4(x) ((((x) & 0xE) >> 1) | (((x) & 0x1) << 3))

#define GF_MULT(a,b) \
	(((a) == 0 || (b) == 0) ? 0 : exp[((unsigned)log[a] + (unsigned)log[b]) % 255])

static const uint8_t rs_matrix[4][8] = {
	{ 0x01, 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E },
	{ 0xA4, 0x56, 0x82, 0xF3, 0x1E, 0xC6, 0x68, 0xE5 },
	{ 0x02, 0xA1, 0xFC, 0xC1, 0x47, 0xAE, 0x3D, 0x19 },
	{ 0xA4, 0x55, 0x87, 0x5A, 0x58, 0xDB, 0x9E, 0x03 }
};

static const uint8_t mds_matrix[4][4] = {
	{ 0x01, 0xEF, 0x5B, 0x5B },
	{ 0x5B, 0xEF, 0xEF, 0x01 },
	{ 0xEF, 0x5B, 0x01, 0xEF },
	{ 0xEF, 0x01, 0xEF, 0x5B }
};

static uint8_t exp[0x100], log[0x100];
static uint8_t Q0[0x100], Q1[0x100];
static uint32_t MDS[4][0x100], RS[8][0x100];

static void
compute_exp_log_MDS() {
	unsigned i, x;
	memset(exp, 0, sizeof(exp));
	memset(log, 0, sizeof(log));
	for (i = 0, x = 1; i < 0x100; i++, x ^= GF_MUL2(x, 0x169)) {
		exp[i] = x;
		log[x] = i;
	}
	log[0] = log[1] = 0;
}

static void
compute_exp_log_RS() {
	unsigned i, x;
	memset(exp, 0, sizeof(exp));
	memset(log, 0, sizeof(log));
	for (i = 0, x = 1; i < 0x100; i++, x ^= GF_MUL2(x, 0x14D)) {
		exp[i] = x;
		log[x] = i;
	}
	log[0] = log[1] = 0;
}

static void
compute_Q() {
	uint8_t x0, y0, x1, y1, a0, b0, a1, b1;
	for (unsigned i = 0; i < 0x100; i++) {
		x0 = (uint8_t)i;
		a0 = x0 / 16;
		b0 = x0 % 16;
		x0 = a0 ^ b0;
		y0 = a0 ^ ROR4(b0) ^ ((8*a0) % 16);
		a0 = q0_t0[x0];
		a1 = q1_t0[x0];
		b0 = q0_t1[y0];
		b1 = q1_t1[y0];
		x0 = a0 ^ b0;
		x1 = a1 ^ b1;
		y0 = a0 ^ ROR4(b0) ^ ((8*a0) % 16);
		y1 = a1 ^ ROR4(b1) ^ ((8*a1) % 16);
		a0 = q0_t2[x0];
		a1 = q1_t2[x1];
		b0 = q0_t3[y0];
		b1 = q1_t3[y1];
		Q0[i] = a0 + 16*b0;
		Q1[i] = a1 + 16*b1;
	}
}

static void
compute_MDS() {
	unsigned i, j;
	compute_exp_log_MDS();
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 0x100; j++) {
			MDS[i][j] = (GF_MULT(j, mds_matrix[0][i]) <<  0) |
						(GF_MULT(j, mds_matrix[1][i]) <<  8) |
						(GF_MULT(j, mds_matrix[2][i]) << 16) |
						(GF_MULT(j, mds_matrix[3][i]) << 24);
		}
	}
}

static void
compute_RS() {
	unsigned i, j;
	compute_exp_log_RS();
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 0x100; j++) {
			RS[i][j] =  (GF_MULT(j, rs_matrix[0][i]) <<  0) |
						(GF_MULT(j, rs_matrix[1][i]) <<  8) |
						(GF_MULT(j, rs_matrix[2][i]) << 16) |
						(GF_MULT(j, rs_matrix[3][i]) << 24);
		}
	}
}

static void
display_byte_table(const char* name, unsigned char *table) {
	unsigned i, j;
	printf("const uint8_t %s[0x100] = {", name);
	for (i = 0; i < 0x100; i += BYTE_COLUMNS) {
		printf("\n\t" BYTE_FORMAT, table[i]);
		for (j = 1; j < BYTE_COLUMNS; j++) {
			printf("," BYTE_FORMAT, table[i + j]);
		}
		if (i < 0x100 - BYTE_COLUMNS) {
			printf(",");
		}
	}
	printf("\n};\n\n");
}

static void
display_table(const char* name, unsigned long (*table)[0x100], unsigned columns) {
	unsigned i, j, k;
	printf("const uint32_t %s[%u][0x100] = {\n\t", name, columns);
	for (k = 0; k < columns; k++) {
		printf("{");
		for (i = 0; i < 0x100; i += WORD_COLUMNS) {
			printf("\n\t\t" WORD_FORMAT, table[k][i]);
			for (j = 1; j < WORD_COLUMNS; j++) {
				printf("," WORD_FORMAT, table[k][i + j]);
			}
			if (i < 0x100 - WORD_COLUMNS) {
				printf(",");
			}
		}
		printf("\n\t}");
		if (k < columns-1) {
			printf(",");
		}
	}
	printf("\n};\n\n");
}

int
main(void) {
	compute_Q();
	compute_MDS();
	compute_RS();

	printf(	"/*\n"
			" * Precomputed Tables of Values for Twofish key expansion\n"
			" */\n"
			"\n"
			"#include \"twofish-internal.h\"\n"
			"\n");

	display_byte_table("_twofish_Q0", Q0);
	display_byte_table("_twofish_Q1", Q1);
	display_table("_twofish_MDS", MDS, 4);
	display_table("_twofish_RS", RS, 8);

	return EXIT_SUCCESS;
}

