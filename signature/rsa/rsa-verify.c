/*
 * rsa-verify.c
 *
 *  Created on: Nov 5, 2019, 9:12:55 AM
 *      Author: Joshua Fehrenbach
 */

#include "rsa-internal.h"

int
_rsa_verify(const struct rsa_public_key *key, mpz_srcptr m, mpz_srcptr s) {
	int res;
	mpz_t m1;

	/* Check that 0 < s < n */
	if (mpz_sgn(s) <= 0 || mpz_cmp(s, key->n) >= 0) {
		return 0;
	}
	mpz_init(m1);

	mpz_powm(m1, s, key->e, key->n);

	res = !mpz_cmp(m, m1);

	mpz_clear(m1);

	return res;
}

