/*
 * mode-meta.c
 *
 *  Created on: Sep 3, 2019, 12:03:56 PM
 *      Author: Joshua Fehrenbach
 */

#include "crypto-meta.h"
#include "cbc.h"
#include "cfb.h"
#include "ccm.h"
#include "ctr.h"
#include "ecb.h"
#include "gcm.h"
#include "ofb.h"
#include <stddef.h>

const struct crypto_cipher_mode_meta crypto_cbc = {
	"cbc", sizeof(struct cbc_ctx), &cbc_init, &cbc_free,
	&cbc_set_encrypt_key, &cbc_set_decrypt_key,
	&cbc_encrypt, &cbc_decrypt,
	&cbc_get_block_size, &cbc_get_key_size, &cbc_get_block_size
};

const struct crypto_cipher_mode_meta crypto_cfb = {
	"cfb", sizeof(struct cfb_ctx), &cfb_init, &cfb_free,
	&cfb_set_key, &cfb_set_key,
	&cfb_encrypt, &cfb_decrypt,
	&cfb_get_block_size, &cfb_get_key_size, &cfb_get_iv_size
};

#define _CFB_META(s) const struct crypto_cipher_mode_meta crypto_cfb##s = { \
	"cfb" #s, sizeof(struct cfb_ctx), &cfb##s##_init, &cfb_free, \
	&cfb_set_key, &cfb_set_key, \
	&cfb_encrypt, &cfb_decrypt, \
	&cfb_get_block_size, &cfb_get_key_size, &cfb_get_iv_size \
}

_CFB_META(  8);
_CFB_META( 16);
_CFB_META( 24);
_CFB_META( 32);
_CFB_META( 40);
_CFB_META( 48);
_CFB_META( 56);
_CFB_META( 64);
_CFB_META( 72);
_CFB_META( 80);
_CFB_META( 88);
_CFB_META( 96);
_CFB_META(104);
_CFB_META(112);
_CFB_META(120);
_CFB_META(128);
_CFB_META(136);
_CFB_META(144);
_CFB_META(152);
_CFB_META(160);
_CFB_META(168);
_CFB_META(176);
_CFB_META(184);
_CFB_META(192);
_CFB_META(200);
_CFB_META(208);
_CFB_META(216);
_CFB_META(224);
_CFB_META(232);
_CFB_META(240);
_CFB_META(248);
_CFB_META(256);

#undef _CFB_META

const struct crypto_cipher_mode_meta crypto_ctr = {
	"ctr", sizeof(struct ctr_ctx), &ctr_init, &ctr_free,
	&ctr_set_key, &ctr_set_key,
	&ctr_encrypt, &ctr_encrypt,
	&ctr_get_block_size, &ctr_get_key_size, &ctr_get_block_size
};

const struct crypto_cipher_mode_meta crypto_ecb = {
	"ecb", sizeof(struct ecb_ctx), &ecb_init, &ecb_free,
	&ecb_set_encrypt_key, &ecb_set_decrypt_key,
	&ecb_encrypt, &ecb_decrypt,
	&ecb_get_block_size, &ecb_get_key_size, &ecb_get_iv_size
};

const struct crypto_cipher_mode_meta crypto_ofb = {
	"ofb", sizeof(struct ofb_ctx), &ofb_init, &ofb_free,
	&ofb_set_key, &ofb_set_key,
	&ofb_encrypt, &ofb_encrypt,
	&ofb_get_block_size, &ofb_get_key_size, &ofb_get_block_size
};


const struct crypto_cipher_mode_meta * const crypto_cipher_modes[] = {
	&crypto_cbc,
	&crypto_cfb,
	&crypto_ctr,
	&crypto_ecb,
	&crypto_ofb,
	&crypto_cfb8,   &crypto_cfb16,  &crypto_cfb24,  &crypto_cfb32,
	&crypto_cfb40,  &crypto_cfb48,  &crypto_cfb56,  &crypto_cfb64,
	&crypto_cfb72,  &crypto_cfb80,  &crypto_cfb88,  &crypto_cfb96,
	&crypto_cfb104, &crypto_cfb112, &crypto_cfb120, &crypto_cfb128,
	&crypto_cfb136, &crypto_cfb144, &crypto_cfb152, &crypto_cfb160,
	&crypto_cfb168, &crypto_cfb176, &crypto_cfb184, &crypto_cfb192,
	&crypto_cfb200, &crypto_cfb208, &crypto_cfb216, &crypto_cfb224,
	&crypto_cfb232, &crypto_cfb240, &crypto_cfb248, &crypto_cfb256,
	NULL
};


const struct crypto_aead_cipher_mode_meta crypto_ccm = {
	{
		"ccm", sizeof(struct ccm_ctx), &ccm_init, &ccm_free,
		&ccm_set_key_default, &ccm_set_key_default,
		&ccm_encrypt, &ccm_decrypt,
		&ccm_get_block_size, &ccm_get_key_size, &ccm_get_nonce_size
	}, &ccm_set_key, &ccm_set_key, &ccm_update, &ccm_digest,
	&ccm_encrypt_full, &ccm_decrypt_full, &ccm_get_digest_size
};

const struct crypto_aead_cipher_mode_meta crypto_gcm = {
	{
		"gcm", sizeof(struct gcm_ctx), &gcm_init, &gcm_free,
		&gcm_set_key_default, &gcm_set_key_default,
		&gcm_encrypt, &gcm_decrypt,
		&gcm_get_block_size, &gcm_get_key_size, &gcm_get_nonce_size
	}, &gcm_set_key, &gcm_set_key, &gcm_update, &gcm_digest,
	&gcm_encrypt_full, &gcm_decrypt_full, &gcm_get_digest_size
};


const struct crypto_aead_cipher_mode_meta * const crypto_aead_cipher_modes[] = {
	&crypto_ccm,
	&crypto_gcm,
	NULL
};

