/*
 * rsa-internal.h
 *
 *  Created on: Nov 4, 2019, 12:03:31 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef RSA_RSA_INTERNAL_H_
#define RSA_RSA_INTERNAL_H_

#include "rsa.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Name mangling */
#define _rsa_verify _crypto_rsa_verify
#define _rsa_check_size _crypto_rsa_check_size

int
_rsa_verify(const struct rsa_public_key *key, mpz_srcptr m, mpz_srcptr s);

size_t
_rsa_check_size(mpz_srcptr n) ATTRIBUTE(pure);

#define _RSA_SIGN(suf, key, s, ...) do { \
	if (pkcs1_rsa_##suf(s, (key)->size), __VA_ARGS__) { \
		rsa_compute_root(key, s, s); \
		return 1; \
	} else { \
		mpz_set_ui(s, 0); \
		return 0; \
	} \
} while (0)
#define _RSA_SIGN_TR(suf, pub, key, s, rand_ctx, rand_func, ...) do { \
	mpz_t __sign_tr_m; \
	int __sign_tr_result; \
	mpz_init(__sign_tr_m); \
	__sign_tr_result = (pkcs1_rsa_##suf(__sign_tr_m, (key)->size, __VA_ARGS__) \
			&& rsa_compute_root_tr(pub, key, s, __sign_tr_m, rand_ctx, rand_func)); \
	mpz_clear(__sign_tr_m); \
	return __sign_tr_result; \
} while (0)
#define _RSA_VERIFY(suf, key, s, ...) do { \
	mpz_t __verify_m; \
	int __verify_result; \
	mpz_init(__verify_m); \
	__verify_result = (pkcs1_rsa_##suf(__verify_m, (key)->size, __VA_ARGS__) \
			&& _rsa_verify(key, __verify_m, s)); \
	mpz_clear(__verify_m); \
	return __verify_result; \
} while (0)

#ifdef __cplusplus
}
#endif

#endif /* RSA_RSA_INTERNAL_H_ */
