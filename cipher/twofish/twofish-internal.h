/*
 * twofish-internal.h
 *
 *  Created on: Mar 4, 2019, 2:31:45 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef TWOFISH_INTERNAL_H_
#define TWOFISH_INTERNAL_H_

#include "twofish.h"

/* Name mangling */
#define _twofish_Q0 _crypto_twofish_Q0
#define _twofish_Q1 _crypto_twofish_Q1
#define _twofish_MDS _crypto_twofish_MDS
#define _twofish_RS _crypto_twofish_RS

extern const uint8_t  _twofish_Q0[0x100];
extern const uint8_t  _twofish_Q1[0x100];
extern const uint32_t _twofish_MDS[4][0x100];
extern const uint32_t _twofish_RS[8][0x100];

#endif /* TWOFISH_INTERNAL_H_ */
