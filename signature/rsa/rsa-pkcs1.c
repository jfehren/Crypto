/*
 * rsa-pkcs1.c
 *
 *  Created on: Dec 15, 2019, 4:34:16 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"
#include "rsa-internal.h"
#include "pkcs1.h"

int
rsa_pkcs1_sign(const struct rsa_private_key *key, mpz_ptr s,
		const uint8_t *digest, size_t length) {
	_RSA_SIGN(digest_encode, key, s, digest, length);
}

int
rsa_pkcs1_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key, mpz_ptr s,
		const uint8_t *digest, size_t length,
		void *random_ctx, crypto_random_func *random) {
	_RSA_SIGN_TR(digest_encode, pub, key, s, random_ctx, random, digest, length);
}

int
rsa_pkcs1_verify(const struct rsa_public_key *key, mpz_srcptr s,
		const uint8_t *digest, size_t length) {
	_RSA_VERIFY(digest_encode, key, s, digest, length);
}
