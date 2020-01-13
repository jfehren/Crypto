/*
 * der2dsa.c
 *
 *  Created on: Oct 31, 2019, 2:55:02 PM
 *      Author: Joshua Fehrenbach
 */

#include "dsa.h"
#include "asn1.h"

#define GET(i, x, l) \
	(asn1_der_iterator_next((i)) == ASN1_ITERATOR_PRIMITIVE	\
		&& (i)->type == ASN1_INTEGER \
		&& asn1_der_get_bignum((i), (x), (l)) \
		&& mpz_sgn((x)) > 0)

/* if q_bits > 0, q is required to be of exactly this size */
int
dsa_params_from_der_iterator(struct dsa_params *params,
		struct asn1_der_iterator *i, unsigned p_max_bits, unsigned q_bits) {
	/* DSS-Params ::= SEQUENCE {
	 *     p  INTEGER,
	 *     q  INTEGER,
	 *     q  INTEGER
	 * }
	 */
	if (i->type == ASN1_INTEGER && asn1_der_get_bignum(i, params->p, p_max_bits)
			&& mpz_sgn(params->p) > 0) {
		unsigned p_bits = mpz_sizeinbase(params->p, 2);
		return (GET(i, params->q, q_bits ? q_bits : p_bits)
			&& (q_bits == 0 || mpz_sizeinbase(params->q, 2) == q_bits)
			&& mpz_cmp(params->q, params->p) < 0
			&& GET(i, params->g, p_bits)
			&& mpz_cmp(params->g, params->p) < 0
			&& asn1_der_iterator_next(i) == ASN1_ITERATOR_END);
	} else {
		return 0;
	}
}

int
dsa_public_key_from_der_iterator(const struct dsa_params *params,
		mpz_ptr pub, struct asn1_der_iterator *i) {
	/* DSAPublicKey ::= INTEGER */
	return (i->type == ASN1_INTEGER
		&& asn1_der_get_bignum(i, pub, mpz_sizeinbase(params->p, 2))
		&& mpz_sgn(pub) > 0 && mpz_cmp(pub, params->p) < 0);
}

int
dsa_openssl_private_key_from_der_iterator(struct dsa_params *params,
		struct asn1_der_iterator *i, mpz_ptr pub, mpz_ptr priv,
		unsigned p_max_bits) {
	/* DSAPrivateKey ::= SEQUENCE {
	 *     version   Version,
	 *     p         INTEGER,
	 *     q         INTEGER,
	 *     g         INTEGER,
	 *     pub_key   INTEGER, -- y
	 *     priv_key  INTEGER, -- x
	 * }
	 */
	uint32_t version;
	if (i->type == ASN1_SEQUENCE
			&& asn1_der_decode_constructed_last(i) == ASN1_ITERATOR_PRIMITIVE
			&& i->type == ASN1_INTEGER
			&& asn1_der_get_uint32(i, &version)
			&& version == 0
			&& GET(i, params->p, p_max_bits)) {
		unsigned p_bits = mpz_sizeinbase(params->p, 2);
		return (GET(i, params->q, DSA_SHA1_Q_BITS)
			&& GET(i, params->g, p_bits)
			&& mpz_cmp(params->g, params->p) < 0
			&& GET(i, pub, p_bits)
			&& mpz_cmp(pub, params->p) < 0
			&& GET(i, priv, DSA_SHA1_Q_BITS)
			&& asn1_der_iterator_next(i) == ASN1_ITERATOR_END);
	} else {
		return 0;
	}
}

int
dsa_openssl_private_key_from_der(struct dsa_params *params,
		mpz_ptr pub, mpz_ptr priv, const uint8_t *data, size_t length,
		unsigned p_max_bits) {
	struct asn1_der_iterator i;
	return (asn1_der_iterator_first(&i, data, length) == ASN1_ITERATOR_CONSTRUCTED
		&& dsa_openssl_private_key_from_der_iterator(params, &i, pub, priv, p_max_bits));
}
