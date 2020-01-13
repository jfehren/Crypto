/*
 * crypto-gmp-random.c
 *
 *  Created on: Oct 31, 2019, 11:34:54 AM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifdef CRYPTO_ALLOCA
#include <alloca.h>
#endif

#include "crypto-gmp.h"
#include "tmp-alloc.h"

/* Returns a random number x, 0 <= x < 2^bits */
void
crypto_mpz_random_size(mpz_ptr x, unsigned bits,
		void *random_ctx, crypto_random_func *random) {
	unsigned length = (bits + 7) / 8;
	TMP_DECL(data, uint8_t);
	TMP_ALLOC(data, length);

	random(random_ctx, data, length);
	crypto_mpz_set_str_256_u(x, length, data);

	if ((bits & 7) != 0) {
		mpz_fdiv_r_2exp(x, x, bits);
	}

	TMP_FREE(data);
}

/* Returns a random number x, 0 <= x < n */
void
crypto_mpz_random(mpz_ptr x, mpz_srcptr n, void *random_ctx, crypto_random_func *random) {
	/* Add a few bits extra, to decrease the bias from the final modulo
	 * operation. NIST FIPS 186-3 specifies 64 extra bits, for use with
	 * DSA. */
	crypto_mpz_random_size(x, mpz_sizeinbase(n, 2) + 64, random_ctx, random);
	mpz_fdiv_r(x, x, n);
}

