/*
 * blowfish-gen-tables.c
 *
 *  Created on: Feb 18, 2019, 4:08:58 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <gmp.h>

#define WORD_FORMAT "0x%08lX"
#define WORD_COLUMNS 4

typedef struct {
	mpq_t x;
	unsigned long n;
} pi_gen;

typedef pi_gen  pi_gen_t[1];
typedef pi_gen* pi_gen_ptr;

/*
 * Generate sequential hexadecimal digits of the fractional
 * part of pi using the Bailey-Borwein-Plouffe (BBP) Formula.
 */
static unsigned char
get_next_hex(pi_gen_ptr gen) {
	/*
	 * The BBP Formula for sequential digits of pi is as follows:
	 *               /               /           120*n^2 - 89*n + 16           \ \
	 *     hex(n) = [ 16*hex(n-1) + ( ----------------------------------------- ) ]
	 *               \               \ 512n^4 - 1024n^3 + 712*n^2 - 206*n + 21 / /
	 * where floor(16*hex(n)) is the n-th hexadecimal digit
	 * of the fractional part of pi.
	 *
	 *
	 * Almost all arithmetic performed here is done on mpz and mpq types,
	 * from the GNU Multiple-Precision (GMP) Library. As such, these
	 * operations are significantly more costly than simple integer
	 * arithmetic, or even floating point arithmetic. To reduce the cost
	 * of these operations as much as possible, multiplication and/or
	 * division by a power of 2 is performed using bit-shifts, rational
	 * addition and subtraction are avoided wherever possible, and integer
	 * division is avoided like the plague. Rational addition/subtraction
	 * is avoided as each one of these operations requires at best 1 GCD
	 * calculation, 3 multiplications, and 1 addition/subtraction, and at
	 * worst 2 GCD calculations, 4 division-by-GCD's, 3 multiplications,
	 * and one addition/subtraction. Unfortunately, at least one addition
	 * MUST occur (in the equation x := x*(2^4) + p), and one division
	 * MUST occur (in the equation den := floor(x*2^4)). In reducing x
	 * modulo 1, there is the option of using the modulo operation or
	 * using the subtraction operation (using the value of floor(x)). I
	 * chose to use the subtraction operation as, even if this is a costly
	 * operation, it is far more efficient than the alternative of finding
	 * the modulo by means of multiple-precision integer division, especially
	 * when considering that the value of floor(x*2^4) must be computed to
	 * get the hex-digit output, and floor-division by a power of 2 is
	 * a simple bit shift, so finding the value of floor(floor(x*2^4)/2^4)
	 * is a very fast operation.
	 */
	mpz_ptr num, den;
	mpz_t tmp;
	mpq_t p;
	unsigned long out;
	mpq_init(p);
	mpz_init(tmp);
	num = mpq_numref(p);
	den = mpq_denref(p);

	mpz_set_ui(num, 120);
	mpz_mul_ui(num, num, gen->n); mpz_sub_ui(num, num, 89);
	mpz_mul_ui(num, num, gen->n); mpz_add_ui(num, num, 16);

	mpz_set_ui(den, 512);
	mpz_mul_ui(den, den, gen->n); mpz_sub_ui(den, den, 1024);
	mpz_mul_ui(den, den, gen->n); mpz_add_ui(den, den, 712);
	mpz_mul_ui(den, den, gen->n); mpz_sub_ui(den, den, 206);
	mpz_mul_ui(den, den, gen->n); mpz_add_ui(den, den, 21);

	/*
	 * The formula for p is
	 *      /           120*n^2 - 89*n + 16           \
	 * p = ( ----------------------------------------- )
	 *      \ 512n^4 - 1024n^3 + 712*n^2 - 206*n + 21 /
	 */
	++gen->n;							/* Increment n */
	mpq_mul_2exp(gen->x, gen->x, 4);	/* Update x with next p (x = 16*x + p) */
	mpq_add(gen->x, gen->x, p);			/* Update x with next p (x = 16*x + p) */
	mpq_set(p, gen->x);					/* Set p to current x */
	mpq_mul_2exp(p, p, 4);				/* Multiply p by 16 to get the current hex digit */
	mpz_set_q(tmp, p);					/* tmp = floor(p) = floor(16 * x) */
	/* Check if x is greater than 1 */
	if (mpz_cmp_ui(tmp, 16) >= 0) {
		/* x is greater that 1 (b/c tmp = floor(p) = floor(16 * x)) */
		mpz_set(num, tmp);			/* set num to floor(p) */
		mpz_div_2exp(num, num, 4);	/* set num to floor(x) */
		mpz_set_ui(den, 1);			/* set den to 1 (now p = floor(x)/1) */
		mpq_sub(gen->x, gen->x, p);	/* x = x-floor(x) = x mod 1 */
	}
	/* return floor(p) from before reduction of x */
	out = mpz_get_ui(tmp);
	mpz_clear(tmp);
	mpq_clear(p);
	return ((unsigned char) (out & 15));	/* Reduce mod 16, just to be safe */
}

static unsigned long
get_next(pi_gen_ptr gen) {
	unsigned long out = ((unsigned long)get_next_hex(gen)) << 28;
	out |= ((unsigned long)get_next_hex(gen)) << 24;
	out |= ((unsigned long)get_next_hex(gen)) << 20;
	out |= ((unsigned long)get_next_hex(gen)) << 16;
	out |= ((unsigned long)get_next_hex(gen)) << 12;
	out |= ((unsigned long)get_next_hex(gen)) <<  8;
	out |= ((unsigned long)get_next_hex(gen)) <<  4;
	out |= ((unsigned long)get_next_hex(gen)) <<  0;
	return out;
}

int
main(int argc, char *argv[]) {
	if (argc == 1) {
		unsigned i, j, k;
		pi_gen_t gen;
		unsigned long ptable[18];
		unsigned long stable[4][256];
		mpq_init(gen->x);
		gen->n = 1;
		for (i = 0; i < 18; i++) {
			ptable[i] = get_next(gen);
		}
		for (j = 0; j < 4; j++) {
			for (i = 0; i < 0x100; i++) {
				stable[j][i] = get_next(gen);
			}
		}
		mpq_clear(gen->x);
		printf(	"/*\n"
				" * Precomputed Tables of Values for Blowfish Key Expansion\n"
				" */\n"
				"\n"
				"#include \"blowfish-internal.h\"\n"
				"\n"
				"const struct blowfish_ctx _blowfish_init_ctx = {\n\t{\n\t\t");
		for (k = 0; k < 4; k++) {
			printf("{");
			for (i = 0; i < 0x100; i += WORD_COLUMNS) {
				printf("\n\t\t\t" WORD_FORMAT, stable[k][i]);
				for (j = 1; j < WORD_COLUMNS; j++) {
					printf("," WORD_FORMAT, stable[k][i + j]);
				}
				if (i < 0x100 - WORD_COLUMNS) {
					printf(",");
				}
			}
			printf("\n\t\t}");
			if (k < 3) {
				printf(",");
			}
		}
		printf("\n\t},{"
			"\n\t\t" WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT ","
					 WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT ","
			"\n\t\t" WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT ","
					 WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT ","
			"\n\t\t" WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT ","
					 WORD_FORMAT "," WORD_FORMAT "," WORD_FORMAT "\n\t}\n};\n\n",
			ptable[ 0], ptable[ 1], ptable[ 2], ptable[ 3], ptable[ 4], ptable[ 5],
			ptable[ 6], ptable[ 7], ptable[ 8], ptable[ 9], ptable[10], ptable[11],
			ptable[12], ptable[13], ptable[14], ptable[15], ptable[16], ptable[17]);

		return EXIT_SUCCESS;
	}
	return EXIT_FAILURE;
}


