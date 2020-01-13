/*
 * crypto-internal.c
 *
 *  Created on: Sep 3, 2019, 12:49:26 PM
 *      Author: Joshua Fehrenbach
 */

#include "crypto-internal.h"

#if __IS_x86__ && CRYPTO_FAT

int _is_x86_64 = -1;
int _is_aesni = -1;
int _is_avx = -1;
int _is_sha = -1;
int _is_pclmul = -1;

#endif
