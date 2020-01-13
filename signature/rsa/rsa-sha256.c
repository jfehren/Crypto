/*
 * rsa-sha256.c
 *
 *  Created on: Dec 15, 2019, 5:12:14 PM
 *      Author: Joshua Fehrenbach
 */

#include "rsa.h"
#include "rsa-internal.h"
#include "pkcs1.h"

int
rsa_sha256_sign(const struct rsa_private_key *key, mpz_ptr s, struct sha256_ctx *hash) {
	_RSA_SIGN(sha256_encode, key, s, hash);
}

int
rsa_sha256_sign_digest(const struct rsa_private_key *key, mpz_ptr s, const uint8_t *digest) {
	_RSA_SIGN(sha256_encode_digest, key, s, digest);
}

int
rsa_sha256_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key, mpz_ptr s,
		struct sha256_ctx *hash, void *random_ctx,
		crypto_random_func *random) {
	_RSA_SIGN_TR(sha256_encode, pub, key, s, random_ctx, random, hash);
}

int
rsa_sha256_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key, mpz_ptr s,
		const uint8_t *digest, void *random_ctx,
		crypto_random_func *random) {
	_RSA_SIGN_TR(sha256_encode_digest, pub, key, s, random_ctx, random, digest);
}

int
rsa_sha256_verify(const struct rsa_public_key *key, mpz_srcptr s, struct sha256_ctx *hash) {
	_RSA_VERIFY(sha256_encode, key, s, hash);
}

int
rsa_sha256_verify_digest(const struct rsa_private_key *key, mpz_srcptr s, const uint8_t *digest) {
	_RSA_VERIFY(sha256_encode_digest, key, s, digest);
}
