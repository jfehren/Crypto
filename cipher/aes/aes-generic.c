/*
 * aes-generic.c
 *
 *  Created on: Feb 11, 2019, 1:42:51 PM
 *      Author: Joshua Fehrenbach
 */

#include "aes-internal.h"
#include "macros.h"

#define SBOX	_aes_sbox
#define IBOX	_aes_ibox
#define DTABLE	_aes_dtable
#define ITABLE	_aes_itable

#define AES_ENCRYPT_ROUND(r, k, w0, w1, w2, w3, t0, t1, t2, t3, T) \
	(t0)  = R0(w0,T); (t3)  = R1(w0,T); (t2)  = R2(w0,T); (t1)  = R3(w0,T); \
	(t1) ^= R0(w1,T); (t0) ^= R1(w1,T); (t3) ^= R2(w1,T); (t2) ^= R3(w1,T); \
	(t2) ^= R0(w2,T); (t1) ^= R1(w2,T); (t0) ^= R2(w2,T); (t3) ^= R3(w2,T); \
	(t3) ^= R0(w3,T); (t2) ^= R1(w3,T); (t1) ^= R2(w3,T); (t0) ^= R3(w3,T); \
	(t0) ^= (k)[((r)*4)+0]; \
	(t1) ^= (k)[((r)*4)+1]; \
	(t2) ^= (k)[((r)*4)+2]; \
	(t3) ^= (k)[((r)*4)+3]
#define AES_ENCRYPT_FINAL(r, k, w0, w1, w2, w3, t0, t1, t2, t3, T) \
	(t0)  = S0(w0,T); (t3)  = S1(w0,T); (t2)  = S2(w0,T); (t1)  = S3(w0,T); \
	(t1) |= S0(w1,T); (t0) |= S1(w1,T); (t3) |= S2(w1,T); (t2) |= S3(w1,T); \
	(t2) |= S0(w2,T); (t1) |= S1(w2,T); (t0) |= S2(w2,T); (t3) |= S3(w2,T); \
	(t3) |= S0(w3,T); (t2) |= S1(w3,T); (t1) |= S2(w3,T); (t0) |= S3(w3,T); \
	(t0) ^= (k)[((r)*4)+0]; \
	(t1) ^= (k)[((r)*4)+1]; \
	(t2) ^= (k)[((r)*4)+2]; \
	(t3) ^= (k)[((r)*4)+3]

#define AES_DECRYPT_ROUND(r, k, w0,w1,w2,w3, t0,t1,t2,t3, T) \
	(t0)  = R0(w0,T); (t1)  = R1(w0,T); (t2)  = R2(w0,T); (t3)  = R3(w0,T); \
	(t1) ^= R0(w1,T); (t2) ^= R1(w1,T); (t3) ^= R2(w1,T); (t0) ^= R3(w1,T); \
	(t2) ^= R0(w2,T); (t3) ^= R1(w2,T); (t0) ^= R2(w2,T); (t1) ^= R3(w2,T); \
	(t3) ^= R0(w3,T); (t0) ^= R1(w3,T); (t1) ^= R2(w3,T); (t2) ^= R3(w3,T); \
	(t0) ^= (k)[((r)*4)+0]; \
	(t1) ^= (k)[((r)*4)+1]; \
	(t2) ^= (k)[((r)*4)+2]; \
	(t3) ^= (k)[((r)*4)+3]
#define AES_DECRYPT_FINAL(r, k, w0,w1,w2,w3, t0,t1,t2,t3, T) \
	(t0)  = S0(w0,T); (t1)  = S1(w0,T); (t2)  = S2(w0,T); (t3)  = S3(w0,T); \
	(t1) |= S0(w1,T); (t2) |= S1(w1,T); (t3) |= S2(w1,T); (t0) |= S3(w1,T); \
	(t2) |= S0(w2,T); (t3) |= S1(w2,T); (t0) |= S2(w2,T); (t1) |= S3(w2,T); \
	(t3) |= S0(w3,T); (t0) |= S1(w3,T); (t1) |= S2(w3,T); (t2) |= S3(w3,T); \
	(t0) ^= (k)[((r)*4)+0]; \
	(t1) ^= (k)[((r)*4)+1]; \
	(t2) ^= (k)[((r)*4)+2]; \
	(t3) ^= (k)[((r)*4)+3]

#define AES_INVERT_ROUND_KEY(r,dst,src) \
	(dst)[((r)*4)+0] = INVERT_KEY((src)[((r)*4)+0]); \
	(dst)[((r)*4)+1] = INVERT_KEY((src)[((r)*4)+1]); \
	(dst)[((r)*4)+2] = INVERT_KEY((src)[((r)*4)+2]); \
	(dst)[((r)*4)+3] = INVERT_KEY((src)[((r)*4)+3])

#define KEY_EXPAND128(r, k, rc) \
(k)[((r)*4)+0] = (k)[((r)*4)-4] ^ (rc) \
							 ^ ROTL32(SUBBYTE((k)[((r)*4)-1]), 24); \
(k)[((r)*4)+1] = (k)[((r)*4)-3] ^ (k)[((r)*4)+0]; \
(k)[((r)*4)+2] = (k)[((r)*4)-2] ^ (k)[((r)*4)+1]; \
(k)[((r)*4)+3] = (k)[((r)*4)-1] ^ (k)[((r)*4)+2]

#define KEY_EXPAND192_A(r, k, rc) \
(k)[((r)*6)+0] = (k)[((r)*6)-6] ^ (rc) \
							 ^ ROTL32(SUBBYTE((k)[((r)*6)-1]), 24); \
(k)[((r)*6)+1] = (k)[((r)*6)-5] ^ (k)[((r)*6)+0]; \
(k)[((r)*6)+2] = (k)[((r)*6)-4] ^ (k)[((r)*6)+1]; \
(k)[((r)*6)+3] = (k)[((r)*6)-3] ^ (k)[((r)*6)+2]
#define KEY_EXPAND192(r, k, rc) \
KEY_EXPAND192_A(r, k, rc); \
(k)[((r)*6)+4] = (k)[((r)*6)-2] ^ (k)[((r)*6)+3]; \
(k)[((r)*6)+5] = (k)[((r)*6)-1] ^ (k)[((r)*6)+4]

#define KEY_EXPAND256_A(r, k, rc) \
(k)[((r)*8)+0] = (k)[((r)*8)-8] ^ (rc) \
							 ^ ROTL32(SUBBYTE((k)[((r)*8)-1]), 24); \
(k)[((r)*8)+1] = (k)[((r)*8)-7] ^ (k)[((r)*8)+0]; \
(k)[((r)*8)+2] = (k)[((r)*8)-6] ^ (k)[((r)*8)+1]; \
(k)[((r)*8)+3] = (k)[((r)*8)-5] ^ (k)[((r)*8)+2]
#define KEY_EXPAND256(r, k, rc) \
KEY_EXPAND256_A(r, k, rc); \
(k)[((r)*8)+4] = (k)[((r)*8)-4] ^ SUBBYTE((k)[((r)*8)+3]); \
(k)[((r)*8)+5] = (k)[((r)*8)-3] ^ (k)[((r)*8)+4]; \
(k)[((r)*8)+6] = (k)[((r)*8)-2] ^ (k)[((r)*8)+5]; \
(k)[((r)*8)+7] = (k)[((r)*8)-1] ^ (k)[((r)*8)+6]

void
_aes128_set_key_gen(uint32_t *subkeys, const uint8_t *key) {
	subkeys[0] = LE_READ_UINT32(key +  0);
	subkeys[1] = LE_READ_UINT32(key +  4);
	subkeys[2] = LE_READ_UINT32(key +  8);
	subkeys[3] = LE_READ_UINT32(key + 12);
	KEY_EXPAND128( 1, subkeys, 0x01);
	KEY_EXPAND128( 2, subkeys, 0x02);
	KEY_EXPAND128( 3, subkeys, 0x04);
	KEY_EXPAND128( 4, subkeys, 0x08);
	KEY_EXPAND128( 5, subkeys, 0x10);
	KEY_EXPAND128( 6, subkeys, 0x20);
	KEY_EXPAND128( 7, subkeys, 0x40);
	KEY_EXPAND128( 8, subkeys, 0x80);
	KEY_EXPAND128( 9, subkeys, 0x1B);
	KEY_EXPAND128(10, subkeys, 0x36);
}

void
_aes192_set_key_gen(uint32_t *subkeys, const uint8_t *key) {
	subkeys[0] = LE_READ_UINT32(key +  0);
	subkeys[1] = LE_READ_UINT32(key +  4);
	subkeys[2] = LE_READ_UINT32(key +  8);
	subkeys[3] = LE_READ_UINT32(key + 12);
	subkeys[4] = LE_READ_UINT32(key + 16);
	subkeys[5] = LE_READ_UINT32(key + 20);
	KEY_EXPAND192( 1, subkeys, 0x01);
	KEY_EXPAND192( 2, subkeys, 0x02);
	KEY_EXPAND192( 3, subkeys, 0x04);
	KEY_EXPAND192( 4, subkeys, 0x08);
	KEY_EXPAND192( 5, subkeys, 0x10);
	KEY_EXPAND192( 6, subkeys, 0x20);
	KEY_EXPAND192( 7, subkeys, 0x40);
	KEY_EXPAND192_A( 8, subkeys, 0x80);
}

void
_aes256_set_key_gen(uint32_t *subkeys, const uint8_t *key) {
	subkeys[0] = LE_READ_UINT32(key +  0);
	subkeys[1] = LE_READ_UINT32(key +  4);
	subkeys[2] = LE_READ_UINT32(key +  8);
	subkeys[3] = LE_READ_UINT32(key + 12);
	subkeys[4] = LE_READ_UINT32(key + 16);
	subkeys[5] = LE_READ_UINT32(key + 20);
	subkeys[6] = LE_READ_UINT32(key + 24);
	subkeys[7] = LE_READ_UINT32(key + 28);
	KEY_EXPAND256( 1, subkeys, 0x01);
	KEY_EXPAND256( 2, subkeys, 0x02);
	KEY_EXPAND256( 3, subkeys, 0x04);
	KEY_EXPAND256( 4, subkeys, 0x08);
	KEY_EXPAND256( 5, subkeys, 0x10);
	KEY_EXPAND256( 6, subkeys, 0x20);
	KEY_EXPAND256_A( 7, subkeys, 0x40);
}


void
_aes128_invert_gen(uint32_t *dst, const uint32_t *src) {
	if (src != dst) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}
	AES_INVERT_ROUND_KEY( 1, dst, src);
	AES_INVERT_ROUND_KEY( 2, dst, src);
	AES_INVERT_ROUND_KEY( 3, dst, src);
	AES_INVERT_ROUND_KEY( 4, dst, src);
	AES_INVERT_ROUND_KEY( 5, dst, src);
	AES_INVERT_ROUND_KEY( 6, dst, src);
	AES_INVERT_ROUND_KEY( 7, dst, src);
	AES_INVERT_ROUND_KEY( 8, dst, src);
	AES_INVERT_ROUND_KEY( 9, dst, src);
	if (src != dst) {
		dst[40] = src[40];
		dst[41] = src[41];
		dst[42] = src[42];
		dst[43] = src[43];
	}
}

void
_aes192_invert_gen(uint32_t *dst, const uint32_t *src) {
	if (src != dst) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}
	AES_INVERT_ROUND_KEY( 1, dst, src);
	AES_INVERT_ROUND_KEY( 2, dst, src);
	AES_INVERT_ROUND_KEY( 3, dst, src);
	AES_INVERT_ROUND_KEY( 4, dst, src);
	AES_INVERT_ROUND_KEY( 5, dst, src);
	AES_INVERT_ROUND_KEY( 6, dst, src);
	AES_INVERT_ROUND_KEY( 7, dst, src);
	AES_INVERT_ROUND_KEY( 8, dst, src);
	AES_INVERT_ROUND_KEY( 9, dst, src);
	AES_INVERT_ROUND_KEY(10, dst, src);
	AES_INVERT_ROUND_KEY(11, dst, src);
	if (src != dst) {
		dst[48] = src[48];
		dst[49] = src[49];
		dst[50] = src[50];
		dst[51] = src[51];
	}
}

void
_aes256_invert_gen(uint32_t *dst, const uint32_t *src) {
	if (src != dst) {
		dst[0] = src[0];
		dst[1] = src[1];
		dst[2] = src[2];
		dst[3] = src[3];
	}
	AES_INVERT_ROUND_KEY( 1, dst, src);
	AES_INVERT_ROUND_KEY( 2, dst, src);
	AES_INVERT_ROUND_KEY( 3, dst, src);
	AES_INVERT_ROUND_KEY( 4, dst, src);
	AES_INVERT_ROUND_KEY( 5, dst, src);
	AES_INVERT_ROUND_KEY( 6, dst, src);
	AES_INVERT_ROUND_KEY( 7, dst, src);
	AES_INVERT_ROUND_KEY( 8, dst, src);
	AES_INVERT_ROUND_KEY( 9, dst, src);
	AES_INVERT_ROUND_KEY(10, dst, src);
	AES_INVERT_ROUND_KEY(11, dst, src);
	AES_INVERT_ROUND_KEY(12, dst, src);
	AES_INVERT_ROUND_KEY(13, dst, src);
	if (src != dst) {
		dst[56] = src[56];
		dst[57] = src[57];
		dst[58] = src[58];
		dst[59] = src[59];
	}
}


void
_aes128_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const dtable)[0x100] = DTABLE;
	register const uint8_t *const sbox = SBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[ 0*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[ 0*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[ 0*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[ 0*4 + 3];
		AES_ENCRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_FINAL(10, keys, t0, t1, t2, t3, w0, w1, w2, w3, sbox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}

void
_aes192_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const dtable)[0x100] = DTABLE;
	register const uint8_t *const sbox = SBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[ 0*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[ 0*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[ 0*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[ 0*4 + 3];
		AES_ENCRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND(10, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND(11, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_FINAL(12, keys, t0, t1, t2, t3, w0, w1, w2, w3, sbox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}

void
_aes256_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const dtable)[0x100] = DTABLE;
	register const uint8_t *const sbox = SBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[ 0*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[ 0*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[ 0*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[ 0*4 + 3];
		AES_ENCRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND(10, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND(11, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_ROUND(12, keys, t0, t1, t2, t3, w0, w1, w2, w3, dtable);
		AES_ENCRYPT_ROUND(13, keys, w0, w1, w2, w3, t0, t1, t2, t3, dtable);
		AES_ENCRYPT_FINAL(14, keys, t0, t1, t2, t3, w0, w1, w2, w3, sbox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}


void
_aes128_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const itable)[0x100] = ITABLE;
	register const uint8_t *const ibox = IBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[10*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[10*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[10*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[10*4 + 3];
		AES_DECRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_FINAL( 0, keys, t0, t1, t2, t3, w0, w1, w2, w3, ibox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}

void
_aes192_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const itable)[0x100] = ITABLE;
	register const uint8_t *const ibox = IBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[12*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[12*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[12*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[12*4 + 3];
		AES_DECRYPT_ROUND(11, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND(10, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_FINAL( 0, keys, t0, t1, t2, t3, w0, w1, w2, w3, ibox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}

void
_aes256_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	register uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
	register const uint32_t (*const itable)[0x100] = ITABLE;
	register const uint8_t *const ibox = IBOX;
	while (length >= 16) {
		w0 = LE_READ_UINT32(src +  0) ^ keys[14*4 + 0];
		w1 = LE_READ_UINT32(src +  4) ^ keys[14*4 + 1];
		w2 = LE_READ_UINT32(src +  8) ^ keys[14*4 + 2];
		w3 = LE_READ_UINT32(src + 12) ^ keys[14*4 + 3];
		AES_DECRYPT_ROUND(13, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND(12, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND(11, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND(10, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 9, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 8, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 7, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 6, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 5, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 4, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 3, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_ROUND( 2, keys, t0, t1, t2, t3, w0, w1, w2, w3, itable);
		AES_DECRYPT_ROUND( 1, keys, w0, w1, w2, w3, t0, t1, t2, t3, itable);
		AES_DECRYPT_FINAL( 0, keys, t0, t1, t2, t3, w0, w1, w2, w3, ibox);
		LE_WRITE_UINT32(dst +  0, w0);
		LE_WRITE_UINT32(dst +  4, w1);
		LE_WRITE_UINT32(dst +  8, w2);
		LE_WRITE_UINT32(dst + 12, w3);
		dst += 16;
		src += 16;
		length -= 16;
	}
}

