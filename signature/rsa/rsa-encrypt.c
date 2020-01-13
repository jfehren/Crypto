/*
 * rsa-encrypt.c
 *
 *  Created on: Dec 15, 2019, 5:15:35 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"
#include "pkcs1.h"

int
rsa_encrypt(const struct rsa_public_key *key,
		mpz_ptr dst, const uint8_t *src, size_t length,
		/* For padding */
		void *random_ctx, crypto_random_func *random) {
	if (pkcs1_encrypt(dst, key->size, src, length, random_ctx, random)) {
		mpz_powm(dst, dst, key->e, key->n);
		return 1;
	} else {
		return 0;
	}
}
