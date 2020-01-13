/*
 * ecc-func.c
 *
 *  Created on: Dec 19, 2019, 4:04:35 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "ecc.h"
#include "ecc-internal.h"

ecc_group*
ecc_group_alloc(const ecc_method *method) {
	ecc_group *ret;
	mpz_ptr t;
	if (method == NULL) {
		return NULL;
	}
	if (method->group_init == NULL) {
		return NULL;
	}
	ret = calloc(1, sizeof(ecc_group));
	if (ret == NULL) {
		return NULL;
	}
	ret->method = method;
	t = calloc(2, sizeof(mpz_t));
	if (t == NULL) {
		free(ret);
		return NULL;
	}
	ret->order = &t[0];
	ret->cofactor = &t[1];
	if (!method->group_init(ret)) {
		free(ret);
		free(t);
		return NULL;
	}
	return ret;
}

void
ecc_group_free(ecc_group *ec) {

}

