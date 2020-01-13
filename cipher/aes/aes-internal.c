/*
 * aes-internal.c
 *
 *  Created on: Feb 11, 2019, 2:16:53 PM
 *      Author: Joshua Fehrenbach
 */

#include "aes-internal.h"
#include "crypto-internal.h"
#include "macros.h"
#include <assert.h>

void
aes_set_encrypt_key(struct aes_ctx *ctx, const uint8_t *key, size_t length) {
	assert (AES_MIN_KEY_SIZE <= length && length <= AES_MAX_KEY_SIZE && length % 4 == 0);
	ctx->rounds = (length / 4) + 6;
	if (ctx->rounds == _AES128_ROUNDS && length == AES128_KEY_SIZE) {
		_aes128_set_key(ctx->keys, key);
	} else if (ctx->rounds == _AES192_ROUNDS && length == AES192_KEY_SIZE) {
		_aes192_set_key(ctx->keys, key);
	} else if (ctx->rounds == _AES256_ROUNDS && length == AES256_KEY_SIZE) {
		_aes256_set_key(ctx->keys, key);
	} else {
		static const uint8_t rcon[10] = {
			0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x1b,0x36,
		};
		const uint8_t *rp;
		unsigned lastkey, i, nk;
		uint32_t t;

		lastkey = (AES_BLOCK_SIZE/4) * (ctx->rounds + 1);
		nk = length / 4;

		for (i = 0, rp = rcon; i < nk; i++) {
			ctx->keys[i] = LE_READ_UINT32(key + i*4);
		}

		for (i = nk; i < lastkey; i++) {
			t = ctx->keys[i-1];
			if (i % nk == 0) {
				t = SUBBYTE(ROTL32(t, 24)) ^ *rp++;
			} else if (nk > 6 && (i%nk) == 4) {
				t = SUBBYTE(t);
			}
			ctx->keys[i] = ctx->keys[i-nk] ^ t;
		}
	}
}

void
aes_set_decrypt_key(struct aes_ctx *ctx, const uint8_t *key, size_t length) {
	aes_set_encrypt_key(ctx, key, length);
	aes_invert_key(ctx, ctx);
}

void
aes_invert_key(struct aes_ctx *dst, const struct aes_ctx *src) {
	unsigned rounds = src->rounds;
	if (dst != src) {
		dst->rounds = rounds;
	}
	if (rounds == _AES128_ROUNDS) {
		_aes128_invert(dst->keys, src->keys);
	} else if (rounds == _AES192_ROUNDS) {
		_aes192_invert(dst->keys, src->keys);
	} else if (rounds == _AES256_ROUNDS) {
		_aes256_invert(dst->keys, src->keys);
	} else {
		unsigned i;
		const uint32_t *sk = src->keys;
		uint32_t *dk = dst->keys;
		if (dst != src) {
			dst->keys[0] = src->keys[0];
			dst->keys[1] = src->keys[1];
			dst->keys[2] = src->keys[2];
			dst->keys[3] = src->keys[3];

			dst->keys[4*rounds+0] = src->keys[4*rounds+0];
			dst->keys[4*rounds+1] = src->keys[4*rounds+1];
			dst->keys[4*rounds+2] = src->keys[4*rounds+2];
			dst->keys[4*rounds+3] = src->keys[4*rounds+3];
		}
		for (i = 4; i < 4*rounds; i++) {
			dst->keys[i] = INVERT_KEY(src->keys[i]);
		}
	}
}

void
aes_encrypt(const struct aes_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned rounds = ctx->rounds;
	const uint32_t *const keys = ctx->keys;
	if (ctx->rounds == _AES128_ROUNDS) {
		_aes128_encrypt(ctx->keys, dst, src, length);
	} else if (ctx->rounds == _AES192_ROUNDS) {
		_aes192_encrypt(ctx->keys, dst, src, length);
	} else if (ctx->rounds == _AES256_ROUNDS) {
		_aes256_encrypt(ctx->keys, dst, src, length);
	} else {
		uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
		unsigned i;
		while (length >= 16) {
			if (rounds % 2 == 0) {
				t0 = LE_READ_UINT32(src +  0) ^ keys[0];
				t1 = LE_READ_UINT32(src +  4) ^ keys[1];
				t2 = LE_READ_UINT32(src +  8) ^ keys[2];
				t3 = LE_READ_UINT32(src + 12) ^ keys[3];

				w0 = AES_ROUND(_aes_dtable, t0, t1, t2, t3, keys[4]);
				w1 = AES_ROUND(_aes_dtable, t1, t2, t3, t0, keys[5]);
				w2 = AES_ROUND(_aes_dtable, t2, t3, t0, t1, keys[6]);
				w3 = AES_ROUND(_aes_dtable, t3, t0, t1, t2, keys[7]);
				i = 2;
			} else {
				w0 = LE_READ_UINT32(src +  0) ^ keys[0];
				w1 = LE_READ_UINT32(src +  4) ^ keys[1];
				w2 = LE_READ_UINT32(src +  8) ^ keys[2];
				w3 = LE_READ_UINT32(src + 12) ^ keys[3];
				i = 1;
			}

			while (i < rounds) {
				t0 = AES_ROUND(_aes_dtable, w0, w1, w2, w3, keys[4*i + 0]);
				t1 = AES_ROUND(_aes_dtable, w1, w2, w3, w0, keys[4*i + 1]);
				t2 = AES_ROUND(_aes_dtable, w2, w3, w0, w1, keys[4*i + 2]);
				t3 = AES_ROUND(_aes_dtable, w3, w0, w1, w2, keys[4*i + 3]);

				w0 = AES_ROUND(_aes_dtable, t0, t1, t2, t3, keys[4*i + 4]);
				w1 = AES_ROUND(_aes_dtable, t1, t2, t3, t0, keys[4*i + 5]);
				w2 = AES_ROUND(_aes_dtable, t2, t3, t0, t1, keys[4*i + 6]);
				w3 = AES_ROUND(_aes_dtable, t3, t0, t1, t2, keys[4*i + 7]);
				i += 2;
			}

			t0 = AES_FINAL_ROUND(_aes_sbox, w0, w1, w2, w3, keys[4*rounds + 0]);
			t1 = AES_FINAL_ROUND(_aes_sbox, w1, w2, w3, w0, keys[4*rounds + 1]);
			t2 = AES_FINAL_ROUND(_aes_sbox, w2, w3, w0, w1, keys[4*rounds + 2]);
			t3 = AES_FINAL_ROUND(_aes_sbox, w3, w0, w1, w2, keys[4*rounds + 3]);

			LE_WRITE_UINT32(dst +  0, t0);
			LE_WRITE_UINT32(dst +  4, t1);
			LE_WRITE_UINT32(dst +  8, t2);
			LE_WRITE_UINT32(dst + 12, t3);

			dst += 16;
			src += 16;
			length -= 16;
		}
	}
}

void
aes_decrypt(const struct aes_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	const unsigned rounds = ctx->rounds;
	const uint32_t *const keys = ctx->keys;
	if (rounds == _AES128_ROUNDS) {
		_aes128_decrypt(keys, dst, src, length);
	} else if (rounds == _AES192_ROUNDS) {
		_aes192_decrypt(keys, dst, src, length);
	} else if (rounds == _AES256_ROUNDS) {
		_aes256_decrypt(keys, dst, src, length);
	} else {
		uint32_t w0, w1, w2, w3, t0, t1, t2, t3;
		unsigned i;
		while (length >= 16) {
			if (rounds % 2 == 0) {
				t0 = LE_READ_UINT32(src +  0) ^ keys[4*rounds + 0];
				t1 = LE_READ_UINT32(src +  4) ^ keys[4*rounds + 1];
				t2 = LE_READ_UINT32(src +  8) ^ keys[4*rounds + 2];
				t3 = LE_READ_UINT32(src + 12) ^ keys[4*rounds + 3];

				w0 = AES_ROUND(_aes_itable, t0, t3, t2, t1, keys[4*rounds - 4]);
				w1 = AES_ROUND(_aes_itable, t1, t0, t3, t2, keys[4*rounds - 3]);
				w2 = AES_ROUND(_aes_itable, t2, t1, t0, t3, keys[4*rounds - 2]);
				w3 = AES_ROUND(_aes_itable, t3, t2, t1, t0, keys[4*rounds - 1]);
				i = rounds - 1;
			} else {
				w0 = LE_READ_UINT32(src +  0) ^ keys[4*rounds + 0];
				w1 = LE_READ_UINT32(src +  4) ^ keys[4*rounds + 1];
				w2 = LE_READ_UINT32(src +  8) ^ keys[4*rounds + 2];
				w3 = LE_READ_UINT32(src + 12) ^ keys[4*rounds + 3];
				i = rounds;
			}

			while (i >= 2) {
				t0 = AES_ROUND(_aes_itable, w0, w3, w2, w1, keys[4*i - 8]);
				t1 = AES_ROUND(_aes_itable, w1, w0, w3, w2, keys[4*i - 7]);
				t2 = AES_ROUND(_aes_itable, w2, w1, w0, w3, keys[4*i - 6]);
				t3 = AES_ROUND(_aes_itable, w3, w2, w1, w0, keys[4*i - 5]);

				w0 = AES_ROUND(_aes_itable, t0, t3, t2, t1, keys[4*i - 4]);
				w1 = AES_ROUND(_aes_itable, t1, t0, t3, t2, keys[4*i - 3]);
				w2 = AES_ROUND(_aes_itable, t2, t1, t0, t3, keys[4*i - 2]);
				w3 = AES_ROUND(_aes_itable, t3, t2, t1, t0, keys[4*i - 1]);
				i -= 2;
			}

			t0 = AES_FINAL_ROUND(_aes_ibox, w0, w3, w2, w1, keys[0]);
			t1 = AES_FINAL_ROUND(_aes_ibox, w1, w0, w3, w2, keys[1]);
			t2 = AES_FINAL_ROUND(_aes_ibox, w2, w1, w0, w3, keys[2]);
			t3 = AES_FINAL_ROUND(_aes_ibox, w3, w2, w1, w0, keys[3]);

			LE_WRITE_UINT32(dst +  0, t0);
			LE_WRITE_UINT32(dst +  4, t1);
			LE_WRITE_UINT32(dst +  8, t2);
			LE_WRITE_UINT32(dst + 12, t3);

			dst += 16;
			src += 16;
			length -= 16;
		}
	}
}


void
aes128_set_encrypt_key(struct aes128_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES128_KEY_SIZE);
	_aes128_set_key(ctx->keys, key);
}

void
aes128_set_decrypt_key(struct aes128_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES128_KEY_SIZE);
	_aes128_set_key(ctx->keys, key);
	_aes128_invert(ctx->keys, ctx->keys);
}

void
aes128_invert_key(struct aes128_ctx *dst, const struct aes128_ctx *src) {
	_aes128_invert(dst->keys, src->keys);
}

void
aes128_encrypt(const struct aes128_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes128_encrypt(ctx->keys, dst, src, length);
}

void
aes128_decrypt(const struct aes128_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes128_decrypt(ctx->keys, dst, src, length);
}


void
aes192_set_encrypt_key(struct aes192_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES128_KEY_SIZE);
	_aes192_set_key(ctx->keys, key);
}

void
aes192_set_decrypt_key(struct aes192_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES192_KEY_SIZE);
	_aes192_set_key(ctx->keys, key);
	_aes192_invert(ctx->keys, ctx->keys);
}

void
aes192_invert_key(struct aes192_ctx *dst, const struct aes192_ctx *src) {
	_aes192_invert(dst->keys, src->keys);
}

void
aes192_encrypt(const struct aes192_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes192_encrypt(ctx->keys, dst, src, length);
}

void
aes192_decrypt(const struct aes192_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes192_decrypt(ctx->keys, dst, src, length);
}


void
aes256_set_encrypt_key(struct aes256_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES256_KEY_SIZE);
	_aes256_set_key(ctx->keys, key);
}

void
aes256_set_decrypt_key(struct aes256_ctx *ctx, const uint8_t *key, size_t length) {
	assert (length == AES256_KEY_SIZE);
	_aes256_set_key(ctx->keys, key);
	_aes256_invert(ctx->keys, ctx->keys);
}

void
aes256_invert_key(struct aes256_ctx *dst, const struct aes256_ctx *src) {
	_aes256_invert(dst->keys, src->keys);
}

void
aes256_encrypt(const struct aes256_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes256_encrypt(ctx->keys, dst, src, length);
}

void
aes256_decrypt(const struct aes256_ctx *ctx, uint8_t *dst, const uint8_t *src, size_t length) {
	_aes256_decrypt(ctx->keys, dst, src, length);
}


#if __IS_x86__ && CRYPTO_FAT

static void
_crypto_aes128_set_key_default(uint32_t *subkeys, const uint8_t *key) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes128_set_key_fat = &_aes128_set_key_aesni;
		} else {
			_crypto_aes128_set_key_fat = &_aes128_set_key_x86_64;
		}
	} else {
		_crypto_aes128_set_key_fat = &_aes128_set_key_x86;
	}
	_aes128_set_key(subkeys, key);
}

static void
_crypto_aes192_set_key_default(uint32_t *subkeys, const uint8_t *key) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes192_set_key_fat = &_aes192_set_key_aesni;
		} else {
			_crypto_aes192_set_key_fat = &_aes192_set_key_x86_64;
		}
	} else {
		_crypto_aes192_set_key_fat = &_aes192_set_key_x86;
	}
	_aes192_set_key(subkeys, key);
}

static void
_crypto_aes256_set_key_default(uint32_t *subkeys, const uint8_t *key) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes256_set_key_fat = &_aes256_set_key_aesni;
		} else {
			_crypto_aes256_set_key_fat = &_aes256_set_key_x86_64;
		}
	} else {
		_crypto_aes256_set_key_fat = &_aes256_set_key_x86;
	}
	_aes256_set_key(subkeys, key);
}


static void
_crypto_aes128_invert_default(uint32_t *dst, const uint32_t *src) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes128_invert_fat = &_aes128_invert_aesni;
		} else {
			_crypto_aes128_invert_fat = &_aes128_invert_x86_64;
		}
	} else {
		_crypto_aes128_invert_fat = &_aes128_invert_x86;
	}
	_aes128_invert(dst, src);
}

static void
_crypto_aes192_invert_default(uint32_t *dst, const uint32_t *src) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes192_invert_fat = &_aes192_invert_aesni;
		} else {
			_crypto_aes192_invert_fat = &_aes192_invert_x86_64;
		}
	} else {
		_crypto_aes192_invert_fat = &_aes192_invert_x86;
	}
	_aes192_invert(dst, src);
}

static void
_crypto_aes256_invert_default(uint32_t *dst, const uint32_t *src) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes256_invert_fat = &_aes256_invert_aesni;
		} else {
			_crypto_aes256_invert_fat = &_aes256_invert_x86_64;
		}
	} else {
		_crypto_aes256_invert_fat = &_aes256_invert_x86;
	}
	_aes256_invert(dst, src);
}


static void
_crypto_aes128_encrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes128_encrypt_fat = &_aes128_encrypt_aesni;
		} else {
			_crypto_aes128_encrypt_fat = &_aes128_encrypt_x86_64;
		}
	} else {
		_crypto_aes128_encrypt_fat = &_aes128_encrypt_x86;
	}
	_aes128_encrypt(keys, dst, src, length);
}

static void
_crypto_aes192_encrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes192_encrypt_fat = &_aes192_encrypt_aesni;
		} else {
			_crypto_aes192_encrypt_fat = &_aes192_encrypt_x86_64;
		}
	} else {
		_crypto_aes192_encrypt_fat = &_aes192_encrypt_x86;
	}
	_aes192_encrypt(keys, dst, src, length);
}

static void
_crypto_aes256_encrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes256_encrypt_fat = &_aes256_encrypt_aesni;
		} else {
			_crypto_aes256_encrypt_fat = &_aes256_encrypt_x86_64;
		}
	} else {
		_crypto_aes256_encrypt_fat = &_aes256_encrypt_x86;
	}
	_aes256_encrypt(keys, dst, src, length);
}


static void
_crypto_aes128_decrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes128_decrypt_fat = &_aes128_decrypt_aesni;
		} else {
			_crypto_aes128_decrypt_fat = &_aes128_decrypt_x86_64;
		}
	} else {
		_crypto_aes128_decrypt_fat = &_aes128_decrypt_x86;
	}
	_aes128_decrypt(keys, dst, src, length);
}

static void
_crypto_aes192_decrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes192_decrypt_fat = &_aes192_decrypt_aesni;
		} else {
			_crypto_aes192_decrypt_fat = &_aes192_decrypt_x86_64;
		}
	} else {
		_crypto_aes192_decrypt_fat = &_aes192_decrypt_x86;
	}
	_aes192_decrypt(keys, dst, src, length);
}

static void
_crypto_aes256_decrypt_default(const uint32_t *keys, uint8_t *dst,
		const uint8_t *src, size_t length) {
	if (is_x86_64()) {
		if (is_aesni()) {
			_crypto_aes256_decrypt_fat = &_aes256_decrypt_aesni;
		} else {
			_crypto_aes256_decrypt_fat = &_aes256_decrypt_x86_64;
		}
	} else {
		_crypto_aes256_decrypt_fat = &_aes256_decrypt_x86;
	}
	_aes256_decrypt(keys, dst, src, length);
}


void _aes128_set_key(uint32_t*, const uint8_t*) = &_crypto_aes128_set_key_default;
void _aes192_set_key(uint32_t*, const uint8_t*) = &_crypto_aes192_set_key_default;
void _aes256_set_key(uint32_t*, const uint8_t*) = &_crypto_aes256_set_key_default;

void _aes128_invert(uint32_t*, const uint32_t*) = &_crypto_aes128_invert_default;
void _aes192_invert(uint32_t*, const uint32_t*) = &_crypto_aes192_invert_default;
void _aes256_invert(uint32_t*, const uint32_t*) = &_crypto_aes256_invert_default;

void _aes128_encrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes128_encrypt_default;
void _aes192_encrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes192_encrypt_default;
void _aes256_encrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes256_encrypt_default;

void _aes128_decrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes128_decrypt_default;
void _aes192_decrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes192_decrypt_default;
void _aes256_decrypt(const uint32_t*, uint8_t*, const uint8_t*, size_t)
		= &_crypto_aes256_decrypt_default;

#endif

