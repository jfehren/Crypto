/*
 * sha1-internal.c
 *
 *  Created on: Apr 19, 2019, 10:25:05 AM
 *      Author: Joshua Fehrenbach
 */

#include "sha1-internal.h"
#include "crypto-internal.h"
#include "macros.h"

/* The SHA f()-functions. The f1 and f3 functions can be optimized to
   save one boolean operation each - thanks to Rich Schroeppel,
   rcs@cs.arizona.edu for discovering this */
#define f1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )           /* Rounds  0-19 */
#define f2(x,y,z)   ( x ^ y ^ z )                       /* Rounds 20-39 */
#define f3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )   /* Rounds 40-59 */
#define f4 f2											/* Rounds 60-79 */

/* The SHA Mysterious Constants */
#define K1  0x5A827999L                                 /* Rounds  0-19 */
#define K2  0x6ED9EBA1L                                 /* Rounds 20-39 */
#define K3  0x8F1BBCDCL                                 /* Rounds 40-59 */
#define K4  0xCA62C1D6L                                 /* Rounds 60-79 */

/* The initial expanding function.  The hash function is defined over an
   80-word expanded input array W, where the first 16 are copies of the input
   data, and the remaining 64 are defined by

        W[ i ] = W[ i - 16 ] ^ W[ i - 14 ] ^ W[ i - 8 ] ^ W[ i - 3 ]

   This implementation generates these values on the fly in a circular
   buffer - thanks to Colin Plumb, colin@nyx10.cs.du.edu for this
   optimization.

   The updated SHA changes the expanding function by adding a rotate of 1
   bit.  Thanks to Jim Gillogly, jim@rand.org, and an anonymous contributor
   for this information */

#define expand(W,i) ( W[ i & 15 ] = \
	ROTL32( (W[ i & 15 ] ^ W[ (i - 14) & 15 ] ^ \
			W[ (i - 8) & 15 ] ^ W[ (i - 3) & 15 ]), 1) )

/* The prototype SHA sub-round.  The fundamental sub-round is:

        a' = e + ROTL32( 5, a ) + f( b, c, d ) + k + data;
        b' = a;
        c' = ROTL32( 30, b );
        d' = c;
        e' = d;

   but this is implemented by unrolling the loop 5 times and renaming the
   variables ( e, a, b, c, d ) = ( a', b', c', d', e' ) each iteration.
   This code is then replicated 20 times for each of the 4 functions, using
   the next 20 values from the W[] array each time */

#define subRound(a, b, c, d, e, f, k, data) \
	( e += ROTL32(a, 5) + f(b, c, d) + k + data, b = ROTL32(b, 30) )

void
_sha1_compress_gen(uint32_t* state, const uint8_t* input, size_t length) {
	uint32_t data[16];
	uint32_t A, B, C, D, E;
	int i;

	FOR_BLOCKS_INPLACE(length, input, SHA1_BLOCK_SIZE) {
		/* Read Input */
		for (i = 0; i < 16; i++) {
			data[i] = BE_READ_UINT32(input + 4*i);
		}

		/* Setup first buffer and local data buffer */
		A = state[0];
		B = state[1];
		C = state[2];
		D = state[3];
		E = state[4];

		/* 4 sub-rounds of 20 iterations each */
		subRound(A, B, C, D, E, f1, K1, data[ 0]);
		subRound(E, A, B, C, D, f1, K1, data[ 1]);
		subRound(D, E, A, B, C, f1, K1, data[ 2]);
		subRound(C, D, E, A, B, f1, K1, data[ 3]);
		subRound(B, C, D, E, A, f1, K1, data[ 4]);
		subRound(A, B, C, D, E, f1, K1, data[ 5]);
		subRound(E, A, B, C, D, f1, K1, data[ 6]);
		subRound(D, E, A, B, C, f1, K1, data[ 7]);
		subRound(C, D, E, A, B, f1, K1, data[ 8]);
		subRound(B, C, D, E, A, f1, K1, data[ 9]);
		subRound(A, B, C, D, E, f1, K1, data[10]);
		subRound(E, A, B, C, D, f1, K1, data[11]);
		subRound(D, E, A, B, C, f1, K1, data[12]);
		subRound(C, D, E, A, B, f1, K1, data[13]);
		subRound(B, C, D, E, A, f1, K1, data[14]);
		subRound(A, B, C, D, E, f1, K1, data[15]);
		subRound(E, A, B, C, D, f1, K1, expand(data, 16));
		subRound(D, E, A, B, C, f1, K1, expand(data, 17));
		subRound(C, D, E, A, B, f1, K1, expand(data, 18));
		subRound(B, C, D, E, A, f1, K1, expand(data, 19));

		subRound(A, B, C, D, E, f2, K2, expand(data, 20));
		subRound(E, A, B, C, D, f2, K2, expand(data, 21));
		subRound(D, E, A, B, C, f2, K2, expand(data, 22));
		subRound(C, D, E, A, B, f2, K2, expand(data, 23));
		subRound(B, C, D, E, A, f2, K2, expand(data, 24));
		subRound(A, B, C, D, E, f2, K2, expand(data, 25));
		subRound(E, A, B, C, D, f2, K2, expand(data, 26));
		subRound(D, E, A, B, C, f2, K2, expand(data, 27));
		subRound(C, D, E, A, B, f2, K2, expand(data, 28));
		subRound(B, C, D, E, A, f2, K2, expand(data, 29));
		subRound(A, B, C, D, E, f2, K2, expand(data, 30));
		subRound(E, A, B, C, D, f2, K2, expand(data, 31));
		subRound(D, E, A, B, C, f2, K2, expand(data, 32));
		subRound(C, D, E, A, B, f2, K2, expand(data, 33));
		subRound(B, C, D, E, A, f2, K2, expand(data, 34));
		subRound(A, B, C, D, E, f2, K2, expand(data, 35));
		subRound(E, A, B, C, D, f2, K2, expand(data, 36));
		subRound(D, E, A, B, C, f2, K2, expand(data, 37));
		subRound(C, D, E, A, B, f2, K2, expand(data, 38));
		subRound(B, C, D, E, A, f2, K2, expand(data, 39));

		subRound(A, B, C, D, E, f3, K3, expand(data, 40));
		subRound(E, A, B, C, D, f3, K3, expand(data, 41));
		subRound(D, E, A, B, C, f3, K3, expand(data, 42));
		subRound(C, D, E, A, B, f3, K3, expand(data, 43));
		subRound(B, C, D, E, A, f3, K3, expand(data, 44));
		subRound(A, B, C, D, E, f3, K3, expand(data, 45));
		subRound(E, A, B, C, D, f3, K3, expand(data, 46));
		subRound(D, E, A, B, C, f3, K3, expand(data, 47));
		subRound(C, D, E, A, B, f3, K3, expand(data, 48));
		subRound(B, C, D, E, A, f3, K3, expand(data, 49));
		subRound(A, B, C, D, E, f3, K3, expand(data, 50));
		subRound(E, A, B, C, D, f3, K3, expand(data, 51));
		subRound(D, E, A, B, C, f3, K3, expand(data, 52));
		subRound(C, D, E, A, B, f3, K3, expand(data, 53));
		subRound(B, C, D, E, A, f3, K3, expand(data, 54));
		subRound(A, B, C, D, E, f3, K3, expand(data, 55));
		subRound(E, A, B, C, D, f3, K3, expand(data, 56));
		subRound(D, E, A, B, C, f3, K3, expand(data, 57));
		subRound(C, D, E, A, B, f3, K3, expand(data, 58));
		subRound(B, C, D, E, A, f3, K3, expand(data, 59));

		subRound(A, B, C, D, E, f4, K4, expand(data, 60));
		subRound(E, A, B, C, D, f4, K4, expand(data, 61));
		subRound(D, E, A, B, C, f4, K4, expand(data, 62));
		subRound(C, D, E, A, B, f4, K4, expand(data, 63));
		subRound(B, C, D, E, A, f4, K4, expand(data, 64));
		subRound(A, B, C, D, E, f4, K4, expand(data, 65));
		subRound(E, A, B, C, D, f4, K4, expand(data, 66));
		subRound(D, E, A, B, C, f4, K4, expand(data, 67));
		subRound(C, D, E, A, B, f4, K4, expand(data, 68));
		subRound(B, C, D, E, A, f4, K4, expand(data, 69));
		subRound(A, B, C, D, E, f4, K4, expand(data, 70));
		subRound(E, A, B, C, D, f4, K4, expand(data, 71));
		subRound(D, E, A, B, C, f4, K4, expand(data, 72));
		subRound(C, D, E, A, B, f4, K4, expand(data, 73));
		subRound(B, C, D, E, A, f4, K4, expand(data, 74));
		subRound(A, B, C, D, E, f4, K4, expand(data, 75));
		subRound(E, A, B, C, D, f4, K4, expand(data, 76));
		subRound(D, E, A, B, C, f4, K4, expand(data, 77));
		subRound(C, D, E, A, B, f4, K4, expand(data, 78));
		subRound(B, C, D, E, A, f4, K4, expand(data, 79));

		/* Update message digest */
		state[0] += A;
		state[1] += B;
		state[2] += C;
		state[3] += D;
		state[4] += E;
	}
}

#if __IS_x86__ && CRYPTO_FAT

static void
_crypto_sha1_compress_default(uint32_t* state, const uint8_t* data, size_t length) {
	if (is_x86_64()) {
		if (is_sha()) {
			_crypto_sha1_compress_fat = &_sha1_compress_sha;
		} else {
			_crypto_sha1_compress_fat = &_sha1_compress_x64;
		}
	} else {
		_crypto_sha1_compress_fat = &_sha1_compress_x86;
	}
	_sha1_compress(state, data, length);
}

void _sha1_compress(uint32_t*, const uint8_t*, size_t) = &_crypto_sha1_compress_default;

#endif

