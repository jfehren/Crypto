/*
 * mont.c
 *
 *  Created on: Dec 19, 2019, 11:42:52 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include "mont.h"

void
montgomery_init(mont_ctx *ctx) {
	ctx->rb = 0;
	mpz_init(ctx->R2);
	mpz_init(ctx->N);
	mpz_init(ctx->Ni);
}

void
montgogery_clear(mont_ctx *ctx) {
	mpz_clear(ctx->Ni);
	mpz_clear(ctx->N);
	mpz_clear(ctx->R2);
}


/*
 * Implement efficient calculation of multiplicative inverses
 * modulo powers of 2 according to the algorithms described in
 * arxiv.org/pdf/1209.6626.pdf "On Newton-Raphson iteration for
 * multiplicative inverses modulo prime powers" by Jean-Guillaume
 * Dumas. Heavy use of the GMP function mpz_fdiv_r_2exp is done
 * in these functions, especially before multiplications, since,
 * when performed in-place, this function operates in worst-case
 * linear time, best-case constant time, and average-case nearly
 * constant (but still technically linear) time. mpn functions
 * are used by inverse_pow2_explicit to eliminate redundant
 * overhead in mpz function calls and unnecessary temporary
 * memory allocation.
 *
 * Unfortunately, mpn_mullo_n and mpn_sqrlo are internal GMP
 * functions, or they could be used to greatly improve the efficiency
 * of all multiplications by skipping the calculation of the upper
 * half of the product.
 */

#if !defined(__GNUC__) || __GNUC__ < 2
#define __builtin_constant_p(c) 1
#endif

#define ABOVE_THRESHOLD(size,thresh) \
	((__builtin_constant_p (thresh) && (thresh) == 0) \
		|| (!(__builtin_constant_p (thresh) && (thresh) == MP_SIZE_T_MAX) \
				&& (size) >= (thresh)))
#define BELOW_THRESHOLD(size,thresh) !ABOVE_THRESHOLD(size,thresh)

/* define these macros for simplicity */

#ifdef mpz_mod_2exp
#undef mpz_mod_2exp
#endif
#define mpz_mod_2exp(r,a,m) mpz_fdiv_r_2exp(r,a,m)

#ifdef mpz_div_2exp
#undef mpz_div_2exp
#endif
#define mpz_div_2exp(r,a,m) mpz_fdiv_q_2exp(r,a,m)

#if __GMP_MP_SIZE_T_INT
#define MP_SIZE_T_MAX INT_MAX
#define MP_SIZE_T_MIN INT_MIN
#else
#define MP_SIZE_T_MAX LONG_MAX
#define MP_SIZE_T_MIN LONG_MIN
#endif

#if 1

/* Tuning functions here based on GMP's tuning functions */

#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

static void inverse_pow2_explicit(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m);
static void inverse_pow2_arazi_qi(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m);
static void inverse_pow2_hensel(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m);
static void inverse_pow2(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m);

static mp_bitcnt_t explicit_to_hensel_threshold;
static mp_bitcnt_t hensel_to_arazi_threshold;
static mp_bitcnt_t arazi_to_hensel_threshold;

gmp_randstate_t rand_st;

#define EXPLICIT_TO_HENSEL_THRESHOLD explicit_to_hensel_threshold
#define HENSEL_TO_ARAZI_THRESHOLD hensel_to_arazi_threshold
#define ARAZI_TO_HENSEL_THRESHOLD arazi_to_hensel_threshold

void
gmp_random(void *ctx, mpz_ptr dst, mp_size_t size) {
	mpz_urandomb(dst, (gmp_randstate_t)ctx, size * GMP_NUMB_BITS);
}


double
speed_inverse_pow2(struct speed_params *s) {

}

int main(void) {
	gmp_randinit_mt(rand_st);
}

#else

#define EXPLICIT_TO_HENSEL_THRESHOLD	1700
#define HENSEL_TO_ARAZI_THRESHOLD		14000
#define ARAZI_TO_HENSEL_THRESHOLD		600000

#endif

static void
inverse_pow2_explicit(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m) {
	assert (mpz_odd_p(a));				/* ensure a is odd; otherwise a has no inverse */
#if 0
	mp_bitcnt_t s, i;
	mpz_t amone, t;
	mp_limb_t mask;
	mp_ptr ap, tp, rp, ttp;
	mp_size_t n, tn;
	mpz_init(amone);
	mpz_init(t);

	/* amone = a - 1 */
	mpz_sub_ui(amone, a, 1);
	/* find s such that, for some odd integer t, a = 1 + t*2^s */
	s = mpz_scan1(amone, 0);
	/* r = 2 - a
	 *   = 1 - a + 1
	 *   = 1 - (a - 1)
	 *   = 1 - amone
	 *   = -(amone - 1) */
	mpz_sub_ui(t, amone, 1);
	mpz_neg(t, t);
	mpz_mod_2exp(r, t, m);

	/* use mpn for loop calculations */
	n = (m + GMP_NUMB_BITS - 1) / GMP_NUMB_BITS;
	/* get mpn-style arrays, ensuring they are the correct size */
	ap = mpz_limbs_modify(amone, 2*n);	/* double size, since this must store a product */
	tp = mpz_limbs_modify(t, 2*n);		/* double size, since this must store a product */
	rp = mpz_limbs_modify(r, n);
	/* zero-extend mpn values to n limbs */
	tn = mpz_size(amone);
	if (tn < n) {
		mpn_zero(ap + tn, n - tn);
	}
	tn = mpz_size(t);
	if (tn < n) {
		mpn_zero(tp + tn, n - tn);
	}
	tn = mpz_size(r);
	if (tn < n) {
		mpn_zero(rp + tn, n - tn);
	}
	/* compute mask for the high limb */
	/* for (m % GMP_NUMB_BITS == 0), this gives the identity mask */
	mask = (((mp_limb_t)1) << (m % GMP_NUMB_BITS) - 1);
	for (i = (s << 1); i < m; i <<= 1) {
		/* ignore upper half of mpn_sqr and mpn_mul_n results */
		mpn_sqr(tp, ap, n);			/* (a - 1)^{2^i} (mod 2^m) */
		tp[n-1] &= mask;	/* mask high word */
		mpn_mul_n(ap, rp, tp, n);	/* r * (a - 1)^{2^i} (mod 2^m) */
		ap[n-1] &= mask;	/* mask high word */

		mpn_add_n(rp, rp, ap, n);	/* r += r * (a - 1)^{2^i} (mod 2^m) */
		rp[n-1] &= mask;	/* mask high word */

		/* swap ap and tp */
		ttp = tp; tp = ap; ap = ttp;
	}

	/* discard t and amone */
	mpz_clear(t);
	mpz_clear(amone);

	/* normalize r */
	mpz_limbs_finish(r, n);
	mpz_mod_2exp(r, r, m);	/* sanity */
#else
	mpz_t t1, t2;
	mpz_ptr t1p, t2p, ttp;
	mp_bitcnt_t s, i;
	mpz_init(t1);
	mpz_init(t2);

	/* t1 = (a - 1) mod 2^m */
	mpz_sub_ui(t1, a, 1);
	/* find s such that, for some odd integer t, a = 1 + t*2^s */
	s = mpz_scan1(t1, 0);
	/* r = 2 - a */
	mpz_neg(r, a);
	mpz_add_ui(r, r, 2);
	mpz_mod_2exp(r, r, m);

	t1p = t1; t2p = t2;

	for (i = (s << 1); i < m; i <<= 1) {
		mpz_mul(t2p, t1p, t1p);		/* (a-1)^{2^i} */
		mpz_mod_2exp(t2p, t2p, m);	/* (a-1)^{2^i} mod 2^m */
		mpz_mul(t1p, t2p, r);		/* r*[(a-1)^{2^i} mod 2^m] */
		mpz_add(r, r, t1p);			/* r + r*[(a-1)^{2^i} mod 2^m] */
		mpz_mod_2exp(r, r, m);		/* r*[1 + (a-1)^{2^i}] mod 2^m */

		ttp = t1p; t1p = t2p; t2p = ttp;
	}

	mpz_clear(t2);
	mpz_clear(t1);
#endif
}

static void
inverse_pow2_arazi_qi(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m) {
	mpz_t t, c, b;
	mp_bitcnt_t h = (m + 1) / 2;		/* h = ceil(m/2) */
	mpz_init(b);
	mpz_mod_2exp(b, a, h);			/* b = a mod 2^h */
	if (h > 1) {
		mpz_init(t);
		inverse_pow2_arazi_qi(t, b, h);	/* t = a^{-1} mod 2^h */
	} else {
		mpz_init_set_ui(t, 1);
	}
	mpz_init(c);

	mpz_mul(c, t, b);				/* t*b */
	mpz_div_2exp(c, c, h);			/* c = t*b / 2^h */

	mpz_div_2exp(b, a, h);			/* a / 2^h */
	mpz_mod_2exp(b, b, h);			/* (a / 2^h) mod 2^h -- reduces multiplication size */
	mpz_mul(b, b, t);				/* t*[(a / 2^h) mod 2^h] */
	mpz_mod_2exp(b, b, h);			/* b = t*(a / 2^h) mod 2^h */

	mpz_add(c, c, b);				/* b + c */

	mpz_mul(b, c, t);				/* t*(b + c) */
	mpz_neg(b, b);					/* -t*(b + c) */
	mpz_mod_2exp(b, b, h);			/* -t*(b + c) mod 2^h */

	mpz_mul_2exp(b, b, h);			/* [-t*(b + c) mod 2^h] * 2^h */
	mpz_ior(t, t, b);				/* t + [-t*(b + c) mod 2^h] * 2^h */

	mpz_mod_2exp(r, t, m);			/* r = {t + [-t*(b + c) mod 2^h] * 2^h} mod 2^m */

	mpz_clear(c);
	mpz_clear(t);
	mpz_clear(b);
}

static void
inverse_pow2_hensel(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m) {
	mpz_t b, t;
	mp_bitcnt_t h = (m + 1) / 2;	/* h = ceil(m/2) */
	mpz_init(b);
	mpz_init(t);
	mpz_mod_2exp(t, a, h);			/* a mod 2^h */
	if (h > 1) {
		inverse_pow2_hensel(b, t, h);	/* b = a^{-1} mod 2^h */
	} else {
		mpz_set_ui(b, 1);
	}

	mpz_mul(t, b, b);				/* b^2 */
	mpz_mod_2exp(t, t, m);			/* b^2 mod 2^m */
	mpz_mul(t, t, a);				/* a*(b^2 mod 2^m) */
	mpz_mod_2exp(t, t, m);			/* a*b^2 mod 2^m */

	mpz_mul_2exp(b, b, 1);			/* 2b */
	mpz_sub(b, b, t);				/* 2b - (a*b^2 mod 2^m) */

	mpz_mod_2exp(r, b, m);			/* r = b(2 - a*b) mod 2^m = a^{-1} mod 2^m */

	mpz_clear(b);
	mpz_clear(t);
}

static void
inverse_pow2(mpz_ptr r, mpz_srcptr a, mp_bitcnt_t m) {
	if (BELOW_THRESHOLD(m, EXPLICIT_TO_HENSEL_THRESHOLD)) {
		inverse_pow2_explicit(r, a, m);
		return;
	}
	mp_bitcnt_t h = (m + 1) / 2;	/* h = ceil(m/2) */
	mpz_t t1, t2, t3;
	mpz_init(t1);
	mpz_init(t2);
	mpz_init(t3);

	mpz_mod_2exp(t3, a, h);
	inverse_pow2(t1, t3, h);

	if (BELOW_THRESHOLD(m, HENSEL_TO_ARAZI_THRESHOLD) ||
			ABOVE_THRESHOLD(m, ARAZI_TO_HENSEL_THRESHOLD)) {
		/* Hensel */
		mpz_mul(t3, t1, t1);
		mpz_mod_2exp(t3, t3, m);
		mpz_mul(t2, t3, a);
		mpz_mod_2exp(t2, t2, m);

		mpz_mul_2exp(t1, t1, 1);
		mpz_sub(t1, t1, t2);

		mpz_mod_2exp(r, t1, m);
	} else {
		/* Arazi-Qi */
		mpz_mul(t2, t1, t3);
		mpz_div_2exp(t2, t2, h);

		mpz_div_2exp(t3, a, h);
		mpz_mod_2exp(t3, t3, h);
		mpz_mul(t3, t3, t1);
		mpz_mod_2exp(t3, t3, h);

		mpz_add(t2, t2, t3);

		mpz_mul(t3, t2, t1);
		mpz_neg(t3, t3);
		mpz_mod_2exp(t3, t3, h);

		mpz_mul_2exp(t3, t3, h);
		mpz_ior(t1, t1, t3);

		mpz_mod_2exp(r, t1, m);
	}

	mpz_clear(t3);
	mpz_clear(t2);
	mpz_clear(t1);
}

void
montgomery_set(mont_ctx *ctx, mpz_srcptr mod) {
	assert (ctx != NULL && mod != NULL);	/* ensure the input is non-null */
	assert (mpz_cmp_ui(mod, 3) >= 0);	/* ensure the modulus is >= 3 */
	assert (mpz_odd_p(mod));	/* ensure the modulus is odd. ensures gcd(R,N) = 1 */
	/* copy N */
	mpz_set(ctx->N, mod);
	/* set R to 2^{GMP_NUMB_BITS*ceil(log_2(N) / GMP_NUMB_BITS)}
	 * This ensures that R > N, gcd(R,N) = 1, and that
	 * reduction/division by R requires only omitting/shifting words  */
	ctx->rb = 1 + (mpz_sizeinbase(mod, 2) - 1) / GMP_NUMB_BITS;
	ctx->rb *= GMP_NUMB_BITS;
	/* compute Ni such that N*Ni = 1 (mod R) */
	inverse_pow2(ctx->Ni, ctx->N, ctx->rb);
	/* compute R^2 (mod N) */
	mpz_setbit(ctx->R2, 2*ctx->rb);
	mpz_fdiv_r(ctx->R2, ctx->R2, ctx->N);
}

void
montgomery_copy(mont_ctx *dst, const mont_ctx *src) {
	dst->rb = src->rb;
	mpz_set(dst->R2, src->R2);
	mpz_set(dst->N, src->N);
	mpz_set(dst->Ni, src->Ni);
}

static void
redc(mpz_ptr x, mpz_srcptr a, const mont_ctx *ctx) {
	mpz_t t;
	mpz_init(t);

	/* All divisions and modular reductions here are extremely cheap,
	 * as R is not only a power of 2, but is also a power of 2^GMP_NUMB_BITS,
	 * meaning that division by R requires only shifting limbs and
	 * reduction modulo R requires only omitting the high words */

	/* t := ((a mod R)*Ni) mod R */
	mpz_tdiv_r_2exp(t, a, ctx->rb);	/* a mod R */
	mpz_mul(t, t, ctx->Ni);			/* (a mod R)*Ni */
	mpz_tdiv_r_2exp(t, t, ctx->rb);	/* ((a mod R)*Ni) mod R */

	/* t := (a + t*N) / R */
	mpz_mul(t, t, ctx->N);			/* t*N */
	mpz_add(t, t, a);				/* a + t*N */
	mpz_tdiv_q_2exp(t, t, ctx->rb);	/* (a + t*N) / R */

	if (mpz_cmp(t, ctx->N) >= 0) {
		mpz_sub(x, t, ctx->N);
	} else {
		mpz_set(x, t);
	}
}

void
montgomery_encode(mpz_ptr r, mpz_srcptr a, const mont_ctx *ctx) {
	/* (a)(R^2) (mod N) = (aR)(1R) (mod N)
	 * Perform Montgomery Reduction on aR^2 (mod N) to get
	 * aR (mod N), the Montgomery Form of a */
	/*
	montgomery_mul(r, a, ctx->R2, ctx);
	*/
}

