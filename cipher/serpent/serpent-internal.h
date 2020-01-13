/*
 * serpent-internal.h
 *
 *  Created on: Feb 22, 2019, 3:06:08 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef SERPENT_INTERNAL_H_
#define SERPENT_INTERNAL_H_

#include "serpent.h"

#if __IS_x86__ && CRYPTO_FAT

/* Name mangling */
#define _serpent_encrypt (*_crypto_serpent_encrypt_fat)
#define _serpent_decrypt (*_crypto_serpent_decrypt_fat)

#define _serpent_encrypt_gen _crypto_serpent_encrypt_gen
#define _serpent_decrypt_gen _crypto_serpent_decrypt_gen

#define _serpent_encrypt_x64 _crypto_serpent_encrypt_x64
#define _serpent_encrypt_avx _crypto_serpent_encrypt_avx

#define _serpent_decrypt_x64 _crypto_serpent_decrypt_x64
#define _serpent_decrypt_avx _crypto_serpent_decrypt_avx
void
_serpent_encrypt_x64(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);
void
_serpent_encrypt_avx(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);

void
_serpent_decrypt_x64(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);
void
_serpent_decrypt_avx(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);

extern void _serpent_encrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);
extern void _serpent_decrypt(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);

#else

#define _serpent_encrypt _crypto_serpent_encrypt
#define _serpent_decrypt _crypto_serpent_decrypt

#define _serpent_encrypt_gen _serpent_encrypt
#define _serpent_decrypt_gen _serpent_decrypt

#endif

void
_serpent_encrypt_gen(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);
void
_serpent_decrypt_gen(const uint32_t(*)[4], uint8_t*, const uint8_t*, size_t);

#endif /* SERPENT_INTERNAL_H_ */
