/*
 * dsa-genkey.c
 *
 *  Created on: Oct 31, 2019, 1:21:51 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>

#include "dsa.h"

/* Valid sizes, according to FIPS 186-3 are (1024, 160), (2048, 224),
 * (2048, 256), (3072, 256), but we accept any q_bits >= 30 and any
 * p_bits >= q_bits + 30 */
void
dsa_generate_keypair(const struct dsa_params *params, mpz_ptr y, mpz_ptr x,
		void *random_ctx, crypto_random_func *random) {
	/* Generate private key x and public key y using domain parameters (p,q,g) */
	mpz_t r;
	mpz_init_set(r, params->q);
	mpz_sub_ui(r, r, 2);
	crypto_mpz_random(x, r, random_ctx, random);
	mpz_add_ui(x, x, 1);
	mpz_powm(y, params->g, x, params->p);
	mpz_clear(r);
}

