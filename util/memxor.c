/*
 * memxor.c
 *
 *  Created on: Mar 6, 2019, 2:19:37 PM
 *      Author: Joshua Fehrenbach
 */

#include "memxor.h"
#include "memxor-internal.h"
#include <assert.h>
#include <limits.h>

static void
memxor_same_alignment(word_t *dst, const word_t *src, size_t n) {
	if (n & 1) {
		n--;
		dst[n] ^= src[n];
	}
	if (n & 2) {
		n -= 2;
		dst[n+1] ^= src[n+1];
		dst[n+0] ^= src[n+0];
	}
	while (n >= 4) {
		n -= 4;
		dst[n+3] ^= src[n+3];
		dst[n+2] ^= src[n+2];
		dst[n+1] ^= src[n+1];
		dst[n+0] ^= src[n+0];
	}
}

static void
memxor_different_alignment(word_t *dst, const unsigned char *src, size_t n) {
	unsigned shl, shr;
	unsigned offset;
	const word_t *src_word;
	word_t s0, s1;
	assert (n > 0);

	offset = ALIGN_OFFSET(src);
	shl = CHAR_BIT * offset;
	shr = CHAR_BIT * (sizeof(word_t) - offset);

	src_word = (const word_t*) ((uintptr_t)src & -sizeof(word_t));

	/* Read top partial bytes, in native byte order. */
	READ_PARTIAL(s0, (unsigned char*)&src_word[n], offset);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	s0 <<= shr;
#endif

	/* Do n-1 regular iterations */
	if (n & 1) {
		s1 = s0;
	} else {
		n--;
		s1 = src_word[n];
		dst[n] ^= MERGE(s1, shl, s0, shr);
	}

	assert (n & 1);
	while (n > 2) {
		n -= 2;
		s0 = src_word[n+1];
		dst[n+1] ^= MERGE(s0, shl, s1, shr);
		s1 = src_word[n];
		dst[n] ^= MERGE(s1, shl, s0, shr);
	}
	assert (n == 1);
	/* Read low sizeof(word_t)-offset bytes */
	READ_PARTIAL(s0, src, sizeof(word_t) - offset);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
	s0 <<= shl;
#endif
	dst[0] ^= MERGE(s0, shl, s1, shr);
}

void*
memxor(void *dst_in, const void *src_in, size_t n) {
	unsigned char *dst = dst_in;
	const unsigned char *src = src_in;

	if (n >= 16) {
		unsigned i;
		unsigned offset;
		size_t nwords;
		/* align dst by sizeof(word_t) bytes */
		for (i = ALIGN_OFFSET(dst + n); i > 0; i--) {
			n--;
			dst[n] ^= src[n];
		}
		offset = ALIGN_OFFSET(src + n);
		nwords = n / sizeof(word_t);
		n %= sizeof(word_t);

		if (offset != 0) {
			memxor_different_alignment((word_t*)(dst+n), src+n, nwords);
		} else {
			memxor_same_alignment((word_t*)(dst+n), (const word_t*)(src+n), nwords);
		}
	}
	while (n > 0) {
		n--;
		dst[n] ^= src[n];
	}
	return dst;
}


static void
memxor3_same_alignment(word_t *dst, const word_t *a, const word_t *b, size_t n) {
	if (n & 1) {
		n--;
		dst[n] = a[n] ^ b[n];
	}
	if (n & 2) {
		n -= 2;
		dst[n+1] = a[n+1] ^ b[n+1];
		dst[n+0] = a[n+0] ^ b[n+1];
	}
	while (n >= 4) {
		n -= 4;
		dst[n+3] = a[n+3] ^ b[n+3];
		dst[n+2] = a[n+2] ^ b[n+2];
		dst[n+1] = a[n+1] ^ b[n+1];
		dst[n+0] = a[n+0] ^ b[n+0];
	}
}

static void
memxor3_different_alignment_b(word_t *dst, const word_t *a, const unsigned char *b,
		unsigned offset, size_t n) {
	unsigned shl, shr;
	const word_t *b_word;
	word_t s0, s1;
	assert (n > 0);

	shl = CHAR_BIT * offset;
	shr = CHAR_BIT * (sizeof(word_t) - offset);

	b_word = (const word_t*)((uintptr_t)b & -sizeof(word_t));

	/* Read top offset bytes, in native byte order. */
	READ_PARTIAL (s0, (unsigned char *) &b_word[n], offset);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	s0 <<= shr;
#endif

	if (n & 1) {
		s1 = s0;
	} else {
		n--;
		s1 = b_word[n];
		dst[n] = a[n] ^ MERGE(s1, shl, s0, shr);
	}

	while (n > 2) {
		n -= 2;
		s0 = b_word[n+1];
		dst[n+1] = a[n+1] ^ MERGE(s0, shl, s1, shr);
		s1 = b_word[n];
		dst[n] = a[n] ^ MERGE(s1, shl, s0, shr);
	}
	assert (n == 1);
	/* Read low wordsize - offset bytes */
	READ_PARTIAL (s0, b, sizeof(word_t) - offset);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
	s0 <<= shl;
#endif
	dst[0] = a[0] ^ MERGE(s0, shl, s1, shr);
}

static void
memxor3_different_alignment_ab(word_t *dst, const unsigned char *a, const unsigned char *b,
		unsigned offset, size_t n) {
	unsigned shl, shr;
	const word_t *a_word;
	const word_t *b_word;
	word_t s0, s1, t;
	assert (n > 0);

	shl = CHAR_BIT * offset;
	shr = CHAR_BIT * (sizeof(word_t) - offset);

	a_word = (const word_t*)((uintptr_t)a & -sizeof(word_t));
	b_word = (const word_t*)((uintptr_t)b & -sizeof(word_t));

	/* Read top offset bytes, in native byte order. */
	READ_PARTIAL (s0, (unsigned char*) &a_word[n], offset);
	READ_PARTIAL (t,  (unsigned char*) &b_word[n], offset);
	s0 ^= t;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	s0 <<= shr;
#endif

	if (n & 1) {
		s1 = s0;
	} else {
		n--;
		s1 = a_word[n] ^ b_word[n];
		dst[n] = MERGE(s1, shl, s0, shr);
	}

	while (n > 2) {
		n -= 2;
		s0 = a_word[n+1] ^ b_word[n+1];
		dst[n+1] = MERGE(s0, shl, s1, shr);
		s1 = a_word[n] ^ b_word[n];
		dst[n] = MERGE(s1, shl, s0, shr);
	}
	assert (n == 1);
	/* Read low wordsize - offset bytes */
	READ_PARTIAL (s0, a, sizeof(word_t) - offset);
	READ_PARTIAL (t,  b, sizeof(word_t) - offset);
	s0 ^= t;
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
	s0 <<= shl;
#endif
	dst[0] = MERGE(s0, shl, s1, shr);
}

static void
memxor3_different_alignment_all(word_t *dst, const unsigned char *a, const unsigned char *b,
		unsigned a_offset, unsigned b_offset, size_t n) {
	unsigned al, ar, bl, br;
	const word_t *a_word;
	const word_t *b_word;
	word_t a0, a1, b0, b1;

	al = CHAR_BIT * a_offset;
	ar = CHAR_BIT * (sizeof(word_t) - a_offset);
	bl = CHAR_BIT * b_offset;
	br = CHAR_BIT * (sizeof(word_t) - b_offset);

	a_word = (const word_t*)((uintptr_t)a & -sizeof(word_t));
	b_word = (const word_t*)((uintptr_t)b & -sizeof(word_t));

	/* Read top offset bytes, in native byte order. */
	READ_PARTIAL (a0, (unsigned char*) &a_word[n], a_offset);
	READ_PARTIAL (b0, (unsigned char*) &b_word[n], b_offset);
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	a0 <<= ar;
	b0 <<= br;
#endif

	if (n & 1) {
		a1 = a0;
		b1 = b0;
	} else {
		n--;
		a1 = a_word[n];
		b1 = b_word[n];
		dst[n] = MERGE(a1, al, a0, ar) ^ MERGE(b1, bl, b0, br);
	}

	while (n > 2) {
		n -= 2;
		a0 = a_word[n+1];
		b0 = b_word[n+1];
		dst[n+1] = MERGE(a0, al, a1, ar) ^ MERGE(b0, bl, b1, br);
		a1 = a_word[n];
		b1 = b_word[n];
		dst[n] = MERGE(a1, al, a0, ar) ^ MERGE(b1, bl, b0, br);
	}
	assert (n == 1);
	/* Read low wordsize - offset bytes */
	READ_PARTIAL (a0, a, sizeof(word_t) - a_offset);
	READ_PARTIAL (b0, b, sizeof(word_t) - b_offset);
#if __BYTE_ORDER__ != __ORDER_BIG_ENDIAN__
	a0 <<= al;
	b0 <<= bl;
#endif
	dst[0] = MERGE(a0, al, a1, ar) ^ MERGE(b0, bl, b1, br);
}

void*
memxor3(void *dst_in, const void *a_in, const void *b_in, size_t n) {
	unsigned char *dst = dst_in;
	const unsigned char *a = a_in;
	const unsigned char *b = b_in;

	if (n >= 16) {
		unsigned i;
		unsigned a_offset;
		unsigned b_offset;
		size_t nwords;

		for (i = ALIGN_OFFSET(dst + n); i > 0; i--) {
			n--;
			dst[n] = a[n] ^ b[n];
		}

		a_offset = ALIGN_OFFSET(a + n);
		b_offset = ALIGN_OFFSET(b + n);

		nwords = n / sizeof(word_t);
		n %= sizeof(word_t);

		if (a_offset == b_offset) {
			if (a_offset == 0) {
				memxor3_same_alignment((word_t*)(dst+n),
						(const word_t*)(a+n), (const word_t*)(b+n), nwords);
			} else {
				memxor3_different_alignment_ab((word_t*)(dst+n), a+n, b+n, a_offset, nwords);
			}
		} else if (a_offset == 0) {
			memxor3_different_alignment_b((word_t*)(dst+n), (const word_t*)(a+n), b+n,
					b_offset, nwords);
		} else if (b_offset == 0) {
			memxor3_different_alignment_b((word_t*)(dst+n), (const word_t*)(b+n), a+n,
					a_offset, nwords);
		} else {
			memxor3_different_alignment_all((word_t*)(dst+n), a+n, b+n,
					a_offset, b_offset, nwords);
		}
	}
	while (n > 0) {
		n--;
		dst[n] = a[n] ^ b[n];
	}
	return dst;
}

