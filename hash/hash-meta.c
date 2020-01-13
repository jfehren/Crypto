/*
 * hash-meta.c
 *
 *  Created on: Sep 3, 2019, 1:09:26 PM
 *      Author: Joshua Fehrenbach
 */

#include "crypto-meta.h"
#include "md2.h"
#include "md4.h"
#include "md5.h"
#include "md6.h"
#include "sha1.h"
#include "sha2.h"
#include "sha3.h"
#include <stddef.h>

const struct crypto_hash_meta crypto_md2 = {
	"md2", sizeof(struct md2_ctx), MD2_DIGEST_SIZE, MD2_BLOCK_SIZE,
	&md2_init, &md2_update, &md2_digest
};

const struct crypto_hash_meta crypto_md4 = {
	"md4", sizeof(struct md4_ctx), MD4_DIGEST_SIZE, MD4_BLOCK_SIZE,
	&md4_init, &md4_update, &md4_digest
};

const struct crypto_hash_meta crypto_md5 = {
	"md5", sizeof(struct md5_ctx), MD5_DIGEST_SIZE, MD5_BLOCK_SIZE,
	&md5_init, &md5_update, &md5_digest
};

const struct crypto_hash_meta crypto_md6_224 = {
	"md6-224", sizeof(struct md6_ctx), MD6_224_DIGEST_SIZE, MD6_224_BLOCK_SIZE,
	&md6_224_init, &md6_update, &md6_digest
};

const struct crypto_hash_meta crypto_md6_256 = {
	"md6-256", sizeof(struct md6_ctx), MD6_256_DIGEST_SIZE, MD6_256_BLOCK_SIZE,
	&md6_256_init, &md6_update, &md6_digest
};

const struct crypto_hash_meta crypto_md6_384 = {
	"md6-384", sizeof(struct md6_ctx), MD6_384_DIGEST_SIZE, MD6_384_BLOCK_SIZE,
	&md6_384_init, &md6_update, &md6_digest
};

const struct crypto_hash_meta crypto_md6_512 = {
	"md6-512", sizeof(struct md6_ctx), MD6_512_DIGEST_SIZE, MD6_512_BLOCK_SIZE,
	&md6_512_init, &md6_update, &md6_digest
};

const struct crypto_hash_meta crypto_sha1 = {
	"sha1", sizeof(struct sha1_ctx), SHA1_DIGEST_SIZE, SHA1_BLOCK_SIZE,
	&sha1_init, &sha1_update, &sha1_digest
};

const struct crypto_hash_meta crypto_sha224 = {
	"sha224", sizeof(struct sha224_ctx), SHA224_DIGEST_SIZE, SHA224_BLOCK_SIZE,
	&sha224_init, &sha224_update, &sha224_digest
};

const struct crypto_hash_meta crypto_sha256 = {
	"sha256", sizeof(struct sha256_ctx), SHA256_DIGEST_SIZE, SHA256_BLOCK_SIZE,
	&sha256_init, &sha256_update, &sha256_digest
};

const struct crypto_hash_meta crypto_sha384 = {
	"sha384", sizeof(struct sha384_ctx), SHA384_DIGEST_SIZE, SHA384_BLOCK_SIZE,
	&sha384_init, &sha384_update, &sha384_digest
};

const struct crypto_hash_meta crypto_sha512 = {
	"sha512", sizeof(struct sha512_ctx), SHA512_DIGEST_SIZE, SHA512_BLOCK_SIZE,
	&sha512_init, &sha512_update, &sha512_digest
};

const struct crypto_hash_meta crypto_sha512_224 = {
	"sha512/224", sizeof(struct sha512_224_ctx), SHA512_224_DIGEST_SIZE, SHA512_224_BLOCK_SIZE,
	&sha512_224_init, &sha512_224_update, &sha512_224_digest
};

const struct crypto_hash_meta crypto_sha512_256 = {
	"sha512/256", sizeof(struct sha512_256_ctx), SHA512_256_DIGEST_SIZE, SHA512_256_BLOCK_SIZE,
	&sha512_256_init, &sha512_256_update, &sha512_256_digest
};

const struct crypto_hash_meta crypto_sha3_224 = {
	"sha3-224", sizeof(struct sha3_224_ctx), SHA3_224_DIGEST_SIZE, SHA3_224_BLOCK_SIZE,
	&sha3_224_init, &sha3_224_update, &sha3_224_digest
};

const struct crypto_hash_meta crypto_sha3_256 = {
	"sha3-256", sizeof(struct sha3_256_ctx), SHA3_256_DIGEST_SIZE, SHA3_256_BLOCK_SIZE,
	&sha3_256_init, &sha3_256_update, &sha3_256_digest
};

const struct crypto_hash_meta crypto_sha3_384 = {
	"sha3-384", sizeof(struct sha3_384_ctx), SHA3_384_DIGEST_SIZE, SHA3_384_BLOCK_SIZE,
	&sha3_384_init, &sha3_384_update, &sha3_384_digest
};

const struct crypto_hash_meta crypto_sha3_512 = {
	"sha3-512", sizeof(struct sha3_512_ctx), SHA3_512_DIGEST_SIZE, SHA3_512_BLOCK_SIZE,
	&sha3_512_init, &sha3_512_update, &sha3_512_digest
};


const struct crypto_hash_meta * const crypto_hashes[] = {
	&crypto_md2,
	&crypto_md4,
	&crypto_md5,
	&crypto_md6_224,
	&crypto_md6_256,
	&crypto_md6_384,
	&crypto_md6_512,
	&crypto_sha1,
	&crypto_sha224,
	&crypto_sha256,
	&crypto_sha384,
	&crypto_sha512,
	&crypto_sha512_224,
	&crypto_sha512_256,
	&crypto_sha3_224,
	&crypto_sha3_256,
	&crypto_sha3_384,
	&crypto_sha3_512,
	NULL
};
