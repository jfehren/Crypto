/*
 * dsa-hash.c
 *
 *  Created on: Oct 31, 2019, 12:46:27 PM
 *      Author: Joshua Fehrenbach
 */

#include "dsa-hash.h"

void
_dsa_hash(mpz_ptr h, unsigned bit_size, const uint8_t *digest, size_t length) {
	if (length > (bit_size+7) / 8) {
		length = (bit_size+7) / 8;
	}
	crypto_mpz_set_str_256_u(h, digest, length);
	if (8*length > bit_size) {
		/* have a few extra bits */
		mpz_tdiv_q_2exp(h, h, 8*length - bit_size);
	}
}

