/*
 * aes-gen-tables.c
 *
 *  Created on: Feb 18, 2019, 12:27:13 PM
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

#define ROTL32(x,n)	 (((((x)<<(n))) | (((x)>>((-(n))&31))&((1<<(n))-1))) & 0xFFFFFFFF)

static unsigned char sbox[0x100];
static unsigned char ibox[0x100];

static unsigned char gf2_log[0x100];
static unsigned char gf2_exp[0x100];

static unsigned long dtable[4][0x100];
static unsigned long itable[4][0x100];
static unsigned long mtable[4][0x100];

/*
 * Perform multiplication by 2 in the Galois Field GF(2^8) modulo the irreducible
 * polynomial x^8 + x^4 + x^3 + x + 1 with coefficients in GF(2).
 */
static unsigned xtime(unsigned x) {
	assert(x < 0x100);
	x <<= 1;
	if (x & 0x100) {
		x ^= 0x11b;
	}
	assert(x < 0x100);
	return x;
}

/*
 * Compute an exponentiation and logarithm tables using the generator
 * element x + 1 for the Galois Field GF(2^8) with characteristic polynomial
 * x^8 + x^4 + x^3 + x + 1 with coefficients in GF(2) for use in fast
 * multiplication of two elements in the aforementioned field.
 */
static void compute_exp_log(void) {
	unsigned i, x;
	memset(gf2_log, 0, 0x100);
	for (i = 0, x = 1; i < 0x100; i++, x ^= xtime(x)) {
		gf2_exp[i] = x;
		gf2_log[x] = i;
	}
	/* log(0) is undefined */
	gf2_log[0] = 0;
	/* log(1) = 0xFF, according to the loop above, which is correct, but
	   log(1) = 0x00 is equivalent, and makes more sense */
	gf2_log[1] = 0;
}

/*
 * Perform multiplication of two elements of the Galois Field GF(2^8) modulo
 * the irreducible polynomial x^8 + x^4 + x^3 + x + 1 with coefficients in
 * GF(2) by exponentiating the sum of the logarithms of the elements.
 */
static unsigned mult(unsigned a, unsigned b) {
	return (a && b) ? gf2_exp[ (gf2_log[a] + gf2_log[b]) % 255] : 0;
}

/*
 * Find the multiplicative inverse of an element of the Galois Field GF(2^8)
 * modulo the irreducible polynomial x^8 + x^4 + x^3 + x + 1 with coefficients
 * in GF(2) by exponentiating 255 minus the logarithm of the element.
 */
static unsigned invert(unsigned x) {
	return x ? gf2_exp[255 - gf2_log[x]] : 0;
}

/*
 * Perform the second step of the AES substitution routine for
 * argument as follows:
 *     sub(x)_i = c_i XOR x_i XOR x_((i+4) mod 8) XOR x_((i+5) mod 8) XOR x_((i+6) mod 8) XOR X_((i+7) mod 8)
 * where a_b is the b-th bit of a, with bit zero being the least significant,
 * and c is the hexadecimal value 63.
 */
static unsigned affine(unsigned x) {
	return (0x63^x^(x>>4)^(x<<4)^(x>>5)^(x<<3)^(x>>6)^(x<<2)^(x>>7)^(x<<1)) & 0xFF;
}

/*
 * Precompute all 256 possible substitution values, where the substituted value
 * of x is sub(invert(x)), and store them in the S-Box, and their inverse in the
 * Inverse S-Box.
 */
static void compute_sbox(void) {
	for (unsigned i = 0, x; i < 0x100; i++) {
		x = affine(invert(i));
		sbox[i] = x;
		ibox[x] = i;
	}
}

/*
 * Precompute the MixColumns routine for the values of the S-Box by multiplying
 * each value by the polynomial 3*x^3 + 1*x^2 + 1*x + 2 with coefficients in
 * the Galois Field GF(2^8) that has the characteristic polynomial
 * x^8 + x^4 + x^3 + x + 1 with coefficients in GF(2).
 */
static void compute_dtable(void) {
	unsigned i, x;
	unsigned long t;
	for (i = 0; i < 0x100; i++) {
		x = sbox[i];
		t = (mult(x, 0x3) << 24) | (mult(x, 0x1) << 16) | (mult(x, 0x1) << 8) | mult(x, 0x2);
		dtable[0][i] = t; t = ROTL32(t, 8);
		dtable[1][i] = t; t = ROTL32(t, 8);
		dtable[2][i] = t; t = ROTL32(t, 8);
		dtable[3][i] = t;
	}
}

/*
 * Precompute the InvMixColumns routine for the values of the Inverse S-Box by
 * multiplying each value by the polynomial 11*x^3 + 13*x^2 + 9*x + 14 with
 * coefficients in the Galois Field GF(2^8) that has the characteristic polynomial
 * x^8 + x^4 + x^3 + x + 1 with coefficients in GF(2).
 */
static void
compute_itable(void) {
	unsigned i, x;
	unsigned long t;
	for (i = 0; i < 0x100; i++) {
		x = ibox[i];
		t = (mult(x, 0xB) << 24) | (mult(x, 0xD) << 16) | (mult(x, 0x9) << 8) | mult(x, 0xE);
		itable[0][i] = t; t = ROTL32(t, 8);
		itable[1][i] = t; t = ROTL32(t, 8);
		itable[2][i] = t; t = ROTL32(t, 8);
		itable[3][i] = t;
	}
}

/*
 * Precompute the InvMixColumns routine without substitution by
 * multiplying all 256 elements of the Galois Field GF(2^8) by the
 * polynomial 11*x^3 + 13*x^2 + 9*x + 14 with coefficients in the
 * Galois Field GF(2^8) that has the characteristic polynomial
 * x^8 + x^4 + x^3 + x + 1 with coefficients in GF(2).
 */
static void
compute_mtable(void) {
	unsigned i;
	unsigned long t;
	for (i = 0; i < 0x100; i++) {
		t = (mult(i, 0xB) << 24) | (mult(i, 0xD) << 16) | (mult(i, 0x9) << 8) | mult(i, 0xE);
		mtable[0][i] = t; t = ROTL32(t, 8);
		mtable[1][i] = t; t = ROTL32(t, 8);
		mtable[2][i] = t; t = ROTL32(t, 8);
		mtable[3][i] = t;
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
display_table(const char* name, unsigned long (*table)[0x100]) {
	unsigned i, j, k;
	printf("const uint32_t %s[4][0x100] = {\n\t", name);
	for (k = 0; k < 4; k++) {
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
		if (k < 3) {
			printf(",");
		}
	}
	printf("\n};\n\n");
}

static void
display_polynomial(const unsigned *p) {
	printf("(%x x^3 + %x x^2 + %x x + %x)", p[3], p[2], p[1], p[0]);
}

int
main(int argc, char *argv[]) {
	compute_exp_log();
	if (argc == 1) {
		compute_sbox();
		compute_dtable();
		compute_itable();
		compute_mtable();

		printf(	"/*\n"
				" * Precomputed Tables of Values for AES\n"
				" * encryption, decryption, and key expansion\n"
				" */\n"
				"\n"
				"#include \"aes-internal.h\"\n"
				"\n");

		display_byte_table("_aes_gf2_log", gf2_log);
		display_byte_table("_aes_gf2_exp", gf2_exp);

		display_byte_table("_aes_sbox", sbox);
		display_byte_table("_aes_ibox", ibox);

		display_table("_aes_dtable", dtable);
		display_table("_aes_itable", itable);
		display_table("_aes_mtable", mtable);

		return EXIT_SUCCESS;
	} else if (argc == 2) {
		unsigned a, b, c, u, a1, b1;
		for (a = 1; a < 0x100; a++) {
			a1 = invert(a);
			if (a1 == 0) {
				printf("invert(%x) = 0 !\n", a);
			}
			u = mult(a, a1);
			if (u != 1) {
				printf("invert(%x) = %x; product = %x\n", a, a1, u);
			}
			for (b = 1; b<0x100; b++) {
				b1 = invert(b);
				c = mult(a, b);

				if (c == 0) {
					printf("%x x %x = 0\n", a, b);
				}
				u = mult(c, a1);
				if (u != b) {
					printf("%x x %x = %x, invert(%x) = %x, %x x %x = %x\n",
							a, b, c, a, a1, c, a1, u);
				}
				u = mult(c, b1);
				if (u != a) {
					printf("%x x %x = %x, invert(%x) = %x, %x x %x = %x\n",
							a, b, c, b, b1, c, b1, u);
				}
			}
		}
		return EXIT_SUCCESS;
	} else if (argc == 4) {
		unsigned a, b, c;
		int op = argv[2][0];
		a = strtoul(argv[1], NULL, 16);
		b = strtoul(argv[3], NULL, 16);
		switch (op) {
			case '+':
			case '-':
				c = a ^ b;
				break;
			case '*':
			case 'x':
				c = mult(a,b);
				break;
			case '/':
				c = mult(a, invert(b));
				break;
			default:
				return 1;
		}
		printf("%x %c %x = %x\n", a, op, b, c);
		return EXIT_SUCCESS;
#if 0
	} else if (argc == 5) {
		/* Compute gcd(a, x^4+1) */
		unsigned d[4];
		unsigned u[4];

		for (unsigned i = 0; i<4; i++) {
			a[i] = strtoul(argv[1+i], NULL, 16);
		}
		return EXIT_SUCCESS;
#endif
	} else if (argc == 9) {
		unsigned a[4], b[4], c[4], i;
		for (i = 0; i<4; i++) {
			a[i] = strtoul(argv[1+i], NULL, 16);
			b[i] = strtoul(argv[5+i], NULL, 16);
		}

		c[0] = mult(a[0],b[0])^mult(a[3],b[1])^mult(a[2],b[2])^mult(a[1],b[3]);
		c[1] = mult(a[1],b[0])^mult(a[0],b[1])^mult(a[3],b[2])^mult(a[2],b[3]);
		c[2] = mult(a[2],b[0])^mult(a[1],b[1])^mult(a[0],b[2])^mult(a[3],b[3]);
		c[3] = mult(a[3],b[0])^mult(a[2],b[1])^mult(a[1],b[2])^mult(a[0],b[3]);

		display_polynomial(a); printf(" * "); display_polynomial(b);
		printf(" = "); display_polynomial(c); printf("\n");
		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


