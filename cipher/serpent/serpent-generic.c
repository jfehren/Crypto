/*
 * serpent-generic.c
 *
 *  Created on: Feb 25, 2019, 9:52:43 AM
 *      Author: Joshua Fehrenbach
 */

#include "serpent-internal.h"
#include "macros.h"

#define ADDKEY(x0,x1,x2,x3, subkey) \
do {						\
	(x0) ^= (subkey)[0];	\
	(x1) ^= (subkey)[1];	\
	(x2) ^= (subkey)[2];	\
	(x3) ^= (subkey)[3];	\
} while (0)

/* S0:  3  8 15  1 10  6  5 11 14 13  4  2  7  0  9 12 */
/* Could easily let y0, y1 overlap with x0, x1, and possibly also x2 and y2 */
#define SBOX0(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y3 = x1 ^ x2;				\
    y0 = x0 | x3;				\
    y1 = x0 ^ x1;				\
    y3 = y3 ^ y0;				\
    y2 = x2 | y3;				\
    x0 = x0 ^ x3;				\
    y2 = y2 & x3;				\
    x3 = x3 ^ x2;				\
    x2 = x2 | x1;				\
    y0 = y1 & x2;				\
    y2 = y2 ^ y0;				\
    y0 = y0 & y2;				\
    y0 = y0 ^ x2;				\
    x1 = x1 & x0;				\
    y0 = y0 ^ x0;				\
    y0 = ~ y0;					\
    y1 = y0 ^ x1;				\
    y1 = y1 ^ x3;				\
  } while (0)

/* S1: 15 12  2  7  9  0  5 10  1 11 14  8  6 13  3  4 */
#define SBOX1(x0, x1, x2, x3, y0, y1, y2, y3)		\
  do {							\
    y1 = x0 | x3;				\
    y2 = x2 ^ x3;				\
    y0 = ~ x1;					\
    y3 = x0 ^ x2;				\
    y0 = y0 | x0;				\
    y3 = y3 & x3;				\
    x0 = y1 & y2;				\
    y3 = y3 | x1;				\
    y2 = y2 ^ y0;				\
    y3 = y3 ^ x0;				\
    x0 = y1 ^ y3;				\
    x0 = x0 ^ y2;				\
    y1 = x1 & x3;				\
    y1 = y1 ^ x0;				\
    x3 = y1 | y3;				\
    y3 = ~ y3;					\
    y0 = y0 & x3;				\
    y0 = y0 ^ x2;				\
  } while (0)

/* S2:  8  6  7  9  3 12 10 15 13  1 14  4  0 11  5  2 */
#define SBOX2(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y1 = x0 ^ x1;				\
    y2 = x0 | x2;				\
    y3 = x3 ^ y2;				\
    y0 = y1 ^ y3;				\
    x3 = x3 | x0;				\
    x2 = x2 ^ y0;				\
    x0 = x1 ^ x2;				\
    x2 = x2 | x1;				\
    x0 = x0 & y2;				\
    y3 = y3 ^ x2;				\
    y1 = y1 | y3;				\
    y1 = y1 ^ x0;				\
    y2 = y3 ^ y1;				\
    y2 = y2 ^ x1;				\
    y3 = ~ y3;					\
    y2 = y2 ^ x3;				\
  } while (0)

/* S3:  0 15 11  8 12  9  6  3 13  1  2  4 10  7  5 14 */
#define SBOX3(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y1 = x0 ^ x2;				\
    y0 = x0 | x3;				\
    y3 = x0 & x3;				\
    y2 = x0 & x1;				\
    y1 = y1 & y0;				\
    y2 = y2 | x2;				\
    y3 = y3 | x1;				\
    x2 = x3 ^ y1;				\
    y1 = y1 ^ y3;				\
    x0 = x0 | x2;				\
    x2 = x2 ^ x1;				\
    y3 = y3 & x3;				\
    y0 = y0 ^ y3;				\
    y3 = y2 ^ x2;				\
    y2 = y2 ^ y0;				\
    x3 = x3 | y3;				\
    x1 = x1 & x3;				\
    y0 = x0 ^ x1;				\
  } while (0)

/* S4:  1 15  8  3 12  0 11  6  2  5  4 10  9 14  7 13 */
#define SBOX4(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y3 = x0 | x1;				\
    y2 = x1 | x2;				\
    y3 = y3 & x3;				\
    y2 = y2 ^ x0;				\
    y0 = x1 ^ x3;				\
    x3 = x3 | y2;				\
    x1 = x1 & x2;				\
    x2 = x2 ^ y3;				\
    y3 = y3 ^ y2;				\
    x0 = x0 & x3;				\
    y2 = y2 | x1;				\
    y1 = y3 & y0;				\
    y2 = y2 ^ y1;				\
    y1 = y1 ^ y0;				\
    y0 = y0 & x3;				\
    y1 = y1 | x1;				\
    y0 = y0 ^ x2;				\
    y1 = y1 ^ x0;				\
    y0 = ~ y0;					\
  } while (0)

/* S5: 15  5  2 11  4 10  9 12  0  3 14  8 13  6  7  1 */
#define SBOX5(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y0 = x1 | x3;				\
    y0 = y0 ^ x2;				\
    x2 = x1 ^ x3;				\
    y2 = x0 ^ x2;				\
    x0 = x0 & x2;				\
    y0 = y0 ^ x0;				\
    y3 = x1 | y2;				\
    x1 = x1 | y0;				\
    y0 = ~ y0;					\
    y3 = y3 ^ x2;				\
    x0 = x0 | y0;				\
    y3 = y3 ^ x0;				\
    y1 = x3 | y0;				\
    x3 = x3 ^ y1;				\
    y1 = y1 ^ y2;				\
    y2 = y2 | x3;				\
    y2 = y2 ^ x1;				\
  } while (0)

/* S6:  7  2 12  5  8  4  6 11 14  9  1 15 13  3 10  0 */
#define SBOX6(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y0 = x0 ^ x3;				\
    y1 = x0 & x3;				\
    y2 = x0 | x2;				\
    x3 = x3 | x1;				\
    x0 = x0 ^ x1;				\
    x3 = x3 ^ x2;				\
    y3 = x1 | x2;				\
    x2 = x2 ^ x1;				\
    y3 = y3 & y0;				\
    y1 = y1 ^ x2;				\
    y1 = ~ y1;					\
    x1 = x1 & y1;				\
    y0 = y0 & y1;				\
    x1 = x1 ^ y3;				\
    y3 = y3 ^ x3;				\
    y2 = y2 ^ x1;				\
    y2 = ~ y2;					\
    y0 = y0 ^ x0;				\
    y0 = y0 ^ y2;				\
  } while (0)

/* S7:  1 13 15  0 14  8  2 11  7  4 12 10  9  3  5  6 */
/* It appears impossible to do this with only 8 registers. We
   recompute t02, and t04 (if we have spare registers, hopefully the
   compiler can recognize them as common subexpressions). */
#define SBOX7(x0, x1, x2, x3, y0, y1, y2, y3)	\
  do {							\
    y0 = x0 & x2;				\
    y3 = x1 | y0; /* t04 */		\
    y1 = ~ x3;    /* t02 */		\
    y3 = y3 ^ x2;				\
    y1 = y1 & x0;				\
    y3 = y3 ^ y1;				\
    y1 = x2 | y3;				\
    y2 = x0 & x1;				\
    y1 = y1 ^ x0;				\
    x2 = x2 ^ y2;				\
    y2 = y2 | x3;				\
    y1 = y1 ^ y2;				\
    x3 = ~ x3;    /* t02 */		\
    y2 = x1 | y0; /* t04 */		\
    y0 = y0 ^ y1;				\
    x1 = x1 ^ y1;				\
    y2 = y2 & y3;				\
    y0 = y0 | x3;				\
    y2 = y2 | x1;				\
    y0 = y0 ^ x2;				\
    y2 = y2 ^ x0;				\
  } while (0)

/* In-place linear transformation.  */
#define LT(x0,x1,x2,x3)			\
  do {							\
    x0 = ROTL32 (13, x0);		\
    x2 = ROTL32 (3, x2);		\
    x1 = x1 ^ x0 ^ x2;			\
    x3 = x3 ^ x2 ^ (x0 << 3);	\
    x1 = ROTL32 (1, x1);		\
    x3 = ROTL32 (7, x3);		\
    x0 = x0 ^ x1 ^ x3;			\
    x2 = x2 ^ x3 ^ (x1 << 7);	\
    x0 = ROTL32 (5, x0);		\
    x2 = ROTL32 (22, x2);		\
  } while (0)

/* Round inputs are x0,x1,x2,x3 (destroyed), and round outputs are
 y0,y1,y2,y3. */
#define ENCRYPT(which, subkey, x0,x1,x2,x3, y0,y1,y2,y3) \
do {						       \
  ADDKEY(x0,x1,x2,x3, subkey);		       \
  SBOX##which(x0,x1,x2,x3, y0,y1,y2,y3);	       \
  LT(y0,y1,y2,y3);		       \
} while (0)

void
_serpent_encrypt_gen(const uint32_t (*subkeys)[4], uint8_t *dst,
		const uint8_t *src, size_t length) {
	register x0,x1,x2,x3, y0,y1,y2,y3;
	while (length >= 16) {
		x0 = LE_READ_UINT32(src +  0);
		x1 = LE_READ_UINT32(src +  4);
		x2 = LE_READ_UINT32(src +  8);
		x3 = LE_READ_UINT32(src + 12);
		ENCRYPT(0, subkeys[ 0], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(1, subkeys[ 1], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(2, subkeys[ 2], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(3, subkeys[ 3], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(4, subkeys[ 4], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(5, subkeys[ 5], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(6, subkeys[ 6], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(7, subkeys[ 7], y0,y1,y2,y3, x0,x1,x2,x3);

		ENCRYPT(0, subkeys[ 8], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(1, subkeys[ 9], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(2, subkeys[10], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(3, subkeys[11], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(4, subkeys[12], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(5, subkeys[13], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(6, subkeys[14], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(7, subkeys[15], y0,y1,y2,y3, x0,x1,x2,x3);

		ENCRYPT(0, subkeys[16], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(1, subkeys[17], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(2, subkeys[18], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(3, subkeys[19], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(4, subkeys[20], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(5, subkeys[21], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(6, subkeys[22], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(7, subkeys[23], y0,y1,y2,y3, x0,x1,x2,x3);

		ENCRYPT(0, subkeys[24], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(1, subkeys[25], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(2, subkeys[26], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(3, subkeys[27], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(4, subkeys[28], x0,x1,x2,x3, y0,y1,y2,y3);
		ENCRYPT(5, subkeys[29], y0,y1,y2,y3, x0,x1,x2,x3);
		ENCRYPT(6, subkeys[30], x0,x1,x2,x3, y0,y1,y2,y3);
		ADDKEY(y0,y1,y2,y3, subkeys[31]); SBOX7(y0,y1,y2,y3, x0,x1,x2,x3);
		ADDKEY(x0,x1,x2,x3, subkeys[32]);
		LE_WRITE_UINT32(dst +  0, x0);
		LE_WRITE_UINT32(dst +  4, x1);
		LE_WRITE_UINT32(dst +  8, x2);
		LE_WRITE_UINT32(dst + 12, x3);
		src += 16;
		dst += 16;
		length -= 16;
	}
}


/* S0 inverse:  13  3 11  0 10  6  5 12  1 14  4  7 15  9  8  2 */
#define IBOX0(x0, x1, x2, x3, y0, y1, y2, y3)		\
  do {								\
    y1 = x2 ^ x3;					\
    y2 = x0 | x1;					\
    y0 = x0 ^ x2;					\
    y2 = y2 ^ y1;					\
    y1 = y1 & x2;					\
    x2 = x2 | x1;					\
    x1 = x1 ^ x3;					\
    y1 = y1 | x0;					\
    x1 = x1 & x2;					\
    x0 = x0 | y2;					\
    y1 = y1 ^ x1;					\
    x1 = y2;						\
    x0 = x0 ^ y1;					\
    y2 = ~ y2;						\
    x1 = x1 & x0;					\
    x3 = x3 | y2;					\
    x3 = x3 ^ x2;					\
    x1 = x1 | x3;					\
    y3 = x3 ^ x0;					\
    y0 = y0 ^ x1;					\
  } while (0)

/* S1 inverse:   5  8  2 14 15  6 12  3 11  4  7  9  1 13 10  0 */
#define IBOX1(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {							    \
    y1 = x1 | x3;				    \
    y3 = x0 ^ x1;				    \
    y1 = y1 ^ x2;				    \
    y0 = x0 | y1;				    \
    y0 = y0 & y3;				    \
    y3 = y3 ^ y1;				    \
    x1 = x1 ^ y0;				    \
    y2 = x0 & x2;				    \
    x1 = x1 & x3;				    \
    y1 = y1 | y2;				    \
    y2 = y2 | x3;				    \
    y2 = y2 ^ y0;				    \
    y1 = y1 ^ x1;				    \
    y2 = ~ y2;					    \
    y0 = y0 ^ x2;				    \
    x0 = x0 | y2;				    \
    y0 = y0 ^ y1;				    \
    y0 = y0 ^ x0;				    \
  } while (0)

/* S2 inverse:  12  9 15  4 11 14  1  2  0  3  6 13  5  8 10  7 */
#define IBOX2(x0, x1, x2, x3, y0, y1, y2, y3)		\
  do {								\
    y2 = x2 ^ x3;					\
    y0 = x0 ^ x3;					\
    y1 = x1 | y2;					\
    y0 = y0 ^ y1;					\
    y1 = x3 | y0;					\
    y1 = y1 & x1;					\
    x3 = ~ x3;						\
    y3 = x0 | x2;					\
    y2 = y2 & y3;					\
    y3 = y3 & x1;					\
    x0 = x0 & x2;					\
    y1 = y1 ^ y2;					\
    x0 = x0 | x3;					\
    y3 = y3 ^ x0;					\
    x2 = x2 & y3;					\
    x2 = x2 ^ x0;					\
    y2 = y0 ^ y1;					\
    y2 = y2 ^ x2;					\
  } while (0)

/* S3 inverse:   0  9 10  7 11 14  6 13  3  5 12  2  4  8 15  1 */
#define IBOX3(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {							    \
    y3 = x2 | x3;				    \
    y2 = x0 | x3;				    \
    y0 = x1 & y3;				    \
    y1 = x2 ^ y2;				    \
    x3 = x3 ^ x0;				    \
    y0 = y0 ^ y1;				    \
    y2 = y2 ^ x1;				    \
    y3 = y3 ^ x3;				    \
    y2 = y2 & y1;				    \
    y1 = y1 ^ x0;				    \
    y2 = y2 ^ x3;				    \
    x3 = x3 | y0;				    \
    x0 = x0 & y2;				    \
    y1 = y1 & x3;				    \
    x0 = x0 | x1;				    \
    y1 = y1 ^ x1;				    \
    y3 = y3 ^ x0;				    \
  } while (0)

/* S4 inverse:   5  0  8  3 10  9  7 14  2 12 11  6  4 15 13  1 */
#define IBOX4(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {							    \
    y2 = x2 | x3;				    \
    y1 = x2 ^ x3;				    \
    y2 = y2 ^ x1;				    \
    x1 = x1 | x3;				    \
    y0 = x0 ^ y2;				    \
    x3 = x3 ^ y2;				    \
    y2 = y2 & x0;				    \
    y1 = y1 ^ y2;				    \
    y2 = y2 ^ x0;				    \
    x0 = x0 & x1;				    \
    y2 = y2 | x2;				    \
    y3 = x3 ^ x0;				    \
    x0 = ~ x0;					    \
    x0 = x0 | y1;				    \
    y2 = y2 ^ x1;				    \
    y0 = y0 ^ x0;				    \
    y2 = y2 ^ x0;				    \
  } while (0)

/* S5 inverse:   8 15  2  9  4  1 13 14 11  6  5  3  7 12 10  0 */
#define IBOX5(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {								\
    y1 = x0 & x3;					\
    y3 = x2 ^ y1;					\
    y0 = x1 & y3;					\
    y2 = x0 ^ x3;					\
    x3 = x3 ^ x1;					\
    y0 = y0 ^ y2;					\
    x2 = x2 & x0;					\
    x0 = x0 & y0;					\
    y1 = y1 ^ y0;					\
    x2 = x2 | x1;					\
    y1 = y1 ^ x2;					\
    x1 = ~ x1;						\
    y2 = y0 | y1;					\
    x1 = x1 | x0;					\
    y2 = y2 ^ y3;					\
    y3 = y3 ^ x1;					\
    y2 = y2 ^ x3;					\
  } while (0)

/* S6 inverse:  15 10  1 13  5  3  6  0  4  9 14  7  2 12  8 11 */
#define IBOX6(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {							    \
    y2 = x0 ^ x2;				    \
    y0 = x1 ^ x3;				    \
    x2 = ~ x2;					    \
    y1 = x0 | x2;				    \
    y3 = x1 & y2;				    \
    y1 = y1 ^ y0;				    \
    y3 = y3 | x3;				    \
    x3 = x3 | x2;				    \
    x2 = x2 | x1;				    \
    x2 = x2 & x0;				    \
    y0 = x2 ^ y3;				    \
    y3 = y3 & y2;				    \
    y0 = ~ y0;					    \
    y3 = y3 ^ x2;				    \
    x0 = x0 ^ y1;				    \
    x1 = x1 & y0;				    \
    y2 = y2 ^ x3;				    \
    y3 = y3 ^ x0;				    \
    y2 = y2 ^ x1;				    \
  } while (0)

/* S7 inverse:   3  0  6 13  9 14 15  8  5 12 11  7 10  1  4  2 */
#define IBOX7(x0, x1, x2, x3, y0, y1, y2, y3)	    \
  do {							    \
    y3 = x0 & x1;				    \
    y2 = x1 ^ x3;				    \
    y1 = x0 | x3;				    \
    y2 = y2 | y3;				    \
    y1 = y1 & x2;				    \
    y0 = x0 | x1;				    \
    y3 = y3 | x2;				    \
    y2 = y2 ^ y1;				    \
    y3 = y3 ^ y0;				    \
    y0 = y0 & x3;				    \
    y1 = x3 ^ y3;				    \
    y0 = y0 ^ x1;				    \
    y1 = ~ y1;					    \
    y1 = y1 | y0;				    \
    y0 = y0 ^ x2;				    \
    x3 = x3 | y1;				    \
    y1 = y1 ^ x0;				    \
    y0 = y0 ^ x3;				    \
  } while (0)

/* In-place inverse linear transformation.  */
#define ILT(x0,x1,x2,x3)	 \
  do {                                                   \
    x2 = ROTL32 (10, x2);                    \
    x0 = ROTL32 (27, x0);                    \
    x2 = x2 ^ x3 ^ (x1 << 7); \
    x0 = x0 ^ x1 ^ x3;        \
    x3 = ROTL32 (25, x3);                     \
    x1 = ROTL32 (31, x1);                     \
    x3 = x3 ^ x2 ^ (x0 << 3); \
    x1 = x1 ^ x0 ^ x2;        \
    x2 = ROTL32 (29, x2);                     \
    x0 = ROTL32 (19, x0);                    \
  } while (0)

/* Round inputs are x0,x1,x2,x3 (destroyed), and round outputs are
   y0,y1,y2,y3. */
#define DECRYPT(which, subkey, x0,x1,x2,x3, y0,y1,y2,y3) \
  do {							       \
    ILT(x0,x1,x2,x3);	       \
    IBOX##which(x0,x1,x2,x3, y0,y1,y2,y3);	       \
    ADDKEY(y0,y1,y2,y3, subkey);			       \
  } while (0)

void
_serpent_decrypt_gen(const uint32_t (*subkeys)[4], uint8_t *dst,
		const uint8_t *src, size_t length) {
	register x0,x1,x2,x3, y0,y1,y2,y3;
	while (length >= 16) {
		x0 = LE_READ_UINT32(src +  0);
		x1 = LE_READ_UINT32(src +  4);
		x2 = LE_READ_UINT32(src +  8);
		x3 = LE_READ_UINT32(src + 12);
		ADDKEY(x0,x1,x2,x3, subkeys[32]);
		IBOX7(x0,x1,x2,x3, y0,y1,y2,y3); ADDKEY(y0,y1,y2,y3, subkeys[31]);
		DECRYPT(6, subkeys[30], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(5, subkeys[29], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(4, subkeys[28], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(3, subkeys[27], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(2, subkeys[26], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(1, subkeys[25], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(0, subkeys[24], y0,y1,y2,y3, x0,x1,x2,x3);

		DECRYPT(7, subkeys[23], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(6, subkeys[22], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(5, subkeys[21], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(4, subkeys[20], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(3, subkeys[19], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(2, subkeys[18], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(1, subkeys[17], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(0, subkeys[16], y0,y1,y2,y3, x0,x1,x2,x3);

		DECRYPT(7, subkeys[15], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(6, subkeys[14], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(5, subkeys[13], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(4, subkeys[12], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(3, subkeys[11], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(2, subkeys[10], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(1, subkeys[ 9], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(0, subkeys[ 8], y0,y1,y2,y3, x0,x1,x2,x3);

		DECRYPT(7, subkeys[ 7], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(6, subkeys[ 6], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(5, subkeys[ 5], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(4, subkeys[ 4], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(3, subkeys[ 3], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(2, subkeys[ 2], y0,y1,y2,y3, x0,x1,x2,x3);
		DECRYPT(1, subkeys[ 1], x0,x1,x2,x3, y0,y1,y2,y3);
		DECRYPT(0, subkeys[ 0], y0,y1,y2,y3, x0,x1,x2,x3);
		LE_WRITE_UINT32(dst +  0, x0);
		LE_WRITE_UINT32(dst +  4, x1);
		LE_WRITE_UINT32(dst +  8, x2);
		LE_WRITE_UINT32(dst + 12, x3);
		src += 16;
		dst += 16;
		length -= 16;
	}
}




