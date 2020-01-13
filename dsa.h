/*
 * dsa.h
 *
 *  Created on: Oct 22, 2019, 12:26:06 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef DSA_H_
#define DSA_H_

#include "crypto.h"
#include "crypto-gmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define dsa_params_init		crypto_dsa_params_init
#define dsa_params_clear	crypto_dsa_params_clear

#define dsa_signature_init	crypto_dsa_signature_init
#define dsa_signature_clear	crypto_dsa_signature_clear

#define dsa_sign	crypto_dsa_sign
#define dsa_verify	crypto_dsa_verify

#define dsa_generate_params		crypto_dsa_generate_params
#define dsa_generate_keypair	crypto_dsa_generate_keypair

#define dsa_signature_from_sexp			crypto_dsa_signature_from_sexp
#define dsa_keypair_to_sexp				crypto_dsa_keypair_to_sexp
#define dsa_keypair_from_sexp_alist		crypto_dsa_keypair_from_sexp_alist
#define dsa_sha1_keypair_from_sexp		crypto_dsa_sha1_keypair_from_sexp
#define dsa_sha256_keypair_from_sexp	crypto_dsa_sha256_keypair_from_sexp

#define dsa_params_from_der_iterator				crypto_dsa_params_from_der_iterator
#define dsa_public_key_from_der_iterator			crypto_dsa_public_key_from_der_iterator
#define dsa_openssl_private_key_from_der_iterator	crypto_dsa_openssl_private_key_from_der_iterator
#define dsa_openssl_private_key_from_der			crypto_openssl_provate_key_from_der

/* For FIPS approved parameters */
#define DSA_SHA1_MIN_P_BITS	512
#define DSA_SHA1_Q_OCTETS	20
#define DSA_SHA1_Q_BITS		160

#define DSA_SHA256_MIN_P_BITS	1024
#define DSA_SHA256_Q_OCTETS		32
#define DSA_SHA256_Q_BITS		256

struct dsa_params {
	mpz_t p;	/* Modulus */
	mpz_t q;	/* Group order */
	mpz_t g;	/* Field generator */
};

__CRYPTO_DECLSPEC void
dsa_params_init(struct dsa_params *params);

__CRYPTO_DECLSPEC void
dsa_params_clear(struct dsa_params *params);

struct dsa_signature {
	mpz_t r;
	mpz_t s;
};

__CRYPTO_DECLSPEC void
dsa_signature_init(struct dsa_signature *signature);

__CRYPTO_DECLSPEC void
dsa_signature_clear(struct dsa_signature *signature);

__CRYPTO_DECLSPEC int
dsa_sign(const struct dsa_params *params, mpz_srcptr x, const uint8_t *digest,
		size_t digest_size, struct dsa_signature *signature,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
dsa_verify(const struct dsa_params *params, mpz_srcptr y, const uint8_t *digest,
		size_t digest_size, struct dsa_signature *signature);

/* Key generation */

__CRYPTO_DECLSPEC int
dsa_generate_params(struct dsa_params *params, unsigned p_bits, unsigned q_bits,
		void *random_ctx, crypto_random_func *random, void *progress_ctx,
		crypto_progress_func *progress);

__CRYPTO_DECLSPEC void
dsa_generate_keypair(const struct dsa_params *params, mpz_ptr pub, mpz_ptr key,
		void *random_ctx, crypto_random_func *random);

/* Keys in sexp form */

struct crypto_buffer;
struct sexp_iterator;

__CRYPTO_DECLSPEC int
dsa_keypair_to_sexp(struct crypto_buffer *buffer,
		const char *algorithm_name, /* NULL means "dsa" */
		const struct dsa_params *params,
		mpz_srcptr pub, /* Generates a public-key expression if PRIV is NULL .*/
		mpz_srcptr priv);

__CRYPTO_DECLSPEC int
dsa_signature_from_sexp(struct dsa_signature *rs,
		struct sexp_iterator *i, unsigned q_bits);

__CRYPTO_DECLSPEC int
dsa_keypair_from_sexp_alist(struct dsa_params *params, mpz_ptr pub, mpz_ptr priv,
		struct sexp_iterator *i, unsigned q_bits, unsigned p_max_bits);

/* If PRIV is NULL, expect a public-key expression. If PUB is NULL,
 * expect a private key expression and ignore the parts not needed for
 * the public key. */
/* Keys must be initialized before calling this function. */
__CRYPTO_DECLSPEC int
dsa_sha1_keypair_from_sexp(struct dsa_params *params, mpz_ptr pub, mpz_ptr priv,
		const uint8_t *expr, size_t length, unsigned p_max_bits);

__CRYPTO_DECLSPEC int
dsa_sha256_keypair_from_sexp(struct dsa_params *params, mpz_ptr pub, mpz_ptr priv,
		const uint8_t *expr, size_t length, unsigned p_max_bits);


/* Keys in X.509 and OpenSSL format. */
struct asn1_der_iterator;

__CRYPTO_DECLSPEC int
dsa_params_from_der_iterator(struct dsa_params *params,
		struct asn1_der_iterator *i, unsigned p_max_bits, unsigned q_bits);

__CRYPTO_DECLSPEC int
dsa_public_key_from_der_iterator(const struct dsa_params *params,
		mpz_ptr pub, struct asn1_der_iterator *i);

__CRYPTO_DECLSPEC int
dsa_openssl_private_key_from_der_iterator(struct dsa_params *params,
		struct asn1_der_iterator *i, mpz_ptr pub, mpz_ptr priv,
		unsigned p_max_bits);

__CRYPTO_DECLSPEC int
dsa_openssl_private_key_from_der(struct dsa_params *params,
		mpz_ptr pub, mpz_ptr priv, const uint8_t *data, size_t length,
		unsigned p_max_bits);


#ifdef __cplusplus
}
#endif

#endif /* DSA_H_ */
