/*
 * ecc.h
 *
 *  Created on: Dec 16, 2019, 2:31:47 PM
 *      Author: Joshua Fehrenbach
 *
 * ECC functionality implemented as a sort of fusion of the GNU Nettle
 * ECC implementation and the OpenSSL ECC implementation.
 */

#ifndef ECC_H_
#define ECC_H_

#include "crypto.h"

#include <gmp.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define ecc_GFp_simple_method crypto_ecc_GFp_simple_method
#define ecc_GFp_montgomery_method crypto_ecc_GFp_montgomery_method
#define ecc_GFp_nist_method crypto_ecc_GFp_nist_method
#define ecc_GFp_nistp224_method crypto_ecc_GFp_nistp224_method
#define ecc_GFp_nistp256_method crypto_ecc_GFp_nistp256_method
#define ecc_GFp_nistp521_method crypto_ecc_GFp_nistp521_method
#define ecc_GF2m_simple_method crypto_ecc_GF2m_simple_method

#define ecc_group_alloc crypto_ecc_group_alloc
#define ecc_group_free crypto_ecc_group_free
#define ecc_group_copy crypto_ecc_group_copy
#define ecc_group_duplicate crypto_ecc_group_duplicate

#define ecc_group_is_GFp crypto_ecc_group_is_GFp
#define ecc_group_is_GF2m crypto_ecc_group_is_GF2m

#define ecc_group_get_method crypto_ecc_group_get_method
#define ecc_group_get_generator crypto_ecc_group_get_generator
#define ecc_group_get_order crypto_ecc_group_get_order
#define ecc_group_get_order_bits crypto_ecc_group_get_order_bits
#define ecc_group_get_cofactor crypto_ecc_group_get_cofactor
#define ecc_group_get_field crypto_ecc_group_get_field
#define ecc_group_get_seed crypto_ecc_group_get_seed
#define ecc_group_get_seed_len crypto_ecc_group_get_seed_len
#define ecc_group_get_name crypto_ecc_group_get_name
#define ecc_group_get_degree crypto_ecc_group_get_degree
#define ecc_group_get_curve crypto_ecc_group_get_curve
#define ecc_group_get_GFp crypto_ecc_group_get_GFp
#define ecc_group_get_GF2m crypto_ecc_group_get_GF2m

#define ecc_group_set_generator crypto_ecc_group_set_generator
#define ecc_group_set_seed crypto_ecc_group_set_seed
#define ecc_group_set_name crypto_ecc_group_set_name
#define ecc_group_set_curve crypto_ecc_group_set_curve
#define ecc_group_set_GFp crypto_ecc_group_set_GFp
#define ecc_group_set_GF2m crypto_ecc_group_set_GF2m
#define ecc_group_set_named crypto_ecc_group_set_named

#define ecc_group_check crypto_ecc_group_check
#define ecc_group_check_discriminant crypto_ecc_group_check_discriminant
#define ecc_group_equal crypto_ecc_group_equal
#define ecc_group_nid_to_nist crypto_ecc_group_nid_to_nist
#define ecc_group_nist_to_nid crypto_ecc_group_nist_to_nid

#define ecc_point_alloc crypto_ecc_point_alloc
#define ecc_point_free crypto_ecc_point_free
#define ecc_point_copy crypto_ecc_point_copy
#define ecc_point_duplicate crypto_ecc_point_duplicate

#define ecc_point_get_method crypto_ecc_point_get_method
#define ecc_point_get_jacobian_GFp crypto_ecc_point_get_jacobian_GFp
#define ecc_point_get_affine crypto_ecc_point_get_affine
#define ecc_point_get_affine_GFp crypto_ecc_point_get_affine_GFp
#define ecc_point_get_affine_GF2m crypto_ecc_point_get_affine_GF2m

#define ecc_point_set_infinity crypto_ecc_point_set_infinity
#define ecc_point_set_jacobian_GFp crypto_ecc_point_set_jacobian_GFp
#define ecc_point_set_affine crypto_ecc_point_set_affine
#define ecc_point_set_affine_GFp crypto_ecc_point_set_affine_GFp
#define ecc_point_set_affine_GF2m crypto_ecc_point_set_affine_GF2m
#define ecc_point_set_compressed crypto_ecc_point_set_compressed
#define ecc_point_set_compressed_GFp crypto_ecc_point_set_compressed_GFp
#define ecc_point_set_compressed_GF2m crypto_ecc_point_set_compressed_GF2m

#define ecc_point_is_infinity crypto_ecc_point_is_infinity
#define ecc_point_is_on_curve crypto_ecc_point_is_on_curve
#define ecc_point_equal crypto_ecc_point_equal

#define ecc_point_add crypto_ecc_point_add
#define ecc_point_double crypto_ecc_point_double
#define ecc_point_invert crypto_ecc_point_invert
#define ecc_point_mul crypto_ecc_point_mul
#define ecc_points_mul crypto_ecc_points_mul
#define ecc_point_make_affine crypto_ecc_point_make_affine
#define ecc_points_make_affine crypto_ecc_points_make_affine

/* A collection of function pointers for a class of elliptic curve. Defined internally. */
typedef struct ec_method_st ecc_method;
/* An elliptic curve group object. Defined internally. */
typedef struct ec_group_st ecc_group;
/* An elliptic curve point object, which defines a point
 * on an elliptic curve group. Defined internally. */
typedef struct ec_point_st ecc_point;

typedef struct {
	int nid;	/* The NID of the OID of the named curve, if it is a named curve */
	const char *comment;	/* Brief description string for the curve */
} ec_builtin_curve;


/* A NULL-terminated list of pointers to ec_builtin_curve objects
 * that defines all supported named curves */
#define ecc_builtin_curves crypto_ecc_builtin_curves
__CRYPTO_DECLSPEC extern const ec_builtin_curve * const ecc_builtin_curves[];


__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_simple_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_montgomery_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_nist_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_nistp224_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_nistp256_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GFp_nistp521_method(void) ATTRIBUTE(const);

__CRYPTO_DECLSPEC const ecc_method*
ecc_GF2m_simple_method(void) ATTRIBUTE(const);


/* Creates a new ecc_group object with the indicated function set.
 * The pointer returned by this function must be free'd by a call
 * to ecc_group_free.
 *
 * method - The ecc_method object to use
 *
 * returns the newly created ecc_group object, or NULL on error
 */
__CRYPTO_DECLSPEC ecc_group*
ecc_group_alloc(const ecc_method * method) ATTRIBUTE(malloc);

/* Destroys an ecc_group object created by ecc_group_alloc or ecc_group_duplicate. */
__CRYPTO_DECLSPEC void
ecc_group_free(ecc_group *ec);

/* Sets an ecc_group object to a copy of another ecc_group object.
 * Both ecc_group objects must use the same ecc_method.
 *
 * dst_ec - Destination ecc_group object
 * src_ec - Source ecc_group object
 *
 * returns 1 on success, 0 on failure
 */
__CRYPTO_DECLSPEC int
ecc_group_copy(ecc_group *dst_ec, const ecc_group *src_ec);

/* Creates a new ecc_group object and copies the content of the given
 * ecc_group object to the newly created object. Behaves exactly as
 * follows:
 *
 *     ecc_group *ecc_group_duplicate(const ecc_group *src_ec) {
 *         ecc_group *group = ecc_group_alloc(ecc_group_get_method(src_ec));
 *         if (group == NULL) return NULL;
 *         if (!ecc_group_copy(group, src_ec)) {
 *             ecc_group_free(group);
 *             return NULL;
 *         }
 *         return group;
 *     }
 *
 * src_ec - Source ecc_group object
 *
 * returns the newly created ecc_group object, or NULL on error
 */
__CRYPTO_DECLSPEC ecc_group*
ecc_group_duplicate(const ecc_group *src_ec) ATTRIBUTE(malloc);



/* Checks whether the given curve group is a prime curve group,
 * which has the form
 *     y^2 = x^3 + a*x + b
 * and is defined over the field F_p = GF(p), where p is a prime number.
 *
 * ec - The curve to check
 *
 * returns 1 if the given curve is a prime curve, 0 otherwise
 */
__CRYPTO_DECLSPEC int
ecc_group_is_GFp(const ecc_group *ec) ATTRIBUTE(pure);

/* Checks whether the given curve group is a binary curve group,
 * which has the form
 *     y^2 + x*y = x^3 + a*x^2 + b
 * and is defined over the field F_2^m = GF(2^m) with characteristic
 * polynomial p, which is an irreducible polynomial of degree m with
 * coefficients defined over the field GF(2).
 *
 * ec - The curve to check
 *
 * returns 1 if the given curve is a binary curve, 0 otherwise
 */
__CRYPTO_DECLSPEC int
ecc_group_is_GF2m(const ecc_group *ec) ATTRIBUTE(pure);


/* Returns an ecc_group object's underlying ecc_method object
 *
 * ec - The ecc_group object
 *
 * returns the ecc_method object used by the ecc_group object
 */
__CRYPTO_DECLSPEC const ecc_method*
ecc_group_get_method(const ecc_group *ec) ATTRIBUTE(pure);

/* Gets the generator point of the given curve group.
 *
 * generator - A pointer to the ecc_point object to set to the
 *             generator point. Must be initialized.
 * ec - The curve group to get the generator point from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_generator(ecc_point *generator, const ecc_group *ec);

/* Gets the order of a curve group's generator point.
 *
 * order - A pointer to the mpz_t object to set to the order of
 *         the curve group's generator point. Must be initialized.
 * ec - The curve group to get the generator point's order from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_order(mpz_ptr order, const ecc_group *ec);

/* Gets the number of bits in the order of a curve group's generator
 * point, i.e. the ceiling of the base-2 logarithm of the order
 * of the generator point.
 *
 * ec - The curve group to get the generator point's order from
 *
 * returns the number of bits in the order of the curve's
 * generator point
 */
__CRYPTO_DECLSPEC mp_bitcnt_t
ecc_group_get_order_bits(const ecc_group *ec);

/* Gets the cofactor of the curve group's generator point. For
 * curve E(F_q) and generator point G with order n, where
 * q = p, with p a prime, or q = 2^m, gets the cofactor h
 * such that h = #E(F_q) / n, where #E(F_q) is the order
 * of the curve E when defined over the field F_q.
 *
 * cofactor - A pointer to the mpz_t object to set to the cofactor
 *            of the curve group's generator point. Must be initialized.
 * ec - The curve group to get the generator point's cofactor from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_cofactor(mpz_ptr cofactor, const ecc_group *ec);

/* Gets the field of a curve group. For a prime curve group E(F_p),
 * gets the prime p. For a binary curve group E(F_2^m), gets the
 * irreducible polynomial p of order m which is the characteristic
 * polynomial of the curve group's field F_2^m.
 *
 * field - A pointer to the mpz_t object to set to the curve
 *         group's field. Must be initialized.
 * ec - The curve group to get the field from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_field(mpz_ptr field, const ecc_group *ec);

/* Returns the seed used to generate the curve group's
 * parameters, if the seed is defined.
 *
 * ec - The curve group to retrieve the seed from
 *
 * returns a pointer to the curve group's seed, if it is
 * defined, otherwise returns NULL
 */
__CRYPTO_DECLSPEC const unsigned char*
ecc_group_get_seed(const ecc_group *ec) ATTRIBUTE(pure);

/* Returns the octet length of the seed used to generate
 * the curve group's parameters, if the seed is defined.
 *
 * ec - The curve group to retrieve the seed length from
 *
 * returns the curve group's seed length, in octets, if it
 * is defined, otherwise returns 0
 */
__CRYPTO_DECLSPEC size_t
ecc_group_get_seed_len(const ecc_group *ec) ATTRIBUTE(pure);

/* Returns the curve name of a curve group, if its name is set.
 *
 * ec - The curve group to get the name of
 *
 * returns the NID of the curve group OID, or 0 if it is not set
 */
__CRYPTO_DECLSPEC int
ecc_group_get_name(const ecc_group *ec) ATTRIBUTE(pure);

/* Returns the number of bits required to represent an element of
 * a curve group's field.
 *
 * ec - The curve group
 *
 * returns the number of bits required to represent an element of
 * the curve group's field
 */
__CRYPTO_DECLSPEC mp_bitcnt_t
ecc_group_get_degree(const ecc_group *ec) ATTRIBUTE(pure);

/* Gets the parameters of the elliptic curve defined by an ecc_group object.
 * For a curve over GF(p), the curve is defined by y^2 = x^3 + a*x + b
 * with prime modulus p that defines the field GF(p). For a curve over GF(2^m),
 * the curve is defined by y^2 + x*y = x^3 + a*x^2 + b with irreducible
 * polynomial p of degree m that is the characteristic polynomial of the field
 * GF(2^m).
 *
 * p - A pointer to the mpz_t object to set to the field parameter p
 * a - A pointer to the mpz_t object to set to the curve coefficient a
 * b - A pointer to the mpz_t object to set to the curve coefficient b
 * ec - An ecc_group object representing the curve group to get the
 *      parameters p, a, and b from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_curve(mpz_ptr p, mpz_ptr a, mpz_ptr b, const ecc_group *ec);

/* Gets the parameters of a prime curve group E(F_p), which is in the form
 *     y^2 = x^3 + a*x + b
 * and is defined over the field F_p = GF(p), where p is a prime number.
 * The curve group must be defined over the field GF(p).
 *
 * p - A pointer to the mpz_t object to set to the prime modulus p
 * a - A pointer to the mpz_t object to set to the curve coefficient a
 * b - A pointer to the mpz_t object to set to the curve coefficient b
 * ec - The curve group to get the parameters p, a, and b from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_GFp(mpz_ptr p, mpz_ptr a, mpz_ptr b, const ecc_group *ec);

/* Gets the parameters of a binary curve group E(F_2^m), which is in the form
 *     y^2 + x*y = x^3 + a*x^2 + b
 * and is defined over the field F_2^m = GF(2^m) with characteristic polynomial
 * p, which is an irreducible polynomial of degree m with coefficients defined
 * over the field GF(2).
 * The curve group must be defined over the field GF(2^m).
 *
 * p - A pointer to the mpz_t object to set to the irreducible polynomial p
 * a - A pointer to the mpz_t object to set to the curve coefficient a
 * b - A pointer to the mpz_t object to set to the curve coefficient b
 * ec - The curve group to get the parameters p, a, and b from
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_get_GF2m(mpz_ptr p, mpz_ptr a, mpz_ptr b, const ecc_group *ec);


/* Sets the generator point of the given curve group.
 *
 * ec - The curve group to set the generator point of
 * generator - The new generator point
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_generator(ecc_group *ec, const ecc_point *generator,
		mpz_srcptr order, mpz_srcptr cofactor);

/* Sets the seed used to generate the curve group's parameters.
 *
 * ec - The curve group to set the seed of
 * seed - The seed used to generate the curve group's parameters
 * seed_len - The length of the seed, in octets
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_seed(ecc_group *ec, const unsigned char *seed, size_t seed_len);

/* Sets the name of a curve group.
 *
 * ec - The ecc_group object to set the name of
 * nid - The NID of the OID of the named curve
 */
__CRYPTO_DECLSPEC void
ecc_group_set_name(ecc_group *ec, int nid);

/* Sets the parameters of the elliptic curve defined by an ecc_group object.
 * For a curve over GF(p), the curve is defined by y^2 = x^3 + a*x + b
 * with prime modulus p that defines the field GF(p). For a curve over GF(2^m),
 * the curve is defined by y^2 + x*y = x^3 + a*x^2 + b with irreducible
 * polynomial p of degree m that is the characteristic polynomial of the field
 * GF(2^m).
 *
 * ec - The curve group object to set the parameters of
 * p - The new field parameter p
 * a - The new coefficient a of the curve equation
 * a - The new coefficient b of the curve equation
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_curve(ecc_group *ec, mpz_srcptr p, mpz_srcptr a, mpz_srcptr b);

/* Sets a curve groups to a prime curve group E(F_p) with the
 * given parameters. The curve is in the form
 *     y^2 = x^3 + a*x + b
 * and is defined over the field F_p = GF(p), where p is a prime number.
 * a and b must be given such that 4*a^3 + 27*b^2 != 0 (mod p), and the
 * curve group must be defined over the field GF(p).
 *
 * ec - The curve group object to set the parameters of
 * p - The new prime field modulus p
 * a - The new coefficient a of the curve equation
 * b - The new coefficient b of the curve equation
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_GFp(ecc_group *ec, mpz_srcptr p, mpz_srcptr a, mpz_srcptr b);

/* Sets a curve groups to a binary curve group E(F_2^m) with the
 * given parameters. The curve is in the form
 *     y^2 + x*y = x^3 + a*x^2 + b
 * and is defined over the field F_2^m = GF(2^m) with characteristic
 * polynomial p, which is an irreducible polynomial of degree m with
 * coefficients defined over the field GF(2).
 * b must b non-zero, and the curve group must be defined over the
 * field GF(2^m).
 *
 * ec - The curve group object to set the parameters of
 * p - The new irreducible characteristic polynomial p
 * a - The new coefficient a of the curve equation
 * b - The new coefficient b of the curve equation
 *
 * returns 1 on success, 0 on failure or error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_GF2m(ecc_group *ec, mpz_srcptr p, mpz_srcptr a, mpz_srcptr b);

/* Sets a curve group to the named curve who's OID has the given NID.
 * The named curve must be of a type that is compatible with the
 * ecc_group object's underlying ecc_method object.
 *
 * ec - The curve group to set to the named curve
 * nid - The NID of the OID of the named curve
 *
 * returns 1 on success, 0 if nid is not a (supported) named curve
 * or on error
 */
__CRYPTO_DECLSPEC int
ecc_group_set_named(ecc_group *ec, int nid);



/* Checks whether the parameters of the given curve group are valid.
 *
 * ec - The curve group to check the parameters of
 *
 * returns 1 if the curve group parameters are valid, 0 otherwise
 */
__CRYPTO_DECLSPEC int
ecc_group_check(const ecc_group *ec);

/* Checks whether the discriminant of a curve group's equation
 * is valid for the curve group.
 * For a prime curve group E(F_p), which have an equation of the form
 *     y^2 = x^3 + a*x + b,
 * ensures that 4*a^3 + 27*b^2 != 0 (mod p).
 * For a binary curve group E(F_2^m), which have an equation of the form
 *     y^2 + x*y = x^3 + a*x^2 + b,
 * ensures that b != 0 (mod p), where p is the characteristic polynomial
 * of the curve group's field F_2^m and is an irreducible polynomial of
 * degree m with coefficients defined over the field GF(2).
 *
 * ec - The curve to check the discriminant of
 *
 * returns 1 if the discriminant is valid, 0 otherwise
 */
__CRYPTO_DECLSPEC int
ecc_group_check_discriminant(const ecc_group *ec);

/* Checks whether the given two ecc_group objects represent the
 * same curve group.
 *
 * ec1 - The first ecc_group object
 * ec2 - The second ecc_group object
 *
 * returns 0 if ec1 and ec2 both represent the same curve group, 1
 * if not, -1 on error
 */
__CRYPTO_DECLSPEC int
ecc_group_equal(const ecc_group *ec1, const ecc_group *ec2) ATTRIBUTE(pure);


/* Returns the NIST name for the named curve who's OID has the given NID.
 *
 * ec - The NID of the named curve
 *
 * returns the NIST for the named curve who's OID has the given NID,
 * if the curve is a NIST named curve, otherwise returns NULL
 */
__CRYPTO_DECLSPEC const char*
ecc_group_nid_to_nist(int nid) ATTRIBUTE(pure);

/* Returns the NID of the OID of the given NIST name for a named curve.
 *
 * nist - The NIST name for a named curve
 *
 * returns the NID of the OID of the given NIST name for a named curve,
 * if the name is a valid NIST named curve, otherwise returns NULL
 */
__CRYPTO_DECLSPEC int
ecc_group_nist_to_nid(const char *nist) ATTRIBUTE(pure);




/* Creates a new ecc_point object for the specified ecc_group object.
 *
 * ec - The curve group on which the new point will be defined
 *
 * returns the newly created ecc_point object, or NULL on error
 */
__CRYPTO_DECLSPEC ecc_point*
ecc_point_alloc(const ecc_group *ec);

/* Destroys an ecc_point object created by ecc_point_alloc or ecc_point_duplicate. */
__CRYPTO_DECLSPEC void
ecc_point_free(ecc_point *pt);

/* Sets an ecc_point object to a copy of another ecc_point object.
 * The two ecc_point objects need not be defined for the same curve group.
 *
 * dst_pt - Destination ecc_point object
 * src_pt - Source ecc_point object
 *
 * returns 1 on success, 0 on failure
 */
__CRYPTO_DECLSPEC int
ecc_point_copy(ecc_point *dst_pt, const ecc_point *src_pt);

/* Creates a new ecc_point object and copies the contents of the given
 * ecc_point object to the newly created object.
 *
 * src_pt - Source ecc_point object
 *
 * returns the newly created ecc_point object, or NULL on error
 */
__CRYPTO_DECLSPEC ecc_point*
ecc_point_duplicate(const ecc_point *src_pt);


__CRYPTO_DECLSPEC const ecc_method*
ecc_point_get_method(const ecc_point *pt) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC int
ecc_point_get_jacobian_GFp(mpz_ptr x, mpz_ptr y, mpz_ptr z,
		const ecc_point *pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_get_affine(mpz_ptr x, mpz_ptr y, const ecc_point *pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_get_affine_GFp(mpz_ptr x, mpz_ptr y, const ecc_point *pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_get_affine_GF2m(mpz_ptr x, mpz_ptr y, const ecc_point *pt, const ecc_group *ec);


__CRYPTO_DECLSPEC int
ecc_point_set_infinity(ecc_point *pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_jacobian_GFp(ecc_point *pt, mpz_srcptr x,
		mpz_srcptr y, mpz_srcptr z, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_affine(ecc_point *pt, mpz_srcptr x, mpz_srcptr y, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_affine_GFp(ecc_point *pt, mpz_srcptr x, mpz_srcptr y, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_affine_GF2m(ecc_point *pt, mpz_srcptr x, mpz_srcptr y, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_compressed(ecc_point *pt, mpz_srcptr x, int y_bit, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_compressed_GFp(ecc_point *pt, mpz_srcptr x, int y_bit, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_set_compressed_GF2m(ecc_point *pt, mpz_srcptr x, int y_bit, const ecc_group *ec);


__CRYPTO_DECLSPEC int
ecc_point_is_infinity(const ecc_point *pt, const ecc_group *ec) ATTRIBUTE(pure);

__CRYPTO_DECLSPEC int
ecc_point_is_on_curve(const ecc_point *pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_equal(const ecc_point *pt1, const ecc_point *pt2, const ecc_group *ec);


__CRYPTO_DECLSPEC int
ecc_point_add(ecc_point *r, const ecc_point *a, const ecc_point *b, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_double(ecc_point *r, const ecc_point *a, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_invert(ecc_point *r, const ecc_point *a, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_mul(ecc_point *r, const ecc_point *q, mpz_srcptr m,
		mpz_srcptr n, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_points_mul(ecc_point *r, const ecc_point *const q[], const mpz_srcptr *m,
		size_t num, mpz_srcptr n, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_point_make_affine(ecc_point *dst_pt, const ecc_point *src_pt, const ecc_group *ec);

__CRYPTO_DECLSPEC int
ecc_points_make_affine(ecc_point *const dst_pts[], const ecc_point *const src_pts[],
		size_t num, const ecc_group *ec);

#ifdef __cplusplus
}
#endif

#endif /* ECC_H_ */
