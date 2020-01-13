/*
 * aes-internal.h
 *
 *  Created on: Feb 11, 2019, 1:04:57 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef AES_INTERNAL_H_
#define AES_INTERNAL_H_

#include "aes.h"

#define _aes_gf2_log _crypto_aes_gf2_log
#define _aes_gf2_exp _crypto_aes_gf2_exp
#define _aes_sbox _crypto_aes_sbox
#define _aes_ibox _crypto_aes_ibox
#define _aes_dtable _crypto_aes_dtable
#define _aes_itable _crypto_aes_itable
#define _aes_mtable _crypto_aes_mtable


#if __IS_x86__ && CRYPTO_FAT

/* Name mangling */
#define _aes128_set_key (*_crypto_aes128_set_key_fat)
#define _aes192_set_key (*_crypto_aes192_set_key_fat)
#define _aes256_set_key (*_crypto_aes256_set_key_fat)

#define _aes128_invert  (*_crypto_aes128_invert_fat)
#define _aes192_invert  (*_crypto_aes192_invert_fat)
#define _aes256_invert  (*_crypto_aes256_invert_fat)

#define _aes128_encrypt (*_crypto_aes128_encrypt_fat)
#define _aes192_encrypt (*_crypto_aes192_encrypt_fat)
#define _aes256_encrypt (*_crypto_aes256_encrypt_fat)

#define _aes128_decrypt (*_crypto_aes128_decrypt_fat)
#define _aes192_decrypt (*_crypto_aes192_decrypt_fat)
#define _aes256_decrypt (*_crypto_aes256_decrypt_fat)

#define _aes128_set_key_gen    _crypto_aes128_set_key_gen
#define _aes192_set_key_gen    _crypto_aes192_set_key_gen
#define _aes256_set_key_gen    _crypto_aes256_set_key_gen

#define _aes128_invert_gen    _crypto_aes128_invert_gen
#define _aes192_invert_gen    _crypto_aes192_invert_gen
#define _aes256_invert_gen    _crypto_aes256_invert_gen

#define _aes128_encrypt_gen    _crypto_aes128_encrypt_gen
#define _aes192_encrypt_gen    _crypto_aes192_encrypt_gen
#define _aes256_encrypt_gen    _crypto_aes256_encrypt_gen

#define _aes128_decrypt_gen    _crypto_aes128_decrypt_gen
#define _aes192_decrypt_gen    _crypto_aes192_decrypt_gen
#define _aes256_decrypt_gen    _crypto_aes256_decrypt_gen

#define _aes128_set_key_x86    _crypto_aes128_set_key_x86
#define _aes128_set_key_x86_64 _crypto_aes128_set_key_x86_64
#define _aes128_set_key_aesni  _crypto_aes128_set_key_aesni

#define _aes192_set_key_x86    _crypto_aes192_set_key_x86
#define _aes192_set_key_x86_64 _crypto_aes192_set_key_x86_64
#define _aes192_set_key_aesni  _crypto_aes192_set_key_aesni

#define _aes256_set_key_x86    _crypto_aes256_set_key_x86
#define _aes256_set_key_x86_64 _crypto_aes256_set_key_x86_64
#define _aes256_set_key_aesni  _crypto_aes256_set_key_aesni


#define _aes128_invert_x86    _crypto_aes128_invert_x86
#define _aes128_invert_x86_64 _crypto_aes128_invert_x86_64
#define _aes128_invert_aesni  _crypto_aes128_invert_aesni

#define _aes192_invert_x86    _crypto_aes192_invert_x86
#define _aes192_invert_x86_64 _crypto_aes192_invert_x86_64
#define _aes192_invert_aesni  _crypto_aes192_invert_aesni

#define _aes256_invert_x86    _crypto_aes256_invert_x86
#define _aes256_invert_x86_64 _crypto_aes256_invert_x86_64
#define _aes256_invert_aesni  _crypto_aes256_invert_aesni


#define _aes128_encrypt_x86    _crypto_aes128_encrypt_x86
#define _aes128_encrypt_x86_64 _crypto_aes128_encrypt_x86_64
#define _aes128_encrypt_aesni  _crypto_aes128_encrypt_aesni

#define _aes192_encrypt_x86    _crypto_aes192_encrypt_x86
#define _aes192_encrypt_x86_64 _crypto_aes192_encrypt_x86_64
#define _aes192_encrypt_aesni  _crypto_aes192_encrypt_aesni

#define _aes256_encrypt_x86    _crypto_aes256_encrypt_x86
#define _aes256_encrypt_x86_64 _crypto_aes256_encrypt_x86_64
#define _aes256_encrypt_aesni  _crypto_aes256_encrypt_aesni


#define _aes128_decrypt_x86    _crypto_aes128_decrypt_x86
#define _aes128_decrypt_x86_64 _crypto_aes128_decrypt_x86_64
#define _aes128_decrypt_aesni  _crypto_aes128_decrypt_aesni

#define _aes192_decrypt_x86    _crypto_aes192_decrypt_x86
#define _aes192_decrypt_x86_64 _crypto_aes192_decrypt_x86_64
#define _aes192_decrypt_aesni  _crypto_aes192_decrypt_aesni

#define _aes256_decrypt_x86    _crypto_aes256_decrypt_x86
#define _aes256_decrypt_x86_64 _crypto_aes256_decrypt_x86_64
#define _aes256_decrypt_aesni  _crypto_aes256_decrypt_aesni


void
_aes128_set_key_x86(uint32_t *subkeys, const uint8_t *key);
void
_aes128_set_key_x86_64(uint32_t *subkeys, const uint8_t *key);
void
_aes128_set_key_aesni(uint32_t *subkeys, const uint8_t *key);

void
_aes192_set_key_x86(uint32_t *subkeys, const uint8_t *key);
void
_aes192_set_key_x86_64(uint32_t *subkeys, const uint8_t *key);
void
_aes192_set_key_aesni(uint32_t *subkeys, const uint8_t *key);

void
_aes256_set_key_x86(uint32_t *subkeys, const uint8_t *key);
void
_aes256_set_key_x86_64(uint32_t *subkeys, const uint8_t *key);
void
_aes256_set_key_aesni(uint32_t *subkeys, const uint8_t *key);


void
_aes128_invert_x86(uint32_t *dst, const uint32_t *src);
void
_aes128_invert_x86_64(uint32_t *dst, const uint32_t *src);
void
_aes128_invert_aesni(uint32_t *dst, const uint32_t *src);

void
_aes192_invert_x86(uint32_t *dst, const uint32_t *src);
void
_aes192_invert_x86_64(uint32_t *dst, const uint32_t *src);
void
_aes192_invert_aesni(uint32_t *dst, const uint32_t *src);

void
_aes256_invert_x86(uint32_t *dst, const uint32_t *src);
void
_aes256_invert_x86_64(uint32_t *dst, const uint32_t *src);
void
_aes256_invert_aesni(uint32_t *dst, const uint32_t *src);


void
_aes128_encrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes128_encrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes128_encrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

void
_aes192_encrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_encrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_encrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

void
_aes256_encrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_encrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_encrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);


void
_aes128_decrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes128_decrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes128_decrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

void
_aes192_decrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_decrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_decrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

void
_aes256_decrypt_x86(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_decrypt_x86_64(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_decrypt_aesni(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

extern void _aes128_set_key(uint32_t *subkeys, const uint8_t *key);
extern void _aes192_set_key(uint32_t *subkeys, const uint8_t *key);
extern void _aes256_set_key(uint32_t *subkeys, const uint8_t *key);

extern void _aes128_invert(uint32_t *dst, const uint32_t *src);
extern void _aes192_invert(uint32_t *dst, const uint32_t *src);
extern void _aes256_invert(uint32_t *dst, const uint32_t *src);

extern void _aes128_encrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
extern void _aes192_encrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
extern void _aes256_encrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

extern void _aes128_decrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
extern void _aes192_decrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
extern void _aes256_decrypt(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

#else

/* Name mangling */
#define _aes128_set_key _crypto_aes128_set_key
#define _aes192_set_key _crypto_aes192_set_key
#define _aes256_set_key _crypto_aes256_set_key

#define _aes128_invert  _crypto_aes128_invert
#define _aes192_invert  _crypto_aes192_invert
#define _aes256_invert  _crypto_aes256_invert

#define _aes128_encrypt _crypto_aes128_encrypt
#define _aes192_encrypt _crypto_aes192_encrypt
#define _aes256_encrypt _crypto_aes256_encrypt

#define _aes128_decrypt _crypto_aes128_decrypt
#define _aes192_decrypt _crypto_aes192_decrypt
#define _aes256_decrypt _crypto_aes256_decrypt


#define _aes128_set_key_gen    _aes128_set_key
#define _aes192_set_key_gen    _aes192_set_key
#define _aes256_set_key_gen    _aes256_set_key

#define _aes128_invert_gen    _aes128_invert
#define _aes192_invert_gen    _aes192_invert
#define _aes256_invert_gen    _aes256_invert

#define _aes128_encrypt_gen    _aes128_encrypt
#define _aes192_encrypt_gen    _aes192_encrypt
#define _aes256_encrypt_gen    _aes256_encrypt

#define _aes128_decrypt_gen    _aes128_decrypt
#define _aes192_decrypt_gen    _aes192_decrypt
#define _aes256_decrypt_gen    _aes256_decrypt

#endif

void
_aes128_set_key_gen(uint32_t *subkeys, const uint8_t *key);
void
_aes192_set_key_gen(uint32_t *subkeys, const uint8_t *key);
void
_aes256_set_key_gen(uint32_t *subkeys, const uint8_t *key);

void
_aes128_invert_gen(uint32_t *dst, const uint32_t *src);
void
_aes192_invert_gen(uint32_t *dst, const uint32_t *src);
void
_aes256_invert_gen(uint32_t *dst, const uint32_t *src);

void
_aes128_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_encrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

void
_aes128_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes192_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);
void
_aes256_decrypt_gen(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length);

extern const uint8_t _aes_gf2_log[0x100];
extern const uint8_t _aes_gf2_exp[0x100];

extern const uint8_t _aes_sbox[0x100];
extern const uint8_t _aes_ibox[0x100];

extern const uint32_t _aes_dtable[4][0x100];
extern const uint32_t _aes_itable[4][0x100];
extern const uint32_t _aes_mtable[4][0x100];

#define B0(x) (((x) >> 0x00) & 0xFF)
#define B1(x) (((x) >> 0x08) & 0xFF)
#define B2(x) (((x) >> 0x10) & 0xFF)
#define B3(x) (((x) >> 0x18) & 0xFF)

#define R0(x,T) T[0][((x) >> 0x00) & 0xFF]
#define R1(x,T) T[1][((x) >> 0x08) & 0xFF]
#define R2(x,T) T[2][((x) >> 0x10) & 0xFF]
#define R3(x,T) T[3][((x) >> 0x18) & 0xFF]

#define S0(x,T) ((uint32_t)(T[((x) >> 0x00) & 0xFF]) << 0x00)
#define S1(x,T) ((uint32_t)(T[((x) >> 0x08) & 0xFF]) << 0x08)
#define S2(x,T) ((uint32_t)(T[((x) >> 0x10) & 0xFF]) << 0x10)
#define S3(x,T) ((uint32_t)(T[((x) >> 0x18) & 0xFF]) << 0x18)

#define SUB(a,b,c,d,T) (S0(a,T) | S1(b,T) | S2(c,T) | S3(d,T))

#define SUBBYTE(x) SUB(x,x,x,x,_aes_sbox)

#define M0(x) R0(x,_aes_mtable)
#define M1(x) R1(x,_aes_mtable)
#define M2(x) R2(x,_aes_mtable)
#define M3(x) R3(x,_aes_mtable)

#define INVERT_KEY(key) (M0(key) ^ M1(key) ^ M2(key) ^ M3(key))

#define AES_ROUND(T, w0, w1, w2, w3, k)		\
((  T[0][ B0(w0) ] ^ T[1][ B1(w1) ]			\
  ^ T[2][ B2(w2) ] ^ T[3][ B3(w3) ]) ^ (k))

#define AES_FINAL_ROUND(T, w0, w1, w2, w3, k)		\
((   (uint32_t) T[ B0(w0) ]				\
  | ((uint32_t) T[ B1(w1) ] << 8)		\
  | ((uint32_t) T[ B2(w2) ] << 16)		\
  | ((uint32_t) T[ B3(w3) ] << 24)) ^ (k))


#endif /* AES_INTERNAL_H_ */
