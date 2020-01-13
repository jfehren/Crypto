/*
 * rsa-keygen.c
 *
 *  Created on: Dec 16, 2019, 2:04:58 PM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>

#include "rsa.h"

int
rsa_generate_keypair(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		/* Desired size of the modulus, in bits */
		unsigned n_size,
		/* Desired size of the public exponent, int bits. If
		 * zero, the passed value of pub->e is used. */
		unsigned e_size,
		void *random_ctx, crypto_random_func *random,
		void *progress_ctx, crypto_progress_func *progress) {
	mpz_t p1, q1, phi, tmp;

	if (e_size) {
		/* randomly generate e; ensure the size is reasonable */
		if (e_size < 16 || e_size >= n_size) {
			return 0;
		}
	} else {
		/* fixed e; ensure that it makes sense */

		/* e must be odd */
		if (!mpz_tstbit(pub->e, 0)) {
			return 0;
		}

		/* e must be >= 3 */
		if (mpz_cmp_ui(pub->e, 3) < 0) {
			return 0;
		}

		/* e must be smaller than n */
		if (mpz_sizeinbase(pub->e, 2) >= n_size) {
			return 0;
		}
	}

	if (n_size < RSA_MIN_N_BITS) {
		return 0;
	}

	mpz_init(p1); mpz_init(q1); mpz_init(phi); mpz_init(tmp);

#define PROGRESS(c) if (progress) progress(progress_ctx, c)

	/* generate primes */
	for (;;) {
		/* generate p such that gcd(p-1, e) = 1 */
		for (;;) {
			crypto_random_prime(key->p, (n_size+1)/2, 1,
					random_ctx, random, progress_ctx, progress);

			mpz_sub_ui(p1, key->p, 1);

			/* if e was given, we must choose p such that p-1 has no
			 * common factors with e */
			if (e_size) {
				break;
			}

			mpz_gcd(tmp, pub->e, p1);

			if (mpz_cmp_ui(tmp, 1) == 0) {
				break;
			} else PROGRESS('c');
		}

		PROGRESS('\n');

		/* generate q, such that gcd(q-1, e) = 1 */
		for (;;) {
			crypto_random_prime(key->q, n_size/2, 1,
					random_ctx, random, progress_ctx, progress);

			/* Very unlikely (approx. 1 in 2^(n_size/2)) */
			if (mpz_cmp(key->q, key->p) == 0) {
				continue;
			}
			mpz_sub_ui(q1, key->q, 1);

			/* if e was given, we must choose q such that q-1 has no
			 * common factors with e */
			if (e_size) {
				break;
			}

			mpz_gcd(tmp, pub->e, q1);

			if (mpz_cmp_ui(tmp, 1) == 0) {
				break;
			} else PROGRESS('c');
		}

		/* Now we have the primes. Ensure the product is the right size */
		mpz_mul(pub->n, key->p, key->q);
		assert (mpz_sizeinbase(pub->n, 2) == n_size);

		PROGRESS('\n');

		/* c = q^{-1} (mod p) */
		if (mpz_invert(key->c, key->q, key->p)) {
			/* this should succeed every time but, if it doesn't,
			 * then we must generate a new p and q */
			break;
		} else PROGRESS('?');
	}

	mpz_mul(phi, p1, q1);

	/* generate e, if it is not given to us */
	if (e_size) {
		int retried = 0;
		for (;;) {
			crypto_mpz_random_size(pub->e, e_size, random_ctx, random);

			/* Make sure it's odd and that the MSB is set */
			mpz_setbit(pub->e, 0);
			mpz_setbit(pub->e, e_size-1);

			/* Needs GMP 3, or the inverse might be negative */
			if (mpz_invert(key->d, pub->e, phi)) {
				break;
			}

			PROGRESS('e');
			retried = 1;
		}

		if (retried) PROGRESS('\n');
	} else {
		/* This must always succeed, as we already know that e doesn't
		 * have any common factors with p-1 or q-1. */
		int res = mpz_invert(key->d, pub->e, phi);
		assert(res);
	}

	/* Compute auxiliary private values */
	/* a = d (mod p-1) */
	mpz_fdiv_r(key->a, key->d, p1);
	/* b = d (mod q-1) */
	mpz_fdiv_r(key->b, key->d, q1);
	/* c = q^{-1} (mod p) was computed earlier */

	/* save the key size, in octets */
	pub->size = key->size = (n_size + 7) / 8;
	assert (pub->size >= RSA_MIN_N_OCTETS);

	mpz_clear(tmp); mpz_clear(phi); mpz_clear(q1); mpz_clear(p1);

	return 1;
}
