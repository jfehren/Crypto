/*
 * dsa-verify.c
 *
 *  Created on: Oct 31, 2019, 12:58:33 PM
 *      Author: Joshua Fehrenbach
 */

#include "dsa.h"
#include "dsa-hash.h"

int
dsa_verify(const struct dsa_params *params, mpz_srcptr y, const uint8_t *digest,
		size_t digest_size, struct dsa_signature *sig) {
	mpz_t w, v, tmp;
	int res;

	/* Check that r and s are in the proper range */
	if (mpz_sgn(sig->r) <= 0 || mpz_cmp(sig->r, params->q) >= 0) {
		return 0;
	}
	if (mpz_sgn(sig->s) <= 0 || mpz_cmp(sig->s, params->q) >= 0) {
		return 0;
	}

	/* Compute w = s^-1 (mod q) */
	mpz_init(w);
	if (!mpz_invert(w, sig->s, params->q)) {
		mpz_clear(w);
		return 0;
	}
	mpz_init(v);
	mpz_init(tmp);

	/* Compute hash */
	_dsa_hash(tmp, mpz_sizeinbase(params->q, 2), digest, digest_size);

	/* v = g^{w*h (mod q)} (mod p) */
	mpz_mul(tmp, tmp, w);
	mpz_fdiv_r(tmp, tmp, params->q);
	mpz_powm(v, params->g, tmp, params->p);

	/* y^{w*r (mod q)} (mod p) */
	mpz_mul(tmp, sig->r, w);
	mpz_fdiv_r(tmp, tmp, params->q);
	mpz_powm(tmp, y, tmp, params->p);

	/* v = ( g^{w*h} * y^{w*r} (mod p) ) (mod q) */
	mpz_mul(v, v, tmp);
	mpz_fdiv_r(v, v, params->p);
	mpz_fdiv_r(v, v, params->q);

	res = !mpz_cmp(v, sig->r);

	mpz_clear(tmp);
	mpz_clear(v);
	mpz_clear(w);

	return res;
}

