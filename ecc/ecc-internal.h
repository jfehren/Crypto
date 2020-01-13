/*
 * ecc-internal.h
 *
 *  Created on: Dec 16, 2019, 11:59:29 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef ECC_INTERNAL_H_
#define ECC_INTERNAL_H_

#include "ecc.h"

#ifdef __cplusplus
extern "C" {
#endif

struct ec_method_st {
	/* An NID to determine whether these methods are for
	 * prime curves over GF(p) or binary curves over GF(2^m) */
	int field_type;
	/* Used by ecc_group_alloc, ecc_group_free, and ecc_group_copy */
	int (*group_init)(ecc_group*);
	void (*group_clear)(ecc_group*);
	int (*group_copy)(ecc_group*, const ecc_group*);
	/* Used by ecc_group_set_curve, ecc_group_get_curve */
	int (*group_set_curve)(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
	int (*group_get_curve)(mpz_ptr, mpz_ptr, mpz_ptr, const ecc_group*);
	/* Used by ecc_group_get_degree */
	mp_bitcnt_t (*group_get_degree)(const ecc_group*);
	mp_bitcnt_t (*group_get_order_bits)(const ecc_group*);
	/* Used by ecc_group_check and ecc_group_check_discriminant */
	int (*group_check_discriminant)(const ecc_group*);

	/* Used by ecc_point_alloc, ecc_point_free, and ecc_point_copy */
	int (*point_init)(ecc_point*);
	int (*point_clear)(ecc_point*);
	int (*point_copy)(ecc_point*, const ecc_point*);
	/* Used by ecc_point_set_infinity, ecc_point_set_jacobian_GFp,
	 * ecc_point_set_affine, ecc_point_set_compressed */
	int (*point_set_infinity)(ecc_point*, const ecc_group*);
	int (*point_set_jacobian)(ecc_point*, mpz_srcptr,
			mpz_srcptr, mpz_srcptr, const ecc_group*);
	int (*point_set_affine)(ecc_point*, mpz_srcptr, mpz_srcptr, const ecc_group*);
	int (*point_set_compressed)(ecc_point*, mpz_srcptr, int, const ecc_group*);
	/* Used by ecc_point_get_jacobian_GFp and ecc_point_get_affine */
	int (*point_get_jacobian)(mpz_ptr, mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
	int (*point_get_affine)(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);

	/* Used by ecc_point_add, ecc_point_double, ecc_point_invert */
	int (*add)(ecc_point*, const ecc_point*, const ecc_point*, const ecc_group*);
	int (*dbl)(ecc_point*, const ecc_point*, const ecc_group*);
	int (*invert)(ecc_point*, const ecc_point*, const ecc_group*);
	/* Used by ecc_point_is_infinity, ecc_point_is_on_curve, and ecc_point_equal */
	int (*is_infinity)(const ecc_point*, const ecc_group*);
	int (*is_on_curve)(const ecc_point*, const ecc_group*);
	int (*point_equal)(const ecc_point*, const ecc_point*, const ecc_group*);
	/* Used by ecc_point_make_affine and ecc_points_make_affine */
	int (*make_affine)(ecc_point*, const ecc_point*, const ecc_group*);
	int (*points_make_affine)(ecc_point* const[], const ecc_point* const[],
			size_t, const ecc_group*);
	/* Used by ecc_point_mul, ecc_points_mul, ecc_point_precompute_mul,
	 * and ecc_point_have_precompute_mul
	 */
	/* mul() calculates the value
	 *
	 *   r := G * scalar
	 *      + points[0] * scalars[0]
	 *      + ...
	 *      + points[num-1] * scalars[num-1].
	 *
	 * For a fixed-point multiplication (scalar != NULL, num == 0)
	 * or a variable-point multiplication (scalar == NULL, num == 1),
	 * mul() must use a constant time algorithm: in both cases callers
	 * should provide an input scalar (either scalar or scalars[0])
	 * in the range [0, ec_group_order); for robustness, implementers
	 * should handle the case when the scalar has not been reduced, but
	 * may treat it as an unusual input, without any constant-timeness
	 * guarentee.
	 */
	int (*mul)(ecc_point *r, const ecc_point *const points[],
			const mpz_srcptr *scalars, size_t num,
			mpz_srcptr scalar, const ecc_group *group);
	int (*precompute_mul)(ecc_group*);
	int (*have_precompute_mul)(const ecc_group*);

	/* Internal Functions */
	int (*field_mul)(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
	int (*field_sqr)(mpz_ptr, mpz_srcptr, const ecc_group*);
	int (*field_div)(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
	int (*field_inv)(mpz_ptr, mpz_srcptr, const ecc_group*);
	int (*field_encode)(mpz_ptr, mpz_srcptr, const ecc_group*);
	int (*field_decode)(mpz_ptr, mpz_srcptr, const ecc_group*);
	int (*field_set_to_one)(mpz_ptr, const ecc_group*);
};

struct ec_group_st {
	const ecc_method *method;
	/* Generator Point (optional) */
	ecc_point *G;
	mpz_ptr order, cofactor;
	/* NID for the curve name, if it is known */
	int curve_name;
	/* Optional seed used to generate parameters */
	uint8_t *seed;	size_t seed_len;

	/* Field Specification. For curves over GF(p), this is the modulus; for
	 * curves over GF(2^m), this is the irreducible characteristic polynomial. */
	mpz_ptr field;
	/* Field Specification for curves over GF(2^m). The irreducible polynomial
	 * p(t) is then of the form: t^poly[0] + t^poly[1] + ... t^poly[k], where
	 * m = poly[0] > poly[1] > ... > poly[k] = 0. The array is terminated with
	 * poly[k+1] = -1. All elliptic curve irreducible polynomials have at most
	 * five non-zero terms. */
	int poly[6];

	/* Curve coefficients (assumption is made that mpz_t objects can be used
	 * for all kinds of fields, not just GF(p)). For characteristic > 3, the
	 * curve is defined by a Weierstrass equation of the form
	 * y^2 = x^3 + a*x + b. For characteristic 2, the curve is defined by an
	 * equation of the form y^2 + x*y = x^3 + a*x^2 + b. */
	mpz_t a, b;

	/* enable optimized point arithmetic for special cases */
	int a_is_minus_3;
	/* method-specific (i.e. Montgomery structure) */
	void *field_data1, *field_data2, *field_data3;
	int (*field_mod_func)(mpz_ptr, mpz_srcptr, mpz_srcptr);
};

struct ec_point_st {
	const ecc_method *method;
	/* NID for the curve name, if it is known */
	int curve_name;
	/* Jacobian Projective Coordinates:
	 * Point (X, Y, Z) represents (X/Z^2, Y/Z^3) if Z != 0 */
	mpz_ptr X, Y, Z;
	/* Enables optimized arithmetic for special cases */
	int Z_is_one;
};

int ec_group_simple_order_bits(const ecc_group *group);

/* Simple methods over GF(p) */
int ecc_GFp_simple_group_init(ecc_group*);
int ecc_GFp_simple_group_clear(ecc_group*);
int ecc_GFp_simple_group_copy(ecc_group*, const ecc_group*);
int ecc_GFp_simple_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_simple_group_get_curve(mpz_ptr, mpz_ptr, mpz_ptr, const ecc_group*);
mp_bitcnt_t ecc_GFp_simple_group_get_degree(const ecc_group*);
int ecc_GFp_simple_group_check_discriminant(const ecc_group*);
int ecc_GFp_simple_point_init(ecc_point*);
int ecc_GFp_simple_point_clear(ecc_point*);
int ecc_GFp_simple_point_copy(ecc_point*, const ecc_point*);
int ecc_GFp_simple_point_set_infinity(ecc_point*, const ecc_group*);
int ecc_GFp_simple_point_set_jacobian(ecc_point*, mpz_srcptr,
		mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_point_set_affine(ecc_point*, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_point_set_compressed(ecc_point*, mpz_srcptr, int, const ecc_group*);
int ecc_GFp_simple_point_get_jacobian(mpz_ptr, mpz_ptr, mpz_ptr,
		const ecc_point*, const ecc_group*);
int ecc_GFp_simple_point_get_affine(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_add(ecc_point*, const ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_double(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_invert(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_is_infiniry(const ecc_point*, const ecc_group*);
int ecc_GFp_simple_is_on_curve(const ecc_point*, const ecc_group*);
int ecc_GFp_simple_point_equal(const ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_make_affine(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GFp_simple_points_make_affine(ecc_point* const[], const ecc_point* const[],
		size_t, const ecc_group*);
int ecc_GFp_simple_mul(ecc_point*, const ecc_point* const[], const mpz_srcptr*,
		size_t, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_field_mul(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_field_sqr(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_field_div(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_simple_field_inv(mpz_ptr, mpz_srcptr, const ecc_group*);

/* Simple methods over GF(2^m) */
int ecc_GF2m_simple_group_init(ecc_group*);
int ecc_GF2m_simple_group_clear(ecc_group*);
int ecc_GF2m_simple_group_copy(ecc_group*, const ecc_group*);
int ecc_GF2m_simple_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GF2m_simple_group_get_curve(mpz_ptr, mpz_ptr, mpz_ptr, const ecc_group*);
mp_bitcnt_t ecc_GF2m_simple_group_get_degree(const ecc_group*);
int ecc_GF2m_simple_group_check_discriminant(const ecc_group*);
int ecc_GF2m_simple_point_init(ecc_point*);
int ecc_GF2m_simple_point_clear(ecc_point*);
int ecc_GF2m_simple_point_copy(ecc_point*, const ecc_point*);
int ecc_GF2m_simple_point_set_infinity(ecc_point*, const ecc_group*);
int ecc_GF2m_simple_point_set_affine(ecc_point*, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GF2m_simple_point_get_affine(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_add(ecc_point*, const ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_double(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_invert(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_is_infiniry(const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_is_on_curve(const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_point_equal(const ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_make_affine(ecc_point*, const ecc_point*, const ecc_group*);
int ecc_GF2m_simple_points_make_affine(ecc_point* const[], const ecc_point* const[],
		size_t, const ecc_group*);
int ecc_GF2m_simple_mul(ecc_point*, const ecc_point* const[], const mpz_srcptr*,
		size_t, mpz_srcptr, const ecc_group*);
int ecc_GF2m_simple_field_mul(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GF2m_simple_field_sqr(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GF2m_simple_field_div(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GF2m_simple_field_inv(mpz_ptr, mpz_srcptr, const ecc_group*);

/* Methods over GF(p) with montgomery multiplication */
int ecc_GFp_montgomery_group_init(ecc_group*);
int ecc_GFp_montgomery_group_clear(ecc_group*);
int ecc_GFp_montgomery_group_copy(ecc_group*, const ecc_group*);
int ecc_GFp_montgomery_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_montgomery_field_mul(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_montgomery_field_sqr(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_montgomery_field_inv(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_montgomery_field_encode(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_montgomery_field_decode(mpz_ptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_montgomery_field_set_to_one(mpz_ptr, const ecc_group*);

/* Methods over GF(p) with NIST optimizations */
int ecc_GFp_nist_group_copy(ecc_group*, const ecc_group*);
int ecc_GFp_nist_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_nist_field_mul(mpz_ptr, mpz_srcptr, mpz_srcptr, const ecc_group*);
int ecc_GFp_nist_field_sqr(mpz_ptr, mpz_srcptr, const ecc_group*);

/* Methods optimized for the NIST curve P-224 */
int ecc_GFp_nist224_group_init(ecc_group*);
int ecc_GFp_nist224_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_nist224_point_get_affine(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
int ecc_GFp_nist224_mul(ecc_point*, const ecc_point* const[], const mpz_srcptr*,
		size_t, mpz_srcptr, const ecc_group*);

/* Methods optimized for the NIST curve P-256 */
int ecc_GFp_nist256_group_init(ecc_group*);
int ecc_GFp_nist256_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_nist256_point_get_affine(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
int ecc_GFp_nist256_mul(ecc_point*, const ecc_point* const[], const mpz_srcptr*,
		size_t, mpz_srcptr, const ecc_group*);

/* Methods optimized for the NIST curve P-521 */
int ecc_GFp_nist521_group_init(ecc_group*);
int ecc_GFp_nist521_group_set_curve(ecc_group*, mpz_srcptr, mpz_srcptr, mpz_srcptr);
int ecc_GFp_nist521_point_get_affine(mpz_ptr, mpz_ptr, const ecc_point*, const ecc_group*);
int ecc_GFp_nist521_mul(ecc_point*, const ecc_point* const[], const mpz_srcptr*,
		size_t, mpz_srcptr, const ecc_group*);

/* NIST utility functions */
void ec_GFp_nistp_points_make_affine_internal(size_t num, void *point_array,
		size_t felem_size, void *tmp_felems, void (*felem_one) (void *out),
		int (*felem_is_zero) (const void *in),
		void (*felem_assign) (void *out, const void *in),
		void (*felem_square) (void *out, const void *in),
		void (*felem_mul) (void *out, const void *in1, const void *in2),
		void (*felem_inv) (void *out, const void *in),
		void (*felem_contract) (void *out, const void *in));
void ec_GFp_nistp_recode_scalar_bits(unsigned char *sign,
		unsigned char *digit, unsigned char in);

#ifdef __cplusplus
}
#endif

#endif /* ECC_INTERNAL_H_ */
