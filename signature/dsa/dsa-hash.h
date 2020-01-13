/*
 * dsa-hash.h
 *
 *  Created on: Oct 31, 2019, 12:45:14 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef DSA_DSA_HASH_H_
#define DSA_DSA_HASH_H_

#include "crypto-gmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Convert hash value to an integer. The general description of DSA in
   FIPS186-3 allows both larger and smaller q; in the the latter case,
   the hash must be truncated to the right number of bits. */
void
_dsa_hash(mpz_ptr h, unsigned bit_size, const uint8_t *digest, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* DSA_DSA_HASH_H_ */
