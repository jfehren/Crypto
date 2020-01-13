/*
 * dsa.c
 *
 *  Created on: Oct 31, 2019, 12:40:42 PM
 *      Author: Joshua Fehrenbach
 */

#include "dsa.h"

void
dsa_params_init(struct dsa_params *params) {
	mpz_init(params->p);
	mpz_init(params->q);
	mpz_init(params->g);
}

void
dsa_params_clear(struct dsa_params *params) {
	mpz_clear(params->g);
	mpz_clear(params->q);
	mpz_clear(params->p);
}

void
dsa_signature_init(struct dsa_signature *sig) {
	mpz_init(sig->r);
	mpz_init(sig->s);
}

void
dsa_signature_clear(struct dsa_signature *sig) {
	mpz_clear(sig->s);
	mpz_clear(sig->r);
}
