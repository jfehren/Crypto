/*
 * cipher-meta.c
 *
 *  Created on: Sep 3, 2019, 10:06:14 AM
 *      Author: Joshua Fehrenbach
 */

#include "crypto-meta.h"
#include "aes.h"
#include "blowfish.h"
#include "des.h"
#include "serpent.h"
#include "twofish.h"
#include <stddef.h>
#include <stdalign.h>

const struct crypto_cipher_meta crypto_aes = {
	"aes", sizeof(struct aes_ctx), alignof(struct aes_ctx),
	AES_BLOCK_SIZE, AES_MAX_KEY_SIZE,
	&aes_set_encrypt_key, &aes_set_decrypt_key,
	&aes_encrypt, &aes_decrypt
};

const struct crypto_cipher_meta crypto_aes128 = {
	"aes128", sizeof(struct aes128_ctx), alignof(struct aes128_ctx),
	AES_BLOCK_SIZE, AES128_KEY_SIZE,
	&aes128_set_encrypt_key, &aes128_set_decrypt_key,
	&aes128_encrypt, &aes128_decrypt
};

const struct crypto_cipher_meta crypto_aes192 = {
	"aes192", sizeof(struct aes192_ctx), alignof(struct aes192_ctx),
	AES_BLOCK_SIZE, AES192_KEY_SIZE,
	&aes192_set_encrypt_key, &aes192_set_decrypt_key,
	&aes192_encrypt, &aes192_decrypt
};

const struct crypto_cipher_meta crypto_aes256 = {
	"aes256", sizeof(struct aes256_ctx), alignof(struct aes256_ctx),
	AES_BLOCK_SIZE, AES256_KEY_SIZE,
	&aes256_set_encrypt_key, &aes256_set_decrypt_key,
	&aes256_encrypt, &aes256_decrypt
};

const struct crypto_cipher_meta crypto_blowfish = {
	"blowfish", sizeof(struct blowfish_ctx), alignof(struct blowfish_ctx),
	BLOWFISH_BLOCK_SIZE, BLOWFISH_KEY_SIZE,
	&blowfish_set_key, &blowfish_set_key,
	&blowfish_encrypt, &blowfish_decrypt
};

const struct crypto_cipher_meta crypto_des = {
	"des", sizeof(struct des_ctx), alignof(struct des_ctx),
	DES_BLOCK_SIZE, DES_KEY_SIZE,
	&des_set_key, &des_set_key,
	&des_encrypt, &des_decrypt
};

const struct crypto_cipher_meta crypto_des3 = {
	"des3", sizeof(struct des3_ctx), alignof(struct des3_ctx),
	DES3_BLOCK_SIZE, DES3_KEY_SIZE,
	&des3_set_key, &des3_set_key,
	&des3_encrypt, &des3_decrypt
};

const struct crypto_cipher_meta crypto_serpent = {
	"serpent", sizeof(struct serpent_ctx), alignof(struct serpent_ctx),
	SERPENT_BLOCK_SIZE, SERPENT_MAX_KEY_SIZE,
	&serpent_set_key, &serpent_set_key,
	&serpent_encrypt, &serpent_decrypt
};

const struct crypto_cipher_meta crypto_serpent128 = {
	"serpent128", sizeof(struct serpent_ctx), alignof(struct serpent_ctx),
	SERPENT_BLOCK_SIZE, SERPENT128_KEY_SIZE,
	&serpent128_set_key, &serpent128_set_key,
	&serpent_encrypt, &serpent_decrypt
};

const struct crypto_cipher_meta crypto_serpent192 = {
	"serpent192", sizeof(struct serpent_ctx), alignof(struct serpent_ctx),
	SERPENT_BLOCK_SIZE, SERPENT192_KEY_SIZE,
	&serpent192_set_key, &serpent192_set_key,
	&serpent_encrypt, &serpent_decrypt
};

const struct crypto_cipher_meta crypto_serpent256 = {
	"serpent256", sizeof(struct serpent_ctx), alignof(struct serpent_ctx),
	SERPENT_BLOCK_SIZE, SERPENT256_KEY_SIZE,
	&serpent256_set_key, &serpent256_set_key,
	&serpent_encrypt, &serpent_decrypt
};

const struct crypto_cipher_meta crypto_twofish = {
	"twofish", sizeof(struct twofish_ctx), alignof(struct twofish_ctx),
	TWOFISH_BLOCK_SIZE, TWOFISH_MAX_KEY_SIZE,
	&twofish_set_key, &twofish_set_key,
	&twofish_encrypt, &twofish_decrypt
};

const struct crypto_cipher_meta crypto_twofish128 = {
	"twofish128", sizeof(struct twofish_ctx), alignof(struct twofish_ctx),
	TWOFISH_BLOCK_SIZE, TWOFISH128_KEY_SIZE,
	&twofish128_set_key, &twofish128_set_key,
	&twofish_encrypt, &twofish_decrypt
};

const struct crypto_cipher_meta crypto_twofish192 = {
	"twofish192", sizeof(struct twofish_ctx), alignof(struct twofish_ctx),
	TWOFISH_BLOCK_SIZE, TWOFISH192_KEY_SIZE,
	&twofish192_set_key, &twofish192_set_key,
	&twofish_encrypt, &twofish_decrypt
};

const struct crypto_cipher_meta crypto_twofish256 = {
	"twofish256", sizeof(struct twofish_ctx), alignof(struct twofish_ctx),
	TWOFISH_BLOCK_SIZE, TWOFISH256_KEY_SIZE,
	&twofish256_set_key, &twofish256_set_key,
	&twofish_encrypt, &twofish_decrypt
};


const struct crypto_cipher_meta * const crypto_cipher[] = {
	&crypto_aes,
	&crypto_aes128,
	&crypto_aes192,
	&crypto_aes256,
	&crypto_blowfish,
	&crypto_des,
	&crypto_des3,
	&crypto_serpent,
	&crypto_serpent128,
	&crypto_serpent192,
	&crypto_serpent256,
	&crypto_twofish,
	&crypto_twofish128,
	&crypto_twofish192,
	&crypto_twofish256,
	NULL
};

