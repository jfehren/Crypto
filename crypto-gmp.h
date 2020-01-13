/*
 * crypto-gmp.h
 *
 *  Created on: Oct 24, 2019, 12:31:56 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_GMP_H_
#define CRYPTO_GMP_H_

#include <gmp.h>

#include "crypto.h"
#include "crypto-types.h"
#include "crypto-meta.h"

#ifdef __cplusplus
extern "C" {
#endif

__CRYPTO_DECLSPEC size_t
crypto_mpz_sizeinbase_256_s(mpz_srcptr x);

__CRYPTO_DECLSPEC size_t
crypto_mpz_sizeinbase_256_u(mpz_srcptr x);

__CRYPTO_DECLSPEC void
crypto_mpz_get_str_256(uint8_t *s, size_t length, mpz_srcptr x);

__CRYPTO_DECLSPEC void
crypto_mpz_set_str_256_s(mpz_ptr x, const uint8_t *s, size_t length);

__CRYPTO_DECLSPEC void
crypto_mpz_init_set_str_256_s(mpz_ptr x, const uint8_t *s, size_t length);

__CRYPTO_DECLSPEC void
crypto_mpz_set_str_256_u(mpz_ptr x, const uint8_t *s, size_t length);

__CRYPTO_DECLSPEC void
crypto_mpz_init_set_str_256_u(mpz_ptr x, const uint8_t *s, size_t length);

__CRYPTO_DECLSPEC void
crypto_mpz_random_size(mpz_ptr x, unsigned bits, void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC void
crypto_mpz_random(mpz_ptr x, mpz_srcptr n, void *random_ctx, crypto_random_func *random);

__CRYPTO_DECLSPEC void
crypto_random_prime(mpz_ptr p, unsigned bits, int top_bits_set,
		void *random_ctx, crypto_random_func *random, void *progress_ctx,
		crypto_progress_func *progress);

__CRYPTO_DECLSPEC void
crypto_generate_pocklington_prime(mpz_ptr p, mpz_ptr r, unsigned bits, int top_bits_set,
		mpz_srcptr p0, mpz_srcptr q, mpz_srcptr p0q,
		void *random_ctx, crypto_random_func *random);

struct sexp_iterator;

__CRYPTO_DECLSPEC int
crypto_mpz_set_sexp(mpz_ptr x, unsigned limit, struct sexp_iterator *iterator);

struct asn1_der_iterator;

__CRYPTO_DECLSPEC int
crypto_asn1_der_get_bignum(struct asn1_der_iterator *iterator, mpz_ptr x, unsigned max_bits);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_GMP_H_ */
