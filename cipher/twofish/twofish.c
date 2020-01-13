/*
 * twofish.c
 *
 *  Created on: Mar 4, 2019, 2:39:03 PM
 *      Author: Joshua Fehrenbach
 */

#include "twofish-internal.h"
#include "macros.h"
#include <assert.h>
#include <string.h>

#define Q0 _twofish_Q0
#define Q1 _twofish_Q1
#define MDS _twofish_MDS
#define RS _twofish_RS

#define B0(x) ((x >>  0) & 0xFF)
#define B1(x) ((x >>  8) & 0xFF)
#define B2(x) ((x >> 16) & 0xFF)
#define B3(x) ((x >> 24) & 0xFF)

#define rol1(x) (((x) << 1) | (((x) & 0x80000000) >> 31))
#define rol8(x) (((x) << 8) | (((x) & 0xFF000000) >> 24))
#define rol9(x) (((x) << 9) | (((x) & 0xFF800000) >> 23))
#define ror1(x) (((x) >> 1) | (((x) & 0x00000001) << 31))

static const uint8_t* const q_table[4][5] = {
	{ Q1, Q1, Q0, Q0, Q1 },
	{ Q0, Q1, Q1, Q0, Q0 },
	{ Q0, Q0, Q0, Q1, Q1 },
	{ Q1, Q0, Q1, Q1, Q0 }
};

static inline uint32_t
h_byte(int k, int i, uint8_t x, uint8_t l0, uint8_t l1, uint8_t l2, uint8_t l3) {
	if (k > 2) {
		if (k > 3) {
			x = q_table[i][0][x] ^ l3;
		}
		x = q_table[i][1][x] ^ l2;
	}
	x = q_table[i][0][ q_table[i][1][ q_table[i][2][x] ^ l1] ^ l0];
	return MDS[i][x];
}

static inline uint32_t
h(int k, uint8_t x, uint32_t l0, uint32_t l1, uint32_t l2, uint32_t l3) {
	uint8_t y0, y1, y2, y3;
	y0 = B0(x);
	y1 = B1(x);
	y2 = B2(x);
	y3 = B3(x);
	if (k > 2) {
		if (k > 3) {
			y0 = Q1[y0] ^ B0(l3);
			y1 = Q0[y1] ^ B1(l3);
			y2 = Q0[y2] ^ B2(l3);
			y3 = Q1[y3] ^ B3(l3);
		}
		y0 = Q1[y0] ^ B0(l2);
		y1 = Q1[y1] ^ B1(l2);
		y2 = Q0[y2] ^ B2(l2);
		y3 = Q0[y3] ^ B3(l2);
	}
	y0 = Q1[  Q0[ Q0[y0] ^ B0(l1) ] ^ B0(l0)  ];
	y1 = Q0[  Q0[ Q1[y1] ^ B1(l1) ] ^ B1(l0)  ];
	y2 = Q1[  Q1[ Q0[y2] ^ B2(l1) ] ^ B2(l0)  ];
	y3 = Q0[  Q1[ Q1[y3] ^ B3(l1) ] ^ B3(l0)  ];
	return MDS[0][y0] ^ MDS[1][y1] ^ MDS[2][y2] ^ MDS[3][y3];
}

static inline uint32_t
compute_s(uint32_t m1, uint32_t m2) {
	return	RS[0][B0(m1)] ^ RS[1][B1(m1)] ^ RS[2][B2(m1)] ^ RS[3][B3(m1)] ^
			RS[4][B0(m2)] ^ RS[5][B1(m2)] ^ RS[6][B3(m2)] ^ RS[7][B3(m2)];
}

void
twofish_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length) {
	uint8_t key_pad[32];
	uint32_t m[8], s[4], t;
	int i, j, k;

	assert (TWOFISH_MIN_KEY_SIZE <= length && length <= TWOFISH_MAX_KEY_SIZE);
	memcpy(key_pad, key, length);
	memset(key_pad + length, 0, 32 - length);

	m[0] = LE_READ_UINT32(key_pad +  0);
	m[1] = LE_READ_UINT32(key_pad +  4);
	m[2] = LE_READ_UINT32(key_pad +  8);
	m[3] = LE_READ_UINT32(key_pad + 12);
	m[4] = LE_READ_UINT32(key_pad + 16);
	m[5] = LE_READ_UINT32(key_pad + 20);
	m[6] = LE_READ_UINT32(key_pad + 24);
	m[7] = LE_READ_UINT32(key_pad + 28);

	if (length <= 16) {
		k = 2;
	} else if (length <= 24) {
		k = 3;
	} else {
		k = 4;
	}

	/* Compute sub-keys */
	for (i = 0; i < 20; i++) {
		t = h(k, 2*i+1, m[1], m[3], m[5], m[7]);
		t = rol8(t);
		ctx->keys[2*i] = t + h(k, 2*i, m[0], m[2], m[4], m[6]);
		t += ctx->keys[2*i];
		t = rol9(t);
		ctx->keys[2*i+1] = t;
	}

	/* Compute S-Boxes */
	for (i = 0; i < k; i++) {
		s[k-i-1] = compute_s(m[2*i], m[2*i+1]);
	}
	for (i = 0; i < 0x100; i++) {
		ctx->s_box[0][i] = h_byte(k, 0, i, B0(s[0]), B0(s[1]), B0(s[2]), B0(s[3]));
		ctx->s_box[1][i] = h_byte(k, 1, i, B1(s[0]), B1(s[1]), B1(s[2]), B1(s[3]));
		ctx->s_box[2][i] = h_byte(k, 2, i, B2(s[0]), B2(s[1]), B2(s[2]), B2(s[3]));
		ctx->s_box[3][i] = h_byte(k, 3, i, B3(s[0]), B3(s[1]), B3(s[2]), B3(s[3]));
	}
}

void
twofish128_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == TWOFISH128_KEY_SIZE);
	twofish_set_key(ctx, key, length);
}

void
twofish192_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == TWOFISH192_KEY_SIZE);
	twofish_set_key(ctx, key, length);
}

void
twofish256_set_key(struct twofish_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == TWOFISH256_KEY_SIZE);
	twofish_set_key(ctx, key, length);
}

void
twofish_encrypt(const struct twofish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t r0, r1, r2, r3, t0, t1;
	const uint32_t *const keys = ctx->keys;
	const uint32_t (*const s_box)[256] = ctx->s_box;
	while (length >= 16) {
		r0 = LE_READ_UINT32(src +  0) ^ keys[0];
		r1 = LE_READ_UINT32(src +  4) ^ keys[1];
		r2 = LE_READ_UINT32(src +  8) ^ keys[2];
		r3 = LE_READ_UINT32(src + 12) ^ keys[3];

		for (int i = 0; i < 8; i++) {
			t1 = s_box[0][B3(r1)] ^ s_box[1][B0(r1)] ^ s_box[2][B1(r1)] ^ s_box[3][B2(r1)];
			t0 = s_box[0][B0(r0)] ^ s_box[1][B1(r0)] ^ s_box[2][B2(r0)] ^ s_box[3][B3(r0)];
			t0 += t1;
			r3 = (t1 + t0 + keys[4*i+9]) ^ rol1(r3);
			r2 = (t0 + keys[4*i+8]) ^ r2;
			r2 = ror1(r2);

			t1 = s_box[0][B3(r3)] ^ s_box[1][B0(r3)] ^ s_box[2][B1(r3)] ^ s_box[3][B2(r3)];
			t0 = s_box[0][B0(r2)] ^ s_box[1][B1(r2)] ^ s_box[2][B2(r2)] ^ s_box[3][B3(r2)];
			t0 += t1;
			r1 = (t1 + t0 + keys[4*i+11]) ^ rol1(r1);
			r0 = (t0 + keys[4*i+10]) ^ r0;
			r0 = ror1(r0);
		}

		r2 ^= keys[4];
		r3 ^= keys[5];
		r0 ^= keys[6];
		r1 ^= keys[7];

		LE_WRITE_UINT32(dst +  0, r2);
		LE_WRITE_UINT32(dst +  4, r3);
		LE_WRITE_UINT32(dst +  8, r0);
		LE_WRITE_UINT32(dst + 12, r1);
		src += 16;
		dst += 16;
		length -= 16;
	}
}

void
twofish_decrypt(const struct twofish_ctx *ctx, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t r0, r1, r2, r3, t0, t1;
	const uint32_t *const keys = ctx->keys;
	const uint32_t (*const s_box)[256] = ctx->s_box;
	while (length >= 16) {
		r0 = LE_READ_UINT32(src +  8) ^ keys[6];
		r1 = LE_READ_UINT32(src + 12) ^ keys[7];
		r2 = LE_READ_UINT32(src +  0) ^ keys[4];
		r3 = LE_READ_UINT32(src +  4) ^ keys[5];

		for (int i = 0; i < 8; i++) {
			t1 = s_box[0][B3(r3)] ^ s_box[1][B0(r3)] ^ s_box[2][B1(r3)] ^ s_box[3][B2(r3)];
			t0 = s_box[0][B0(r2)] ^ s_box[1][B1(r2)] ^ s_box[2][B2(r2)] ^ s_box[3][B3(r2)];
			t0 += t1;
			r1 = (t1 + t0 + keys[36-4*i+3]) ^ r1;
			r1 = ror1(r1);
			r0 = (t0 + keys[36-4*i+2]) ^ rol1(r0);

			t1 = s_box[0][B3(r1)] ^ s_box[1][B0(r1)] ^ s_box[2][B1(r1)] ^ s_box[3][B2(r1)];
			t0 = s_box[0][B0(r0)] ^ s_box[1][B1(r0)] ^ s_box[2][B2(r0)] ^ s_box[3][B3(r0)];
			t0 += t1;
			r3 = (t1 + t0 + keys[36-4*i+1]) ^ r3;
			r3 = ror1(r3);
			r2 = (t0 + keys[36-4*i+0]) ^ rol1(r2);
		}

		r0 ^= keys[0];
		r1 ^= keys[1];
		r2 ^= keys[2];
		r3 ^= keys[3];

		LE_WRITE_UINT32(dst +  0, r0);
		LE_WRITE_UINT32(dst +  4, r1);
		LE_WRITE_UINT32(dst +  8, r2);
		LE_WRITE_UINT32(dst + 12, r3);
		src += 16;
		dst += 16;
		length -= 16;
	}
}

