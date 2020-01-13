/*
 * sha3-internal.c
 *
 *  Created on: Aug 19, 2019, 1:58:04 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha3-internal.h"
#include "crypto-internal.h"
#include "macros.h"
#include "memxor.h"
#include <assert.h>
#include <string.h>

void
sha3_permute_gen(uint64_t *A) {
	/* Precomputed round constants for Iota step function */
	static const uint64_t rc[24] = {
		0x0000000000000001ULL, 0X0000000000008082ULL,
		0X800000000000808AULL, 0X8000000080008000ULL,
		0X000000000000808BULL, 0X0000000080000001ULL,
		0X8000000080008081ULL, 0X8000000000008009ULL,
		0X000000000000008AULL, 0X0000000000000088ULL,
		0X0000000080008009ULL, 0X000000008000000AULL,
		0X000000008000808BULL, 0X800000000000008BULL,
		0X8000000000008089ULL, 0X8000000000008003ULL,
		0X8000000000008002ULL, 0X8000000000000080ULL,
		0X000000000000800AULL, 0X800000008000000AULL,
		0X8000000080008081ULL, 0X8000000000008080ULL,
		0X0000000080000001ULL, 0X8000000080008008ULL
	};
	uint64_t C[5], D[5], T, X;
	unsigned i, y;

	C[0] = A[ 0] ^ A[ 5] ^ A[10] ^ A[15] ^ A[20];
	C[1] = A[ 1] ^ A[ 6] ^ A[11] ^ A[16] ^ A[21];
	C[2] = A[ 2] ^ A[ 7] ^ A[12] ^ A[17] ^ A[22];
	C[3] = A[ 3] ^ A[ 8] ^ A[13] ^ A[18] ^ A[23];
	C[4] = A[ 4] ^ A[ 9] ^ A[14] ^ A[19] ^ A[24];

	for (i = 0; i < 24; i++) {
		/* Theta setup */
		D[0] = C[4] ^ ROTL64(C[1], 1);
		D[1] = C[0] ^ ROTL64(C[2], 1);
		D[2] = C[1] ^ ROTL64(C[3], 1);
		D[3] = C[2] ^ ROTL64(C[4], 1);
		D[4] = C[3] ^ ROTL64(C[0], 1);


		/* Combined Theta, Rho and Pi */

		A[0] ^= D[0];
		X = A[ 1] ^ D[1];     T = ROTL64(X,  1);
		X = A[ 6] ^ D[1]; A[ 1] = ROTL64(X, 44);
		X = A[ 9] ^ D[4]; A[ 6] = ROTL64(X, 20);
		X = A[22] ^ D[2]; A[ 9] = ROTL64(X, 61);
		X = A[14] ^ D[4]; A[22] = ROTL64(X, 39);
		X = A[20] ^ D[0]; A[14] = ROTL64(X, 18);
		X = A[ 2] ^ D[2]; A[20] = ROTL64(X, 62);
		X = A[12] ^ D[2]; A[ 2] = ROTL64(X, 43);
		X = A[13] ^ D[3]; A[12] = ROTL64(X, 25);
		X = A[19] ^ D[4]; A[13] = ROTL64(X,  8);
		X = A[23] ^ D[3]; A[19] = ROTL64(X, 56);
		X = A[15] ^ D[0]; A[23] = ROTL64(X, 41);
		X = A[ 4] ^ D[4]; A[15] = ROTL64(X, 27);
		X = A[24] ^ D[4]; A[ 4] = ROTL64(X, 14);
		X = A[21] ^ D[1]; A[24] = ROTL64(X,  2);
		X = A[ 8] ^ D[3]; A[21] = ROTL64(X, 55); /* row 4 done */
		X = A[16] ^ D[1]; A[ 8] = ROTL64(X, 45);
		X = A[ 5] ^ D[0]; A[16] = ROTL64(X, 36);
		X = A[ 3] ^ D[3]; A[ 5] = ROTL64(X, 28);
		X = A[18] ^ D[3]; A[ 3] = ROTL64(X, 21); /* row 0 done */
		X = A[17] ^ D[2]; A[18] = ROTL64(X, 15);
		X = A[11] ^ D[1]; A[17] = ROTL64(X, 10); /* row 3 done */
		X = A[ 7] ^ D[2]; A[11] = ROTL64(X,  6); /* row 1 done */
		X = A[10] ^ D[0]; A[ 7] = ROTL64(X,  3);
		A[10] = T;				/* row 2 done */


		/* Combined Chi and Iota */

		D[0] = ~A[1] & A[2];
		D[1] = ~A[2] & A[3];
		D[2] = ~A[3] & A[4];
		D[3] = ~A[4] & A[0];
		D[4] = ~A[0] & A[1];

		A[0] ^= D[0] ^ rc[i];	C[0] = A[0];
		A[1] ^= D[1];			C[1] = A[1];
		A[2] ^= D[2];			C[2] = A[2];
		A[3] ^= D[3];			C[3] = A[3];
		A[4] ^= D[4];			C[4] = A[4];

		for (y = 5; y < 25; y+= 5) {
			D[0] = ~A[y+1] & A[y+2];
			D[1] = ~A[y+2] & A[y+3];
			D[2] = ~A[y+3] & A[y+4];
			D[3] = ~A[y+4] & A[y+0];
			D[4] = ~A[y+0] & A[y+1];

			A[y+0] ^= D[0]; C[0] ^= A[y+0];
			A[y+1] ^= D[1]; C[1] ^= A[y+1];
			A[y+2] ^= D[2]; C[2] ^= A[y+2];
			A[y+3] ^= D[3]; C[3] ^= A[y+3];
			A[y+4] ^= D[4]; C[4] ^= A[y+4];
		}
	}
}

#if __IS_x86__ && CRYPTO_FAT

static void
_crypto_sha3_permute_default(uint64_t *A) {
	if (is_x86_64() && is_avx()) {
		crypto_sha3_permute_fat = &sha3_permute_avx;
	} else {
		crypto_sha3_permute_fat = &sha3_permute_gen;
	}
	sha3_permute(A);
}

void sha3_permute(uint64_t *) = &_crypto_sha3_permute_default;

#endif

