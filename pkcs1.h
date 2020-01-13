/*
 * pkcs1.h
 *
 *  Created on: Nov 5, 2019, 9:16:33 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef PKCS1_H_
#define PKCS1_H_

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

/* Name mangling */
#define pkcs1_encrypt crypto_pkcs1_encrypt
#define pkcs1_decrypt crypto_pkcs1_decrypt
#define pkcs1_rsa_digest_encode crypto_pkcs1_rsa_digest_encode
#define pkcs1_rsa_md2_encode crypto_pkcs1_rsa_md2_encode
#define pkcs1_rsa_md4_encode crypto_pkcs1_rsa_md4_encode
#define pkcs1_rsa_md5_encode crypto_pkcs1_rsa_md5_encode
#define pkcs1_rsa_sha1_encode crypto_pkcs1_rsa_sha1_encode
#define pkcs1_rsa_sha224_encode crypto_pkcs1_rsa_sha224_encode
#define pkcs1_rsa_sha256_encode crypto_pkcs1_rsa_sha256_encode
#define pkcs1_rsa_sha384_encode crypto_pkcs1_rsa_sha384_encode
#define pkcs1_rsa_sha512_encode crypto_pkcs1_rsa_sha512_encode
#define pkcs1_rsa_sha512_224_encode crypto_pkcs1_rsa_sha512_224_encode
#define pkcs1_rsa_sha512_256_encode crypto_pkcs1_rsa_sha512_256_encode
#define pkcs1_rsa_md2_encode_digest crypto_pkcs1_rsa_md2_encode_digest
#define pkcs1_rsa_md4_encode_digest crypto_pkcs1_rsa_md4_encode_digest
#define pkcs1_rsa_md5_encode_digest crypto_pkcs1_rsa_md5_encode_digest
#define pkcs1_rsa_sha1_encode_digest crypto_pkcs1_rsa_sha1_encode_digest
#define pkcs1_rsa_sha224_encode_digest crypto_pkcs1_rsa_sha224_encode_digest
#define pkcs1_rsa_sha256_encode_digest crypto_pkcs1_rsa_sha256_encode_digest
#define pkcs1_rsa_sha384_encode_digest crypto_pkcs1_rsa_sha384_encode_digest
#define pkcs1_rsa_sha512_encode_digest crypto_pkcs1_rsa_sha512_encode_digest
#define pkcs1_rsa_sha512_224_encode_digest crypto_pkcs1_rsa_sha512_224_encode_digest
#define pkcs1_rsa_sha512_256_encode_digest crypto_pkcs1_rsa_sha512_256_encode_digest


__CRYPTO_DECLSPEC int
pkcs1_encrypt(mpz_t m, size_t key_size, const uint8_t *src, size_t length,
		void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC int
pkcs1_decrypt(uint8_t *dst, size_t *length, mpz_srcptr m, size_t key_size);

__CRYPTO_DECLSPEC int
pkcs1_rsa_digest_encode(mpz_ptr m, size_t key_size, const uint8_t *digest, size_t length);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md2_encode(mpz_ptr m, size_t key_size, struct md2_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md4_encode(mpz_ptr m, size_t key_size, struct md4_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md5_encode(mpz_ptr m, size_t key_size, struct md5_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha1_encode(mpz_ptr m, size_t key_size, struct sha1_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha224_encode(mpz_ptr m, size_t key_size, struct sha224_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha256_encode(mpz_ptr m, size_t key_size, struct sha256_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha384_encode(mpz_ptr m, size_t key_size, struct sha384_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_encode(mpz_ptr m, size_t key_size, struct sha512_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_224_encode(mpz_ptr m, size_t key_size, struct sha512_224_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_256_encode(mpz_ptr m, size_t key_size, struct sha512_256_ctx *hash);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md2_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md4_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_md5_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha1_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha224_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha256_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha384_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_224_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

__CRYPTO_DECLSPEC int
pkcs1_rsa_sha512_256_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest);

#ifdef __cplusplus
}
#endif

#endif /* PKCS1_H_ */
