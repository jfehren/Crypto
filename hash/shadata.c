/*
 * shadata.c
 *
 *  Created on: Apr 24, 2019, 3:27:41 PM
 *      Author: Joshua Fehrenbach
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>	/* needed to generate constants for SHA-512 */

#include "macros.h"

static const unsigned primes[80] = {
	2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41,
	43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97,
	101, 103, 107, 109, 113, 127, 131, 137, 139, 149,
	151, 157, 163, 167, 173, 179, 181, 191, 193, 197,
	199, 211, 223, 227, 229, 233, 239, 241, 251, 257,
	263, 269, 271, 277, 281, 283, 293, 307, 311, 313,
	317, 331, 337, 347, 349, 353, 359, 367, 373, 379,
	383, 389, 397, 401, 409
};

static void
gen_sha256() {
	static const double third = 1.0/3;
	double root, fraction, value;
	unsigned i, j;

	/* Generate initial hash value for SHA-224 as the low 32 bits of the first 32 bits
	 * of the fractional part of the square root of the first 8 prime numbers */
	printf("static const uint32_t SHA224_H[8] = {");
	{
		/* Compute the square root of p by computing sqrt(p*((2^64)^2)) / (2^64),
		 * except rather than divide by 2^64, take the remainder of division by 2^32
		 * so that the result is equal to the low 32 bits of the first 64 bits of the
		 * truncated fractional part of the square root of p */
		mpz_t val, rem, mod;
		mpz_init(val);
		mpz_init(rem);
		mpz_init_set_ui(mod, 1);

		mpz_mul_2exp(mod, mod, 32);		/* mod = 2^32 */
		for (i = 0; i < 8; i += 4) {
			mpz_set_ui(val, primes[i+8]);
			mpz_mul_2exp(val, val, 2*64);
			mpz_sqrt(val, val);
			mpz_tdiv_r(rem, val, mod);
			/* rem = (sqrt(primes[i+8])*2^64) mod 2^32 */
			printf("\n\t0x%08lX", mpz_get_ui(rem));

			for (j = 1; j < 4; j++) {
				mpz_set_ui(val, primes[i+j+8]);
				mpz_mul_2exp(val, val, 2*64);
				mpz_sqrt(val, val);
				mpz_tdiv_r(rem, val, mod);
				/* rem = (sqrt(primes[i+j+8])*2^64) mod 2^32 */
				printf(",0x%08lX", mpz_get_ui(rem));
			}
			if (i < 4) {
				printf(",");
			}
		}
		printf("\n};\n\n");
		mpz_clear(val);
		mpz_clear(rem);
		mpz_clear(mod);
	}

	/* Generate initial hash value for SHA-256 as the first 32 bits of the
	 * fractional part of the square root of the first 8 prime numbers */
	printf("static const uint32_t SHA256_H[8] = {");
	for (i = 0; i < 8; i += 4) {
		root = sqrt(primes[i]);
		fraction = root - floor(root);
		value = floor(ldexp(fraction, 32));

		printf("\n\t0x%08lX", (unsigned long) value);
		for (j = 1; j < 4; j++) {
			root = sqrt(primes[i+j]);
			fraction = root - floor(root);
			value = floor(ldexp(fraction, 32));

			printf(",0x%08lX", (unsigned long) value);
		}
		if (i < 4) {
			printf(",");
		}
	}
	printf("\n};\n\n");

	/* Generate the constants used in SHA-224 and SHA-256 as the first 32 bits of the
	 * fractional part of the cube root of the first 64 prime numbers */
	printf("static const uint32_t SHA256_K[64] = {");
	for (i = 0; i < 64; i += 4) {
		root = pow(primes[i], third);
		fraction = root - floor(root);
		value = floor(ldexp(fraction, 32));

		printf("\n\t0x%08lX", (unsigned long) value);
		for (j = 1; j < 4; j++) {
			root = pow(primes[i + j], third);
			fraction = root - floor(root);
			value = floor(ldexp(fraction, 32));

			printf(",0x%08lX", (unsigned long) value);
		}
		if (i < 60) {
			printf(",");
		}
	}
	printf("\n};\n");
}

static void
gen_sha512_t(unsigned long long *H, const unsigned long long *sha512_H,
		const unsigned long long *K, unsigned t) {
	union {
		unsigned char msg[128];
		unsigned long long data[16];
	} data;
	unsigned long long a, b, c, d, e, f, g, h, t1, t2;
	unsigned len, i;
#define rotr(x,n)	(((x) >> (n)) | ((x) << ((-(n)) & 63)))
#define ch(x,y,z)	(((x) & (y)) ^ ((~(x)) & (z)))
#define maj(x,y,z)	(((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define S0(x)		(rotr(x, 28) ^ rotr(x, 34) ^ rotr(x, 39))
#define S1(x)		(rotr(x, 14) ^ rotr(x, 18) ^ rotr(x, 41))
#define s0(x)		(rotr(x,  1) ^ rotr(x,  8) ^ ((x) >> 7))
#define s1(x)		(rotr(x, 19) ^ rotr(x, 61) ^ ((x) >> 6))
#define EXPAND(W,i)	\
	(W[(i) & 15 ] += (s1(W[((i)-2) & 15]) + W[((i)-7) & 15] + s0(W[((i)-15) & 15])))
	/* Generate the input message */
	len = sprintf(data.msg, "SHA-512/%u", t);
	data.msg[len] = (unsigned char)0x80;
	memset(data.msg + len + 1, 0, 128 - len - 2);
	data.msg[127] = (unsigned char)(len << 3);	/* 72 <= (len * 8) <= 88 < 256 */
	/* Load the initial hash state */
	a = sha512_H[0];
	b = sha512_H[1];
	c = sha512_H[2];
	d = sha512_H[3];
	e = sha512_H[4];
	f = sha512_H[5];
	g = sha512_H[6];
	h = sha512_H[7];
	t1 = t2 = 0;
	/* Read the message into the data array */
	for (i = 0; i < 16; i++) {
		data.data[i] = BE_READ_UINT64(data.msg + i*8);
	}
	/* Perform first 16 subrounds on original data */
	for (i = 0; i < 16; i++) {
		t1 = h + K[i] + data.data[i] + S1(e) + ch(e, f, g);
		t2 = S0(a) + maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
	/* Perform remaining 64 subrounds on expanded data */
	for (; i < 80; i++) {
		t1 = h + K[i] + EXPAND(data.data, i) + S1(e) + ch(e, f, g);
		t2 = S0(a) + maj(a, b, c);
		h = g;
		g = f;
		f = e;
		e = d + t1;
		d = c;
		c = b;
		b = a;
		a = t1 + t2;
	}
	/* Write out generated initial hash value */
	H[0] = sha512_H[0] + a;
	H[1] = sha512_H[1] + b;
	H[2] = sha512_H[2] + c;
	H[3] = sha512_H[3] + d;
	H[4] = sha512_H[4] + e;
	H[5] = sha512_H[5] + f;
	H[6] = sha512_H[6] + g;
	H[7] = sha512_H[7] + h;
}

static void
gen_sha512() {
	/* Compute the cube root of p by computing cbrt(p*((2^64)^3)) / (2^64),
	 * except rather than divide by 2^64, take the remainder of division by 2^64
	 * so that the result is equal to the first 64 bits of the truncated
	 * fractional part of the cube root of p */
	unsigned i, j;
	mpz_t val, rem, mod;
	unsigned long long H[8], H224[8], H256[8];
	unsigned long long K[80];
	mpz_init(val);
	mpz_init(rem);
	mpz_init_set_ui(mod, 1);

	mpz_mul_2exp(mod, mod, 64);		/* mod = 2^64 */

	/* Generate initial hash value for SHA-512 as the first 64 bits of the fractional
	 * part of the square root of the first 8 prime numbers */
	for (i = 0; i < 8; i++) {
		mpz_set_ui(val, primes[i]);
		mpz_mul_2exp(val, val, 2*64);
		mpz_sqrt(val, val);
		mpz_tdiv_r(rem, val, mod);
		/* rem = (sqrt(primes[i])*2^64) mod 2^64 */
		mpz_export(&H[i], NULL, -1, 8, 0, 0, rem);
	}

	/* Generate the constants used in SHA-384, SHA-512, SHA-512/224, and SHA-512/256
	 * as the first 64 bits of the fractional part of the cube root of the first
	 * 80 prime numbers */
	for (i = 0; i < 80; i++) {
		mpz_set_ui(val, primes[i]);
		mpz_mul_2exp(val, val, 3*64);
		mpz_root(val, val, 3);
		mpz_tdiv_r(rem, val, mod);
		/* rem = (cbrt(primes[i])*2^64) mod 2^64 */
		mpz_export(&K[i], NULL, -1, 8, 0, 0, rem);
	}

	/* Generate the initial hash values for SHA-512/224 and SHA-512/256 */
	gen_sha512_t(H224, H, K, 224);
	gen_sha512_t(H256, H, K, 256);

	/* Generate initial hash value for SHA-384 as the first 64 bits of the fractional
	 * part of the square root of the 9th through 16th prime numbers */
	printf("static const uint64_t SHA384_H[8] = {");
	for (i = 0; i < 8; i += 2) {
		mpz_set_ui(val, primes[i+8]);
		mpz_mul_2exp(val, val, 2*64);
		mpz_sqrt(val, val);
		mpz_tdiv_r(rem, val, mod);
		/* rem = (sqrt(primes[i+8])*2^64) mod 2^64 */
		gmp_printf("\n\t0x%016ZX", rem);

		mpz_set_ui(val, primes[i+9]);
		mpz_mul_2exp(val, val, 2*64);
		mpz_sqrt(val, val);
		mpz_tdiv_r(rem, val, mod);
		/* rem = (sqrt(primes[i+9])*2^64) mod 2^64 */
		gmp_printf(",0x%016ZX", rem);
		if (i < 6) {
			printf(",");
		}
	}
	printf("\n};\n\n");
	mpz_clear(val);
	mpz_clear(rem);
	mpz_clear(mod);

	/* Print the initial hash value for SHA-512 */
	printf("static const uint64_t SHA512_H[8] = {");
	for (i = 0; i < 8; i += 2) {
		printf("\n\t0x%016llX,0x%016llX", H[i+0], H[i+1]);
		if (i < 6) {
			printf(",");
		}
	}
	printf("\n};\n\n");

	/* Print the initial hash value for SHA-512/224 */
	printf("static const uint64_t SHA512_224_H[8] = {");
	for (i = 0; i < 8; i += 2) {
		printf("\n\t0x%016llX,0x%016llX", H224[i+0], H224[i+1]);
		if (i < 6) {
			printf(",");
		}
	}
	printf("\n};\n\n");

	/* Print the initial hash value for SHA-512/256 */
	printf("static const uint64_t SHA512_256_H[8] = {");
	for (i = 0; i < 8; i += 2) {
		printf("\n\t0x%016llX,0x%016llX", H256[i+0], H256[i+1]);
		if (i < 6) {
			printf(",");
		}
	}
	printf("\n};\n\n");

	/* Print the constants used in SHA-384, SHA-512, SHA-512/224, and SHA-512/256 */
	printf("static const uint64_t SHA512_K[80] = {");
	for (i = 0; i < 80; i += 2) {
		printf("\n\t0x%016llX,0x%016llX", K[i+0], K[i+1]);
		if (i < 78) {
			printf(",");
		}
	}
	printf("\n};\n");
}

int main(int argc, char *argv[]) {
	switch (argc) {
		case 1:
			printf(	"/*\n"
					" * Precomputed Tables of Values for SHA-224, SHA-256,\n"
					" * SHA-384, SHA-512, SHA-512/224, and SHA-512/256.\n"
					" */\n"
					"\n"
					"#include <stdint.h>\n"
					"\n");
			gen_sha256();
			printf(	"\n"
					"\n");
			gen_sha512();
			printf(	"\n");
			break;
		case 2:
			if (strcmp(argv[1], "SHA256") == 0 || strcmp(argv[1], "SHA224") == 0 ||
				strcmp(argv[1], "sha256") == 0 || strcmp(argv[1], "sha224") == 0 ||
				strcmp(argv[1], "256") == 0 || strcmp(argv[1], "224") == 0) {
				printf(	"/*\n"
						" * Precomputed Tables of Values for SHA-224 and SHA-256.\n"
						" */\n"
						"\n"
						"#include <stdint.h>\n"
						"\n");
				gen_sha256();
				printf(	"\n");
			} else if (strcmp(argv[1], "SHA512") == 0 || strcmp(argv[1], "SHA384") == 0 ||
					strcmp(argv[1], "sha512") == 0 || strcmp(argv[1], "sha384") == 0 ||
					strcmp(argv[1], "512") == 0 || strcmp(argv[1], "384") == 0) {
				printf(	"/*\n"
						" * Precomputed Tables of Values for SHA-384, SHA-512,\n"
						" * SHA-512/224, and SHA-512/256.\n"
						" */\n"
						"\n"
						"#include <stdint.h>\n"
						"\n");
				gen_sha512();
				printf(	"\n");
			} else {
				fprintf(stderr, "Invalid Argument: %s\n", argv[1]);
				abort();
			}
			break;
		default:
			fprintf(stderr, "Invalid Number of Arguments\n");
			abort();
	}
	return EXIT_SUCCESS;
}

