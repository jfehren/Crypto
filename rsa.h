/*
 * rsa.h
 *
 *  Created on: Nov 1, 2019, 11:15:25 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef RSA_H_
#define RSA_H_

#include "crypto-types.h"
#include "crypto-gmp.h"

#include "md2.h"
#include "md4.h"
#include "md5.h"
#include "sha1.h"
#include "sha2.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling - Functions with _tr suffix are Timing-Resistant */
#define rsa_public_key_init crypto_rsa_public_key_init
#define rsa_public_key_clear crypto_rsa_public_key_clear
#define rsa_public_key_prepare crypto_rsa_public_key_prepare
#define rsa_private_key_init crypto_rsa_private_key_init
#define rsa_private_key_clear crypto_rsa_private_key_clear
#define rsa_private_key_prepare crypto_rsa_private_key_prepare
#define rsa_pkcs1_verify	crypto_rsa_pkcs1_verify
#define rsa_pkcs1_sign		crypto_rsa_pkcs1_sign
#define rsa_pkcs1_sign_tr	crypto_rsa_pkcs1_sign_tr
#define rsa_md2_sign		crypto_rsa_md2_sign
#define rsa_md2_sign_tr		crypto_rsa_md2_sign_tr
#define rsa_md2_verify		crypto_rsa_md2_verify
#define rsa_md4_sign		crypto_rsa_md4_sign
#define rsa_md4_sign_tr		crypto_rsa_md4_sign_tr
#define rsa_md4_verify		crypto_rsa_md4_verify
#define rsa_md5_sign		crypto_rsa_md5_sign
#define rsa_md5_sign_tr		crypto_rsa_md5_sign_tr
#define rsa_md5_verify		crypto_rsa_md5_verify
#define rsa_sha1_sign		crypto_rsa_sha1_sign
#define rsa_sha1_sign_tr	crypto_rsa_sha1_sign_tr
#define rsa_sha1_verify		crypto_rsa_sha1_verify
#define rsa_sha224_sign		crypto_rsa_sha224_sign
#define rsa_sha224_sign_tr	crypto_rsa_sha224_sign_tr
#define rsa_sha224_verify	crypto_rsa_sha224_verify
#define rsa_sha256_sign		crypto_rsa_sha256_sign
#define rsa_sha256_sign_tr	crypto_rsa_sha256_sign_tr
#define rsa_sha256_verify	crypto_rsa_sha256_verify
#define rsa_sha384_sign		crypto_rsa_sha384_sign
#define rsa_sha384_sign_tr	crypto_rsa_sha384_sign_tr
#define rsa_sha384_verify	crypto_rsa_sha384_verify
#define rsa_sha512_sign		crypto_rsa_sha512_sign
#define rsa_sha512_sign_tr	crypto_rsa_sha512_sign_tr
#define rsa_sha512_verify	crypto_rsa_sha512_verify
#define rsa_sha512_224_sign		crypto_rsa_sha224_sign
#define rsa_sha512_224_sign_tr	crypto_rsa_sha224_sign_tr
#define rsa_sha512_224_verify	crypto_rsa_sha224_verify
#define rsa_sha512_256_sign		crypto_rsa_sha256_sign
#define rsa_sha512_256_sign_tr	crypto_rsa_sha256_sign_tr
#define rsa_sha512_256_verify	crypto_rsa_sha256_verify
#define rsa_md2_sign_digest		crypto_rsa_md2_sign_digest
#define rsa_md2_sign_digest_tr		crypto_rsa_md2_sign_digest_tr
#define rsa_md2_verify_digest		crypto_rsa_md2_verify_digest
#define rsa_md4_sign_digest			crypto_rsa_md4_sign_digest
#define rsa_md4_sign_digest_tr		crypto_rsa_md4_sign_digest_tr
#define rsa_md4_verify_digest		crypto_rsa_md4_verify_digest
#define rsa_md5_sign_digest			crypto_rsa_md5_sign_digest
#define rsa_md5_sign_digest_tr		crypto_rsa_md5_sign_digest_tr
#define rsa_md5_verify_digest		crypto_rsa_md5_verify_digest
#define rsa_sha1_sign_digest		crypto_rsa_sha1_sign_digest
#define rsa_sha1_sign_digest_tr		crypto_rsa_sha1_sign_digest_tr
#define rsa_sha1_verify_digest		crypto_rsa_sha1_verify_digest
#define rsa_sha224_sign_digest		crypto_rsa_sha224_sign_digest
#define rsa_sha224_sign_digest_tr	crypto_rsa_sha224_sign_digest_tr
#define rsa_sha224_verify_digest	crypto_rsa_sha224_verify_digest
#define rsa_sha256_sign_digest		crypto_rsa_sha256_sign_digest
#define rsa_sha256_sign_digest_tr	crypto_rsa_sha256_sign_digest_tr
#define rsa_sha256_verify_digest	crypto_rsa_sha256_verify_digest
#define rsa_sha384_sign_digest		crypto_rsa_sha384_sign_digest
#define rsa_sha384_sign_digest_tr	crypto_rsa_sha384_sign_digest_tr
#define rsa_sha384_verify_digest	crypto_rsa_sha384_verify_digest
#define rsa_sha512_sign_digest		crypto_rsa_sha512_sign_digest
#define rsa_sha512_sign_digest_tr	crypto_rsa_sha512_sign_digest_tr
#define rsa_sha512_verify_digest	crypto_rsa_sha512_verify_digest
#define rsa_sha512_224_sign_digest		crypto_rsa_sha224_sign_digest
#define rsa_sha512_224_sign_digest_tr	crypto_rsa_sha224_sign_digest_tr
#define rsa_sha512_224_verify_digest	crypto_rsa_sha224_verify_digest
#define rsa_sha512_256_sign_digest		crypto_rsa_sha256_sign_digest
#define rsa_sha512_256_sign_digest_tr	crypto_rsa_sha256_sign_digest_tr
#define rsa_sha512_256_verify_digest	crypto_rsa_sha256_verify_digest
#define rsa_encrypt crypto_rsa_encrypt
#define rsa_decrypt crypto_rsa_decrypt
#define rsa_decrypt_tr crypto_rsa_decrypt_tr
#define rsa_compute_root crypto_rsa_compute_root
#define rsa_compute_root_tr crypto_rsa_compute_root_tr
#define rsa_generate_keypair crypto_rsa_generate_keypair
#define rsa_keypair_to_sexp crypto_rsa_keypair_to_sexp
#define rsa_keypair_from_sexp_alist crypto_rsa_keypair_from_sexp_alist
#define rsa_keypair_from_sexp crypto_rsa_keypair_from_sexp
#define rsa_public_key_from_der_iterator crypto_rsa_public_key_from_der_iterator
#define rsa_private_key_from_der_iterator crypto_rsa_private_key_from_der_iterator
#define rsa_keypair_from_der crypto_rsa_keypair_from_der

/* The smallest size of N that makes sense with PKCS #1, and allows
 * RSA encryption of one byte messages, is 12 octets and 89 bits */

#define RSA_MIN_N_OCTETS	12
#define RSA_MIN_N_BITS		(8*RSA_MIN_N_OCTETS - 7)

struct rsa_public_key {
	/* Size of the modulus, in octets. This is also the size of all
	 * signatures that are created or verified by this key. */
	size_t size;
	/* Modulus */
	mpz_t n;
	/* Public exponent */
	mpz_t e;
};

struct rsa_private_key {
	size_t size;
	/* d is filled in by the key generation function; otherwise it's
	 * completely unused. */
	mpz_t d;
	/* The two factors of the modulus */
	mpz_t p, q;
	/* d mod (p-1), i.e. a*e = 1 (mod (p-1)) */
	mpz_t a;
	/* d mod (q-1), i.e. b*e = 1 (mod (q-1)) */
	mpz_t b;
	/* modular inverse of q, i.e. c*q = 1 (mod p) */
	mpz_t c;
};

/* Signing a message (with md5 hash) works as follows:
 *
 * Store the private key in a rsa_private_key struct.
 *
 * Call rsa_private_key_prepare. This initializes the size attribute
 * to the length of a signature.
 *
 * Initialize a hashing context, by calling
 *   md5_init
 *
 * Hash the message by calling
 *   md5_update
 *
 * Create the signature by calling
 *   rsa_md5_sign
 *
 * The signature is represented as a mpz_t bignum. This call also
 * resets the hashing context.
 *
 * When done with the key and signature, don't forget to call
 * mpz_clear.
 */

/* Calls mpz_init to initialize storage */
__CRYPTO_DECLSPEC void
rsa_public_key_init(struct rsa_public_key *key);

/* Calls mpz_clear to deallocate storage */
__CRYPTO_DECLSPEC void
rsa_public_key_clear(struct rsa_public_key *key);

/* Check the the validity of the public key */
__CRYPTO_DECLSPEC int
rsa_public_key_prepare(struct rsa_public_key *key);

/* Calls mpz_init to initialize storage */
__CRYPTO_DECLSPEC void
rsa_private_key_init(struct rsa_private_key *key);

/* Calls mpz_clear to deallocate storage */
__CRYPTO_DECLSPEC void
rsa_private_key_clear(struct rsa_private_key *key);

/* Check the the validity of the private key */
__CRYPTO_DECLSPEC int
rsa_private_key_prepare(struct rsa_private_key *key);

/* PKCS#1 style signatures */
__CRYPTO_DECLSPEC int
rsa_pkcs1_sign(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest, size_t length);

__CRYPTO_DECLSPEC int
rsa_pkcs1_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest, size_t length,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_pkcs1_verify(const struct rsa_public_key *key, mpz_srcptr signature,
		const uint8_t *digest, size_t length);

/* MD2 signatures */
__CRYPTO_DECLSPEC int
rsa_md2_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct md2_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_md2_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct md2_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md2_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct md2_ctx *hash);

/* MD4 signatures */
__CRYPTO_DECLSPEC int
rsa_md4_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct md4_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_md4_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct md4_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md4_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct md4_ctx *hash);

/* MD5 signatures */
__CRYPTO_DECLSPEC int
rsa_md5_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct md5_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_md5_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct md5_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md5_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct md5_ctx *hash);

/* SHA1 signatures */
__CRYPTO_DECLSPEC int
rsa_sha1_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha1_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha1_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha1_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha1_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha1_ctx *hash);

/* SHA224 signatures */
__CRYPTO_DECLSPEC int
rsa_sha224_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha224_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha224_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha224_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha224_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha224_ctx *hash);

/* SHA256 signatures */
__CRYPTO_DECLSPEC int
rsa_sha256_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha256_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha256_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha256_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha256_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha256_ctx *hash);

/* SHA384 signatures */
__CRYPTO_DECLSPEC int
rsa_sha384_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha384_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha384_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha384_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha384_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha384_ctx *hash);

/* SHA512 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha512_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha512_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha512_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha512_ctx *hash);

/* SHA512/224 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_224_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha512_224_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha512_224_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha512_224_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_224_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha512_224_ctx *hash);

/* SHA512/256 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_256_sign(const struct rsa_private_key *key, mpz_ptr signature,
		struct sha512_256_ctx *hash);

__CRYPTO_DECLSPEC int
rsa_sha512_256_sign_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, struct sha512_256_ctx *hash,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_256_verify(const struct rsa_private_key *key, mpz_srcptr signature,
		struct sha512_256_ctx *hash);

/* Variant taking the digest as the argument */

/* MD2 signatures */
__CRYPTO_DECLSPEC int
rsa_md2_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_md2_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md2_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* MD4 signatures */
__CRYPTO_DECLSPEC int
rsa_md4_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_md4_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md4_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* MD5 signatures */
__CRYPTO_DECLSPEC int
rsa_md5_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_md5_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_md5_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA1 signatures */
__CRYPTO_DECLSPEC int
rsa_sha1_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha1_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha1_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA224 signatures */
__CRYPTO_DECLSPEC int
rsa_sha224_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha224_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha224_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA256 signatures */
__CRYPTO_DECLSPEC int
rsa_sha256_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha256_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha256_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA384 signatures */
__CRYPTO_DECLSPEC int
rsa_sha384_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha384_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha384_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA512 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha512_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA512/224 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_224_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha512_224_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_224_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* SHA512/256 signatures */
__CRYPTO_DECLSPEC int
rsa_sha512_256_sign_digest(const struct rsa_private_key *key, mpz_ptr signature,
		const uint8_t *digest);

__CRYPTO_DECLSPEC int
rsa_sha512_256_sign_digest_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr signature, const uint8_t *digest,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
rsa_sha512_256_verify_digest(const struct rsa_private_key *key, mpz_srcptr signature,
		const uint8_t *digest);

/* RSA encryption, using PKCS#1
 * These functions use the v1.5 padding */

/* Returns 1 on success, 0 on failure, which happens if the
 * message is too long for the key. */
__CRYPTO_DECLSPEC int
rsa_encrypt(const struct rsa_public_key *key,
		mpz_ptr dst, const uint8_t *src, size_t length,
		/* For padding */
		void *random_ctx, crypto_random_func *random);

/* dst must point to a buffer of size *length. key->size is enough
 * for all valid messages. On success, *length is updated to reflect
 * the actual length of the message. Returns 1 on success, 0 on
 * failure, which happens if decryption failed or if the message
 * didn't fit. */
__CRYPTO_DECLSPEC int
rsa_decrypt(const struct rsa_private_key *key,
		uint8_t *dst, size_t *length, mpz_srcptr src);

/* Timing-Resistant decryption, using randomized RSA blinding */
__CRYPTO_DECLSPEC int
rsa_decrypt_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		uint8_t *dst, size_t *length, mpz_srcptr src,
		void *random_ctx, crypto_random_func *random);

/* Compute x, the e-th root of m. Calling it with x == m is allowed. */
__CRYPTO_DECLSPEC void
rsa_compute_root(const struct rsa_private_key *key, mpz_ptr x, mpz_srcptr m);

/* Safer variant, using RSA blinding, and checking the result after CTR. */
__CRYPTO_DECLSPEC int
rsa_compute_root_tr(const struct rsa_public_key *pub,
		const struct rsa_private_key *key,
		mpz_ptr x, mpz_srcptr m,
		void *random_ctx, crypto_random_func *random);

/* Key generation - note that the key structs must be initialized first */
__CRYPTO_DECLSPEC int
rsa_generate_keypair(struct rsa_public_key *pub,
		struct rsa_private_key *key,
		/* Desired size of the modulus, in bits */
		unsigned n_size,
		/* Desired size of the public exponent, int bits. If
		 * zero, the passed value of pub->e is used. */
		unsigned e_size,
		void *random_ctx, crypto_random_func *random,
		void *progress_ctx, crypto_progress_func *progress);


#define RSA_SIGN(key, algorithm, ctx, data, length, signature) ( \
	algorithm##_update(ctx, data, length), \
	rsa_##algorithm##_sign(key, signature, ctx) \
)

#define RSA_SIGN_TR(pub, key, ctx, data, length, signature, random_ctx, random) ( \
	algorithm##_update(ctx, data, length), \
	rsa_##algorithm##_sign_tr(pub, key, signature, ctx, random_ctx, random) \
)

#define RSA_VERIFY(key, algorithm, ctx, data, length, signature) ( \
	algorithm##_update(ctx, data, length), \
	rsa_##algorithm##_verify(key, signature, ctx) \
)

/* TODO Write sexp and der functions */

/* Keys in sexp form */

struct crypto_buffer;
struct sexp_iterator;

/* Generates a public-key expression if PRIV is NULL */
__CRYPTO_DECLSPEC int
rsa_keypair_to_sexp(struct crypto_buffer *buffer,
		const char *algorithm_name,	/* NULL means "rsa" */
		const struct rsa_public_key *pub,
		const struct rsa_private_key *priv);

__CRYPTO_DECLSPEC int
rsa_keypair_from_sexp_alist(struct rsa_public_key *pub,
		struct rsa_private_key *priv,
		unsigned limit, struct sexp_iterator *i);

/* If PRIV is NULL, expect a public-key expression. If PUB is NULL,
 * expect a private-key expression and ignore the parts not needed
 * for the public key. */
/* Keys must be initialized before calling this function, as usual. */
__CRYPTO_DECLSPEC int
rsa_keypair_from_sexp(struct rsa_public_key *pub,
		struct rsa_private_key *priv,
		unsigned limit,
		const uint8_t *expt, size_t length);

/* Keys in PKCS#1 format */
struct asn1_der_iterator;

__CRYPTO_DECLSPEC int
rsa_public_key_from_der_iterator(struct rsa_public_key *pub,
		unsigned limit, struct asn1_der_iterator *i);

__CRYPTO_DECLSPEC int
rsa_private_key_from_der_iterator(struct rsa_public_key *pub,
		struct rsa_private_key *priv, unsigned limit,
		struct asn1_der_iterator *i);

/* For public keys, use PRIV == NULL */
__CRYPTO_DECLSPEC int
rsa_keypair_from_der(struct rsa_public_key *pub,
		struct rsa_private_key *priv, unsigned limit,
		const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* RSA_H_ */
