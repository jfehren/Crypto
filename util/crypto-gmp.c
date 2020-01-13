/*
 * crypto-gmp.c
 *
 *  Created on: Oct 24, 2019, 12:37:57 PM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "crypto-gmp.h"
#include "macros.h"


size_t
crypto_mpz_sizeinbase_256_s(mpz_srcptr x) {
	if (mpz_sgn(x) >= 0) {
		return 1 + mpz_sizeinbase(x, 2) / 8;
	} else {
		/* We'll output ~~x, so we need as many bits as for ~x */
		size_t size;
		mpz_t c;

		mpz_init(c);
		mpz_com(c, x); /* Same as c = - x - 1 = |x| + 1 */
		size = 1 + mpz_sizeinbase(c,2) / 8;
		mpz_clear(c);

		return size;
	}
}

size_t
crypto_mpz_sizeinbase_256_u(mpz_srcptr x) {
	return (7 + mpz_sizeinbase(x, 2)) / 8;
}


/* Don't use mpz_export so that we can preserve the sign */
static void
mpz_to_octets(uint8_t *s, size_t length, mpz_srcptr x, uint8_t sign) {
	size_t i, j, size;
	mp_limb_t limb;
	uint8_t *dst;

	dst = s + length - 1;
	size = mpz_size(x);
	for (i = 0; i<size; i++) {
		limb = mpz_getlimbn(x, i);
		for (j = 0; length && j < sizeof(mp_limb_t); j++) {
			*dst-- = sign ^ (limb & 0xff);
			limb >>= 8;
			length--;
		}
	}

	if (length) {
		memset(s, sign, length);
	}
}

/* Convert mpz_t to big-endian byte-string */
void
crypto_mpz_get_str_256(uint8_t *s, size_t length, mpz_srcptr x) {
	if (!length) {
		assert (!mpz_sgn(x));	/* assert that when length == 0, signum(x) == 0 */
	} else if (mpz_sgn(x) >= 0) {
		assert (crypto_mpz_sizeinbase_256_u(x) <= length);
		mpz_to_octets(s, length, x, 0);
	} else {
		mpz_t c;
		mpz_init(c);
		mpz_com(c, x);

		assert (crypto_mpz_sizeinbase_256_u(c) <= length);
		mpz_to_octets(s, length, c, 0xff);

		mpz_clear(c);
	}
}

/* Convert big-endian byte-string to mpz_t */
#define mpz_from_octets(x, s, length) mpz_import((x), (length), 1, 1, 0, 0, (s))

void
crypto_mpz_set_str_256_u(mpz_ptr x, const uint8_t *s, size_t length) {
	if (!length) {
		mpz_set_ui(x, 0);
	} else {
		mpz_from_octets(x, s, length);
	}
}

void
crypto_mpz_init_set_str_256_u(mpz_ptr x, const uint8_t *s, size_t length) {
	if (!length) {
		mpz_init_set_ui(x, 0);
	} else {
		mpz_init(x);
		mpz_from_octets(x, s, length);
	}
}

void
crypto_mpz_set_str_256_s(mpz_ptr x, const uint8_t *s, size_t length) {
	if (!length) {
		mpz_set_ui(x, 0);
	} else {
		mpz_from_octets(x, s, length);
		if ((s[0] & 0x80) != 0) {
			mpz_t t;
			mpz_init_set_ui(t, 1);
			mpz_mul_2exp(t, t, length*8);
			mpz_sub(x, x, t);
			mpz_clear(t);
		}
	}
}

void
crypto_mpz_set_str_256_s(mpz_ptr x, const uint8_t *s, size_t length) {
	mpz_init(x);
	crypto_mpz_set_str_256_s(x, s, length);
}

