/*
 * rsa.c
 *
 *  Created on: Nov 4, 2019, 12:05:22 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa-internal.h"

void
rsa_public_key_init(struct rsa_public_key *key) {
	key->size = 0;
	mpz_init(key->n);
	mpz_init(key->e);
}

void
rsa_public_key_clear(struct rsa_public_key *key) {
	mpz_clear(key->e);
	mpz_clear(key->n);
}

void
rsa_private_key_init(struct rsa_private_key *key) {
	key->size = 0;
	mpz_init(key->d);
	mpz_init(key->p);
	mpz_init(key->q);
	mpz_init(key->a);
	mpz_init(key->b);
	mpz_init(key->c);
}

void
rsa_private_key_clear(struct rsa_private_key *key) {
	mpz_clear(key->c);
	mpz_clear(key->b);
	mpz_clear(key->a);
	mpz_clear(key->q);
	mpz_clear(key->p);
	mpz_clear(key->d);
}

size_t __attribute__((pure))
_rsa_check_size(mpz_ptr n) {
	size_t size;
	/* Even moduli are invalid, and not supported by mpz_powm_sec anyways */
	if (mpz_even_p(n)) {
		return 0;
	}
	size = (mpz_sizeinbase(n, 2) + 7) / 8;
	return (size >= RSA_MIN_N_OCTETS ? size : 0);
}

int
rsa_public_key_prepare(struct rsa_public_key *key) {
	key->size = _rsa_check_size(key->n);
	return (key->size > 0);
}

int
rsa_private_key_prepare(struct rsa_private_key *key) {
	mpz_t n;

	/* The size of the product is the sum of the sizes of the factors,
	 * or sometimes one less. It's possible but tricky to compute the
	 * size without computing the full product. */

	mpz_init(n);
	mpz_mul(n, key->p, key->q);

	key->size = _rsa_check_size(n);

	mpz_clear(n);

	return (key->size > 0);
}

