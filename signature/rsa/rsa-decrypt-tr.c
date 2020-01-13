/*
 * rsa-decrypt-tr.c
 *
 *  Created on: Dec 15, 2019, 5:20:55 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"
#include "pkcs1.h"

int
rsa_decrypt_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		uint8_t *dst, size_t *length, mpz_srcptr src,
		void *random_ctx, crypto_random_func *random) {
	mpz_t m;
	int res;

	mpz_init_set(m, src);

	res = (rsa_compute_root_tr(pub, key, m, src, random_ctx, random)
			&& pkcs1_decrypt(dst, length, m, key->size));

	mpz_clear(m);
	return res;
}

