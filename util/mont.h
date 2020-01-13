/*
 * mont.h
 *
 *  Created on: Dec 19, 2019, 4:21:15 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef MONT_H_
#define MONT_H_

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct montgomery_ctx {
	mp_bitcnt_t rb;	/* log_2(R), a multiple of GMP_NUMB_BITS */
	mpz_t R2;		/* Value of R^2 (mod N) (for conversion to Montgomery Form) */
	mpz_t N;		/* The modulus N */
	mpz_t Ni;		/* N^{-1} (mod R) */
} mont_ctx;

void
montgomery_init(mont_ctx *ctx);

void
montgomery_clear(mont_ctx *ctx);

void
montgomery_set(mont_ctx *ctx, mpz_srcptr mod);

void
montgomery_copy(mont_ctx *dst, const mont_ctx *src);

void
montgomery_encode(mpz_ptr r, mpz_srcptr a, const mont_ctx *ctx);

void
montgomery_decode(mpz_ptr r, mpz_srcptr a, const mont_ctx *ctx);

void
montgomery_mul(mpz_ptr r, mpz_srcptr a, mpz_srcptr b, const mont_ctx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* MONT_H_ */
