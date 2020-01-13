/*
 * sha512-internal.c
 *
 *  Created on: Apr 28, 2019, 3:01:53 PM
 *      Author: Joshua Fehrenbach
 */

#include "sha512-internal.h"
#include "crypto-internal.h"
#include "macros.h"


/* The SHA512 functions. The Choice function is the same as the SHA1
   function f1, and the majority function is the same as the SHA1 f3
   function, and the same as for SHA256. */

/* #define Choice(x,y,z) ( ( (x) & (y) ) | ( ~(x) & (z) ) ) */
#define Choice(x,y,z)   ( (z) ^ ( (x) & ( (y) ^ (z) ) ) )
/* #define Majority(x,y,z) ( ((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)) ) */
#define Majority(x,y,z) ( ((x) & (y)) ^ ((z) & ((x) ^ (y))) )

#define S0(x) (ROTL64((x),36) ^ ROTL64((x),30) ^ ROTL64((x),25))
#define S1(x) (ROTL64((x),50) ^ ROTL64((x),46) ^ ROTL64((x),23))

#define s0(x) (ROTL64((x),63) ^ ROTL64((x),56) ^ ((x) >> 7))
#define s1(x) (ROTL64((x),45) ^ ROTL64((x), 3) ^ ((x) >> 6))

/* The initial expanding function. The hash function is defined over
   an 64-word expanded input array W, where the first 16 are copies of
   the input data, and the remaining 64 are defined by

        W[ t ] = s1(W[t-2]) + W[t-7] + s0(W[i-15]) + W[i-16]

   This implementation generates these values on the fly in a circular
   buffer.
*/

#define EXPAND(W,i) \
	( W[(i) & 15 ] += (s1(W[((i)-2) & 15]) + W[((i)-7) & 15] + s0(W[((i)-15) & 15])) )

/* The prototype SHA sub-round.  The fundamental sub-round is:

	T1 = h + S1(e) + Choice(e,f,g) + K[t] + W[t]
	T2 = S0(a) + Majority(a,b,c)
	a' = T1+T2
	b' = a
	c' = b
	d' = c
	e' = d + T1
	f' = e
	g' = f
	h' = g

   but this is implemented by unrolling the loop 8 times and renaming
   the variables
   ( h, a, b, c, d, e, f, g ) = ( a, b, c, d, e, f, g, h ) each
   iteration. This code is then replicated 8, using the next 8 values
   from the W[] array each time */

/* It's crucial that DATA is only used once, as that argument will
 * have side effects. */
#define ROUND(a,b,c,d,e,f,g,h,k,data) do { \
	h += S1(e) + Choice(e,f,g) + k + data; \
	d += h; \
	h += S0(a) + Majority(a,b,c); \
} while (0)

void
_sha512_compress_gen(uint64_t *state, const uint8_t *input,
		size_t length, const uint64_t *K) {
	uint64_t data[16];
	register uint64_t A, B, C, D, E, F, G, H;     /* Local vars */
	unsigned i;

	FOR_BLOCKS_INPLACE(length, input, 128) {
		/* Read Input */
		for (i = 0; i < 16; i++) {
			data[i] = BE_READ_UINT64(input + 8*i);
		}

		/* Set up first buffer and local data buffer */
		A = state[0];
		B = state[1];
		C = state[2];
		D = state[3];
		E = state[4];
		F = state[5];
		G = state[6];
		H = state[7];

		/* First 16 subrounds act on the original data */
		ROUND(A, B, C, D, E, F, G, H, K[ 0], data[ 0]);
		ROUND(H, A, B, C, D, E, F, G, K[ 1], data[ 1]);
		ROUND(G, H, A, B, C, D, E, F, K[ 2], data[ 2]);
		ROUND(F, G, H, A, B, C, D, E, K[ 3], data[ 3]);
		ROUND(E, F, G, H, A, B, C, D, K[ 4], data[ 4]);
		ROUND(D, E, F, G, H, A, B, C, K[ 5], data[ 5]);
		ROUND(C, D, E, F, G, H, A, B, K[ 6], data[ 6]);
		ROUND(B, C, D, E, F, G, H, A, K[ 7], data[ 7]);

		ROUND(A, B, C, D, E, F, G, H, K[ 8], data[ 8]);
		ROUND(H, A, B, C, D, E, F, G, K[ 9], data[ 9]);
		ROUND(G, H, A, B, C, D, E, F, K[10], data[10]);
		ROUND(F, G, H, A, B, C, D, E, K[11], data[11]);
		ROUND(E, F, G, H, A, B, C, D, K[12], data[12]);
		ROUND(D, E, F, G, H, A, B, C, K[13], data[13]);
		ROUND(C, D, E, F, G, H, A, B, K[14], data[14]);
		ROUND(B, C, D, E, F, G, H, A, K[15], data[15]);

		ROUND(A, B, C, D, E, F, G, H, K[16], EXPAND(data, 16));
		ROUND(H, A, B, C, D, E, F, G, K[17], EXPAND(data, 17));
		ROUND(G, H, A, B, C, D, E, F, K[18], EXPAND(data, 18));
		ROUND(F, G, H, A, B, C, D, E, K[19], EXPAND(data, 19));
		ROUND(E, F, G, H, A, B, C, D, K[20], EXPAND(data, 20));
		ROUND(D, E, F, G, H, A, B, C, K[21], EXPAND(data, 21));
		ROUND(C, D, E, F, G, H, A, B, K[22], EXPAND(data, 22));
		ROUND(B, C, D, E, F, G, H, A, K[23], EXPAND(data, 23));

		ROUND(A, B, C, D, E, F, G, H, K[24], EXPAND(data, 24));
		ROUND(H, A, B, C, D, E, F, G, K[25], EXPAND(data, 25));
		ROUND(G, H, A, B, C, D, E, F, K[26], EXPAND(data, 26));
		ROUND(F, G, H, A, B, C, D, E, K[27], EXPAND(data, 27));
		ROUND(E, F, G, H, A, B, C, D, K[28], EXPAND(data, 28));
		ROUND(D, E, F, G, H, A, B, C, K[29], EXPAND(data, 29));
		ROUND(C, D, E, F, G, H, A, B, K[30], EXPAND(data, 30));
		ROUND(B, C, D, E, F, G, H, A, K[31], EXPAND(data, 31));

		ROUND(A, B, C, D, E, F, G, H, K[32], EXPAND(data, 32));
		ROUND(H, A, B, C, D, E, F, G, K[33], EXPAND(data, 33));
		ROUND(G, H, A, B, C, D, E, F, K[34], EXPAND(data, 34));
		ROUND(F, G, H, A, B, C, D, E, K[35], EXPAND(data, 35));
		ROUND(E, F, G, H, A, B, C, D, K[36], EXPAND(data, 36));
		ROUND(D, E, F, G, H, A, B, C, K[37], EXPAND(data, 37));
		ROUND(C, D, E, F, G, H, A, B, K[38], EXPAND(data, 38));
		ROUND(B, C, D, E, F, G, H, A, K[39], EXPAND(data, 39));

		ROUND(A, B, C, D, E, F, G, H, K[40], EXPAND(data, 40));
		ROUND(H, A, B, C, D, E, F, G, K[41], EXPAND(data, 41));
		ROUND(G, H, A, B, C, D, E, F, K[42], EXPAND(data, 42));
		ROUND(F, G, H, A, B, C, D, E, K[43], EXPAND(data, 43));
		ROUND(E, F, G, H, A, B, C, D, K[44], EXPAND(data, 44));
		ROUND(D, E, F, G, H, A, B, C, K[45], EXPAND(data, 45));
		ROUND(C, D, E, F, G, H, A, B, K[46], EXPAND(data, 46));
		ROUND(B, C, D, E, F, G, H, A, K[47], EXPAND(data, 47));

		ROUND(A, B, C, D, E, F, G, H, K[48], EXPAND(data, 48));
		ROUND(H, A, B, C, D, E, F, G, K[49], EXPAND(data, 49));
		ROUND(G, H, A, B, C, D, E, F, K[50], EXPAND(data, 50));
		ROUND(F, G, H, A, B, C, D, E, K[51], EXPAND(data, 51));
		ROUND(E, F, G, H, A, B, C, D, K[52], EXPAND(data, 52));
		ROUND(D, E, F, G, H, A, B, C, K[53], EXPAND(data, 53));
		ROUND(C, D, E, F, G, H, A, B, K[54], EXPAND(data, 54));
		ROUND(B, C, D, E, F, G, H, A, K[55], EXPAND(data, 55));

		ROUND(A, B, C, D, E, F, G, H, K[56], EXPAND(data, 56));
		ROUND(H, A, B, C, D, E, F, G, K[57], EXPAND(data, 57));
		ROUND(G, H, A, B, C, D, E, F, K[58], EXPAND(data, 58));
		ROUND(F, G, H, A, B, C, D, E, K[59], EXPAND(data, 59));
		ROUND(E, F, G, H, A, B, C, D, K[60], EXPAND(data, 60));
		ROUND(D, E, F, G, H, A, B, C, K[61], EXPAND(data, 61));
		ROUND(C, D, E, F, G, H, A, B, K[62], EXPAND(data, 62));
		ROUND(B, C, D, E, F, G, H, A, K[63], EXPAND(data, 63));

		/* Update state */
		state[0] += A;
		state[1] += B;
		state[2] += C;
		state[3] += D;
		state[4] += E;
		state[5] += F;
		state[6] += G;
		state[7] += H;
	}
}


#if __IS_x86__ && CRYPTO_FAT

static void
_crypto_sha512_compress_default(uint64_t* state, const uint8_t* data,
		size_t length, const uint64_t *K) {
	if (is_x86_64()) {
		_crypto_sha512_compress_fat = &_sha512_compress_x64;
	} else {
		_crypto_sha512_compress_fat = &_sha512_compress_gen;
	}
	_sha512_compress(state, data, length, K);
}

void _sha512_compress(uint64_t*, const uint8_t*, size_t, const uint64_t*) =
		&_crypto_sha512_compress_default;

#endif

