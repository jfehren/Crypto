/*
 * rsa-decrypt.c
 *
 *  Created on: Dec 15, 2019, 5:18:17 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"
#include "pkcs1.h"

int
rsa_decrypt(const struct rsa_private_key *key,
		uint8_t *dst, size_t *length, mpz_srcptr src) {
	mpz_t m;
	int res;

	mpz_init(m);
	rsa_compute_root(key, m, src);

	res = pkcs1_decrypt(dst, length, m, key->size);

	mpz_clear(m);
	return res;
}
