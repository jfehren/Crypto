/*
 * ecc-GFp-simple.c
 *
 *  Created on: Dec 17, 2019, 5:29:52 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>

#include "ecc-internal.h"

int
ecc_GFp_simple_group_init(ecc_group *group) {
	const ecc_method *method = group->method;
	mpz_ptr tmp;
	if (method == NULL) {
		return 0;
	}
	tmp = malloc(3*sizeof(mpz_t));
	if (tmp == NULL) {
		return 0;
	}
	group->field = &tmp[0];
	group->a = &tmp[1];
	group->b = &tmp[2];
	mpz_init_set_ui(group->field, 0);
	mpz_init_set_ui(group->a, 0);
	mpz_init_set_ui(group->b, 0);
	return 1;
}

