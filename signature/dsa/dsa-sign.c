/*
 * dsa-sign.c
 *
 *  Created on: Oct 31, 2019, 12:43:46 PM
 *      Author: Joshua Fehrenbach
 */

#include "dsa.h"
#include "dsa-hash.h"

int
dsa_sign(const struct dsa_params *params, mpz_srcptr x, const uint8_t *digest,
		size_t digest_size, struct dsa_signature *sig,
		void *random_ctx, crypto_random_func *random) {
	mpz_t k, h, tmp;
	int res;

	/* ensure p is odd */
	if (mpz_even_p(params->p)) {
		return 0;
	}

	/* select k, 0 < k < q, randomly */
	mpz_init_set(tmp, params->q);
	mpz_sub_ui(tmp, tmp, 1);	/* tmp = q-1 */
	mpz_init(k);
	crypto_mpz_random(k, tmp, random_ctx, random);	/* 0 <= k < q-1 */
	mpz_add_ui(k, k, 1);	/* 0 < k < q */

	/* Compute r = (g^k (mod p)) (mod q) */
	mpz_powm_sec(tmp, params->g, k, params->p);
	mpz_fdiv_r(sig->r, tmp, params->q);

	/* Compute hash */
	mpz_init(h);
	_dsa_hash(h, mpz_sizeinbase(params->q, 2), digest, digest_size);

	/* Compute k^-1 (mod q) */
	if (mpz_invert(k, k, params->q)) {
		/* Compute signature s = (k^-1)*(h + x*r) (mod q) */
		mpz_mul(tmp, sig->r, x);		/* r*x */
		mpz_fdiv_r(tmp, tmp, params->q);	/* r*x (mod q) */
		mpz_add(tmp, tmp, h);				/* h + r*x (mod q) */
		mpz_mul(tmp, tmp, k);				/* (k^-1)*(h + r*x (mod q)) */
		mpz_fdiv_r(sig->s, tmp, params->q);	/* (k^-1)*(h + r*x) (mod q) */
		res = 1;
	} else {
		/* key is invalid??? */
		res = 0;
	}

	mpz_clear(h);
	mpz_clear(k);
	mpz_clear(tmp);

	return res;
}
