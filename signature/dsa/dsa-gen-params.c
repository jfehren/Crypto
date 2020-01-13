/*
 * dsa-gen-params.c
 *
 *  Created on: Oct 31, 2019, 1:09:49 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>

#include "dsa.h"

/* Valid sizes, according to FIPS 186-3 are (1024, 160), (2048, 224),
 * (2048, 256), (3072, 256), but we accept any q_bits >= 30 and any
 * p_bits >= q_bits + 30 */
int
dsa_generate_params(struct dsa_params *params, unsigned p_bits, unsigned q_bits,
		void *random_ctx, crypto_random_func *random, void *progress_ctx,
		crypto_progress_func *progress) {
	mpz_t r;
	unsigned p0_bits, a;
	if (q_bits < 30 || p_bits < q_bits + 30) {
		return 0;
	}
	mpz_init(r);

	crypto_random_prime(params->q, q_bits, 0, random_ctx, random, progress_ctx, progress);
	if (q_bits >= (p_bits + 2) / 3) {
		crypto_generate_pocklington_prime(params->p, r, p_bits, 0,
				params->q, NULL, params->q, random_ctx, random);
	} else {
		mpz_t p0, p0q;
		mpz_init(p0);
		mpz_init(p0q);

		p0_bits = (p_bits + 3) / 2;

		crypto_random_prime(p0, p0_bits, 0, random_ctx, random, progress_ctx, progress);

		if (progress) {
			progress(progress_ctx, 'q');
		}

		/* Generate p = 2*r*q*p0+1, such that 2^{n-1} < p < 2^n */
		mpz_mul(p0q, p0, params->q);
		crypto_generate_pocklington_prime(params->p, r, p_bits, 0,
				p0, params->q, p0q, random_ctx, random);
		mpz_mul(r, r, p0);

		mpz_clear(p0q);
		mpz_clear(p0);
	}
	if (progress) {
		progress(progress_ctx, 'p');
	}

	for (a = 2; ; a++) {
		mpz_set_ui(params->g, a);
		mpz_powm(params->g, params->g, r, params->p);
		if (mpz_cmp_ui(params->g, 1) > 0) {
			break;
		}
	}
	mpz_clear(r);

	if (progress) {
		progress(progress_ctx, 'g');
	}

	return 1;
}
