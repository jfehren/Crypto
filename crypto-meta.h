/*
 * crypto-meta.h
 *
 *  Created on: Sep 3, 2019, 9:04:31 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_META_H_
#define CRYPTO_META_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

__CRYPTO_DECLSPEC const struct crypto_cipher_meta * const crypto_ciphers[];

__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_aes;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_aes128;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_aes192;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_aes256;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_blowfish;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_des;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_des3;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_serpent;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_serpent128;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_serpent192;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_serpent256;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_twofish;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_twofish128;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_twofish192;
__CRYPTO_DECLSPEC const struct crypto_cipher_meta crypto_twofish256;


__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta * const crypto_cipher_modes[];

__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cbc;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb8;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb16;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb24;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb32;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb40;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb48;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb56;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb64;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb72;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb80;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb88;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb96;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb104;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb112;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb120;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb128;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb136;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb144;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb152;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb160;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb168;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb176;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb184;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb192;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb200;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb208;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb216;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb224;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb232;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb240;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb248;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_cfb256;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_ctr;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_ecb;
__CRYPTO_DECLSPEC const struct crypto_cipher_mode_meta crypto_ofb;


__CRYPTO_DECLSPEC const struct crypto_aead_cipher_mode_meta * const crypto_aead_cipher_modes[];

__CRYPTO_DECLSPEC const struct crypto_aead_cipher_mode_meta crypto_ccm;
__CRYPTO_DECLSPEC const struct crypto_aead_cipher_mode_meta crypto_gcm;


__CRYPTO_DECLSPEC const struct crypto_hash_meta * const crypto_hashes[];

__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md2;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md4;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md5;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md6_224;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md6_256;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md6_384;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_md6_512;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha1;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha224;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha256;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha384;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha512;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha512_224;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha512_256;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha3_224;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha3_256;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha3_384;
__CRYPTO_DECLSPEC const struct crypto_hash_meta crypto_sha3_512;

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_META_H_ */
