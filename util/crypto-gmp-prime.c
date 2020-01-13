/*
 * crypto-gmp-prime.c
 *
 *  Created on: Oct 31, 2019, 11:32:07 AM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "crypto-gmp.h"
#include "trialdiv.h"

/* Combined Miller-Rabin test to the base a, and checking the conditions
 * from Pocklington's theorem, nm1dq holds (n-1)/q, with q prime. */
static int
miller_rabin_pocklington(mpz_srcptr n, mpz_srcptr nm1, mpz_srcptr nm1dq, mpz_srcptr a) {
	mpz_t r, y;
	mp_bitcnt_t j, k;
	int is_prime = 0;

	/* Eliminate 0, 1, negative integers, and even integers */
	if (mpz_even_p(n) || mpz_cmp_ui(n, 3) < 0) {
		return 0;
	}

	mpz_init(r);
	mpz_init(y);

	k = mpz_scan1(nm1, 0);	/* Find first 1 bit in (n-1), starting at bit 0 */
	assert(k > 0);

	mpz_fdiv_q_2exp(r, nm1, k);	/* Factor powers of 2 from (n-1) */
	mpz_powm(y, a, r, n);	/* Compute a^{(n-1)/2^k} mod n */

	if (mpz_cmp_ui(y, 1) == 0 || mpz_cmp(y, nm1) == 0) {
		goto miller_rabin_pass;
	}

	for (j = 1; j < k; j++) {
		mpz_powm_ui(y, y, 2, n);	/* compute a^{2j*(n-1)} mod n */

		if (mpz_cmp_ui(y, 1) == 0) {
			/* n is composite */
			break;
		}

		if (mpz_cmp(y, nm1) == 0) {
			miller_rabin_pass:
			/* we know that a^{n-1} == 1 (mod n), now check that
			 * gcd(a^{(n-1)/q} - 1, n) == 1 */
			mpz_powm(y, a, nm1dq, n);	/* Compute a^{(n-1)/q} mod n */
			mpz_sub_ui(y, y, 1);	/* (a^{(n-1)/q} - 1) mod n */
			mpz_gcd(y, y, n);		/* Compute gcd(a^{(n-1)/q} - 1, n) */
			is_prime = (mpz_cmp_ui(y, 1) == 0);
			break;
		}
	}

	mpz_clear(y);
	mpz_clear(r);
	return is_prime;
}

/* The most basic variant of Pocklington's theorem:
 *
 * Assume that q^e | (n-1), with q prime. If we can find an a such that
 *
 *   a^{n-1} = 1 (mod n)
 *   gcd(a^{(n-1)/q} - 1, n) = 1
 *
 * then any prime divisor p of n satisfies p = 1 (mod q^e).
 */

/* Generate a prime number p of size bits with 2 p0q dividing (p-1).
 * p0 must be of size >= ceil(bits/3). The extra factor q can be
 * omitted (then p0 and p0q should be equal). If top_bits_set is one,
 * the topmost two bits are set to one, suitable for RSA primes. Also
 * returns r = (p-1)/p0q. */
void
crypto_generate_pocklington_prime(mpz_ptr p, mpz_ptr r, unsigned bits, int top_bits_set,
		mpz_srcptr p0, mpz_srcptr q, mpz_srcptr p0q,
		void *random_ctx, crypto_random_func *random) {
	mpz_t r_min, r_range, pm1, a, e;
	mpz_t x, y, p04;
	uint8_t buf;
	int need_square_test;
	unsigned p0_bits;

	/* Get the number of bits in p0 */
	p0_bits = mpz_sizeinbase(p0, 2);

	/* Check preconditions */
	assert(bits <= 3*p0_bits);
	assert(bits > p0_bits);

	/* Determine whether the square test is needed */
	need_square_test = (bits > 2 * p0_bits);

	if (q == NULL || mpz_cmp_ui(q, 1) == 0) {
		/* q not used, so check that p0 == p0q */
		assert(mpz_cmp(p0, p0q) == 0);
		q = NULL;	/* simplify later checks */
	} else {
		assert(mpz_cmp_ui(q, 1) > 0);
		/* Initialize e - only necessary if q > 1 */
		mpz_init(e);
	}

	if (need_square_test) {
		/* Initialize square test variables */
		mpz_init(x);
		mpz_init(y);
		mpz_init(p04);
		mpz_mul_2exp (p04, p0, 2);	/* p04 = p0^4 */
	}

	mpz_init(r_min);
	mpz_init(r_range);
	mpz_init(pm1);
	mpz_init(a);

	if (top_bits_set) {
		/* I = floor (2^{bits-3} / p0q), then 3I + 3 <= r <= 4I, with I-2 possible values. */
		mpz_set_ui(r_min, 1);
		mpz_mul_2exp(r_min, r_min, bits-3);	/* r_min = 2^{bits-3} */
		mpz_fdiv_q(r_min, r_min, p0q);	/* r_min = I = floor(2^{bits-3} / p0q) */
		mpz_sub_ui(r_range, r_min, 2);	/* r_range = I - 2 */
		mpz_mul_ui(r_min, r_min, 3);	/* r_min = 3*I */
		mpz_add_ui(r_min, r_min, 3);	/* r_min = 3*I + 3 */
	} else {
		/* I = floor (2^{bits-2} / p0q), I + 1 <= r <= 2I */
		mpz_set_ui(r_range, 1);
		mpz_mul_2exp(r_range, r_range, bits-2);	/* r_range = 2^{bits-2} */
		mpz_fdiv_q(r_range, r_range, p0q);		/* r_range = I = floor(2^{bits-2} / p0q) */
		mpz_add_ui(r_min, r_range, 1);			/* r_min = I + 1 */
	}

	/* Generate probable prime */
	for (;;) {
		/* generate r, with r_min <= r <= r_min + r_range */
		crypto_mpz_random(r, r_range, random_ctx, random);
		mpz_add(r, r, r_min);

		/* Set p = 2*r*p0q + 1 */
		mpz_mul_2exp(r, r, 1);
		mpz_mul(pm1, r, p0q);
		mpz_add_ui(p, pm1, 1);

		assert(mpz_sizeinbase(p, 2) == bits);	/* Ensure p is the correct size */

		/* quick check using GMP's miller-rabin test. Only performs trial divisions
		 * and a single iteration of the miller-rabin test */
		if (!mpz_probab_prime_p(p, 1)) {
			continue;
		}

		/* generate a small value for a */
		random(random_ctx, &buf, sizeof(buf));
		mpz_set_ui(a, buf + 2);

		if (q != NULL) {
			mpz_mul(e, r, q);
			/* Check if p is prime */
			if (!miller_rabin_pocklington(p, pm1, e, a)) {
				/* p is composite - continue */
				continue;
			}
			if (need_square_test) {
				/* e = q*r */
				mpz_tdiv_qr(x, y, e, p04);	/* (x, y) = (e / p0^4, e mod p0^4) */
				goto square_test;
			}
		} else {
			/* Check if p is prime */
			if (!miller_rabin_pocklington(p, pm1, r, a)) {
				/* p is composite - continue */
				continue;
			}
			if (need_square_test) {
				mpz_tdiv_qr(x, y, r, p04);	/* (x, y) = (r / p0^4, r mod p0^4) */
				square_test:
				/* We have r' = 2r, x = floor (r/2q) = floor(r'/2q),
				 * and y' = r' - x 4q = 2 (r - x 2q) = 2y.
				 *
				 * Then y^2 - 4x is a square iff y'^2 - 16 x is a
				 * square. */
				mpz_mul(y, y, y);
				mpz_submul_ui(y, x, 16);
				if (mpz_perfect_square_p(y)) {
					continue;
				}
			}
		}
	}

	mpz_clear(a);
	mpz_clear(pm1);
	mpz_clear(r_range);
	mpz_clear(r_min);

	if (need_square_test) {
		mpz_clear(p04);
		mpz_clear(y);
		mpz_clear(x);
	}
	if (q) {
		mpz_clear(e);
	}
}

/* Generate random prime of a given size. Maurer's algorithm (Alg.
 * 6.42 Handbook of applied cryptography), but with ratio = 1/2 (like
 * the variant in fips186-3). */
void
crypto_random_prime(mpz_ptr p, unsigned bits, int top_bits_set,
		void *random_ctx, crypto_random_func *random, void *progress_ctx,
		crypto_progress_func *progress) {
	assert (bits >= 2);
	if (bits <= TRIAL_DIV_BITS/2) {
		/* Very small prime*/
		uint16_t buf;

		assert(!top_bits_set);

		random(random_ctx, &buf, sizeof(buf));

		if (bits == 2) {
			mpz_set_ui(p, (buf & 1) ? 2 : 3);
		} else {
			unsigned first = prime_by_size[bits-3];	/* index of first "bits"-bit prime */
			unsigned choices = prime_by_size[bits-2] - first;	/* number of choices */

			mpz_set_ui(p, primes[first + (buf % choices)]);
		}
	} else if (bits <= TRIAL_DIV_BITS) {
		/* Small prime */
		unsigned long highbit;
		uint32_t x;
		unsigned j;

		assert(!top_bits_set);

		highbit = 1L << (bits - 1);

		/* loop until prime found */
		again:
		random(random_ctx, &x, sizeof(x));
		x &= (highbit - 1);
		x |= highbit | 1;

		/* loop through prime sieve, checking divisibility by every prime
		 * that is <= the square root of x */
		for (j = 0; prime_square[j] <= x; j++) {
			uint32_t q = (x * trial_div_table[j].inverse) & TRIAL_DIV_MASK;
			if (q <= trial_div_table[j].limit) {
				goto again;
			}
		}
		mpz_set_ui(p, x);
	} else {
		/* Large prime */
		mpz_t q, r;

		mpz_init (q);
		mpz_init (r);

		/* Bit size ceil(k/2) + 1, slightly larger than used in Alg. 4.62
		in Handbook of Applied Cryptography (which seems to be
		incorrect for odd k). */
		crypto_random_prime(q, (bits+3)/2, 0, random_ctx, random,
				progress_ctx, progress);

		crypto_generate_pocklington_prime(p, r, bits, top_bits_set,
				random_ctx, random, q, NULL, q);

		if (progress) {
			progress(progress_ctx, 'x');
		}

		mpz_clear (q);
		mpz_clear (r);
	}
}

