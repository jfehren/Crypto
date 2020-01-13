/*
 * pkcs1.c
 *
 *  Created on: Nov 5, 2019, 9:53:32 AM
 *      Author: Joshua Fehrenbach
 */

#include <assert.h>
#include <string.h>

#include "pkcs1.h"
#include "crypto-gmp.h"
#include "tmp-alloc.h"

int
pkcs1_encrypt(mpz_ptr m, size_t key_size, const uint8_t *src, size_t length,
		void *random_ctx, crypto_random_func *random) {
	size_t padding, i;
	TMP_DECL(em, uint8_t);

	/* The message is encoded as a string of the same length as the
	 * modulo n, of the form
	 *
	 *   00 02 pad 00 message
	 *
	 * where padding should be at least 8 pseudo-randomly generated
	 * *non-zero* octets. */
	if (length + 11 > key_size) {
		/* Message too long for this key */
		return 0;
	}

	/* At least 8 octets of random padding */
	padding = key_size - length - 3;
	assert (padding >= 8);

	/* leading 0 octet is redundant and will be stripped out by
	 * crypto_mpz_set_str_256_u because it reads the input as
	 * a big-endian byte-string */
	TMP_ALLOC(em, key_size - 1);
	em[0] = 2;

	/* generate random bytes */
	(*random)(random_ctx, em + 1, padding);

	/* Replace 0-octets with 1-octets */
	for (i = 2; i < padding+1; i++) {
		if (!em[i]) {
			em[i] = 1;
		}
	}

	em[padding+1] = 0;
	memcpy(em + padding + 2, src, length);

	crypto_mpz_set_str_256_u(m, em, key_size - 1);

	TMP_FREE(em);
	return 1;
}

int
pkcs1_decrypt(uint8_t *dst, size_t *length, mpz_srcptr m, size_t key_size) {
	size_t padding, msglen;
	uint8_t *term;
	int ret;
	TMP_DECL(em, uint8_t);

	TMP_ALLOC(em, key_size);
	crypto_mpz_get_str_256(em, key_size, m);

	/* check input format */
	if (em[0] || em[1] != 2) {
		ret = 0;
		goto cleanup;
	}
	term = memchr(em + 2, 0, key_size - 2);
	if (term == NULL) {
		ret = 0;
		goto cleanup;
	}
	padding = term - (em + 2);
	if (padding < 8) {
		ret = 0;
		goto cleanup;
	}

	msglen = key_size - 3 - padding;
	if (*length < msglen) {
		ret = 0;
		goto cleanup;
	}

	memcpy(dst, term + 1, msglen);
	*length = msglen;

	ret = 1;
cleanup:
	TMP_FREE(em);
	return ret;
}

/* Formats the PKCS#1 padding, of the form
 *
 *   0x00 0x01 0xff ... 0xff 0x00 id ...digest...
 *
 * where the 0xff ... 0xff part consists of at least 8 octets. The
 * total size equals the octet size of n.
 */
static inline uint8_t *
_pkcs1_signature_prefix(uint8_t *buffer, unsigned key_size,
		const uint8_t *id, unsigned id_size, unsigned digest_size) {
	unsigned j;
	if (key_size < 11 + id_size + digest_size) {
		return NULL;
	}
	j = key_size - digest_size - id_size;

	memcpy(buffer + j, id, id_size);
	buffer[0] = 0;
	buffer[1] = 1;
	buffer[j-1] = 0;

	assert(j >= 11);
	memset(buffer + 2, 0xff, j - 3);

	return buffer + j + id_size;
}

int
pkcs1_rsa_digest_encode(mpz_ptr m, size_t key_size,
		const uint8_t *digest, size_t length) {
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	if (_pkcs1_signature_prefix(em, key_size, digest, length, 0)) {
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   md2 OBJECT IDENTIFIER ::=
 *     {iso(1) member-body(2) US(840) rsadsi(113549) digestAlgorithm(2) 2}
 */
static const uint8_t
md2_prefix[] = {
	/* 18 octets prefix, 16 octets hash, 34 octets total */
	0x30, 32,	/* SEQUENCE */
		0x30, 12,	/* SEQUENCE */
			0x06, 8,	/* OBJECT IDENTIFIER */
				0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x02,
			0x05, 0x00, /* NULL */
		0x04, 16	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_md2_encode(mpz_ptr m, size_t key_size, struct md2_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md2_prefix, sizeof(md2_prefix),
			MD2_DIGEST_SIZE);

	if (p != NULL) {
		md2_digest(hash, p, MD2_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_md2_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md2_prefix, sizeof(md2_prefix),
			MD2_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, MD2_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 2313, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 1.5
 *
 *   md4 OBJECT IDENTIFIER ::=
 *     {iso(1) member-body(2) US(840) rsadsi(113549) digestAlgorithm(2) 4}
 */
static const uint8_t
md4_prefix[] = {
	/* 18 octets prefix, 16 octets hash, 34 octets total */
	0x30, 32,	/* SEQUENCE */
		0x30, 12,	/* SEQUENCE */
			0x06, 8,	/* OBJECT IDENTIFIER */
				0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x04,
			0x05, 0x00, /* NULL */
		0x04, 16	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_md4_encode(mpz_ptr m, size_t key_size, struct md4_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md4_prefix, sizeof(md4_prefix),
			MD4_DIGEST_SIZE);

	if (p != NULL) {
		md4_digest(hash, p, MD4_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_md4_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md4_prefix, sizeof(md4_prefix),
			MD4_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, MD4_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   md5 OBJECT IDENTIFIER ::=
 *     {iso(1) member-body(2) US(840) rsadsi(113549) digestAlgorithm(2) 5}
 */
static const uint8_t
md5_prefix[] = {
	/* 18 octets prefix, 16 octets hash, 34 octets total */
	0x30, 32,	/* SEQUENCE */
		0x30, 12,	/* SEQUENCE */
			0x06, 8,	/* OBJECT IDENTIFIER */
				0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x02, 0x05,
			0x05, 0x00, /* NULL */
		0x04, 16	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_md5_encode(mpz_ptr m, size_t key_size, struct md5_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md5_prefix, sizeof(md5_prefix),
			MD5_DIGEST_SIZE);

	if (p != NULL) {
		md5_digest(hash, p, MD5_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_md5_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			md5_prefix, sizeof(md5_prefix),
			MD5_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, MD5_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha1 OBJECT IDENTIFIER ::=
 *     {iso(1) identified-organization(3) oiw(14) secsig(3)
 *   	 algorithms(2) 26}
 */
static const uint8_t
sha1_prefix[] = {
	/* 15 octets prefix, 20 octets hash, 35 octets total */
	0x30, 33,	/* SEQUENCE */
		0x30, 9,	/* SEQUENCE */
			0x06, 5,	/* OBJECT IDENTIFIER */
				0x2b, 0x0e, 0x03, 0x02, 0x1a,
			0x05, 0x00, /* NULL */
		0x04, 20	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha1_encode(mpz_ptr m, size_t key_size, struct sha1_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha1_prefix, sizeof(sha1_prefix),
			SHA1_DIGEST_SIZE);

	if (p != NULL) {
		sha1_digest(hash, p, SHA1_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha1_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha1_prefix, sizeof(sha1_prefix),
			SHA1_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA1_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha224 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 4}
 */
static const uint8_t
sha224_prefix[] = {
	/* 19 octets prefix, 28 octets hash, 47 octets total */
	0x30, 45,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x04,
			0x05, 0x00,	/* NULL */
		0x04, 28	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha224_encode(mpz_ptr m, size_t key_size, struct sha224_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha224_prefix, sizeof(sha224_prefix),
			SHA224_DIGEST_SIZE);

	if (p != NULL) {
		sha224_digest(hash, p, SHA224_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha224_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha224_prefix, sizeof(sha224_prefix),
			SHA224_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA224_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha256 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 1}
 */
static const uint8_t
sha256_prefix[] = {
	/* 19 octets prefix, 32 octets hash, 51 octets total */
	0x30, 49,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x01,
			0x05, 0x00,	/* NULL */
		0x04, 32	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha256_encode(mpz_ptr m, size_t key_size, struct sha256_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha256_prefix, sizeof(sha256_prefix),
			SHA256_DIGEST_SIZE);

	if (p != NULL) {
		sha256_digest(hash, p, SHA256_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha256_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha256_prefix, sizeof(sha256_prefix),
			SHA256_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA256_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha384 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 2}
 */
static const uint8_t
sha384_prefix[] = {
	/* 19 octets prefix, 48 octets hash, 67 octets total */
	0x30, 65,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x02,
			0x05, 0x00,	/* NULL */
		0x04, 48	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha384_encode(mpz_ptr m, size_t key_size, struct sha384_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha384_prefix, sizeof(sha384_prefix),
			SHA384_DIGEST_SIZE);

	if (p != NULL) {
		sha384_digest(hash, p, SHA384_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha384_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha384_prefix, sizeof(sha384_prefix),
			SHA384_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA384_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha512 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 3}
 */
static const uint8_t
sha512_prefix[] = {
	/* 19 octets prefix, 64 octets hash, 83 octets total */
	0x30, 81,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x03,
			0x05, 0x00,	/* NULL */
		0x04, 64	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha512_encode(mpz_ptr m, size_t key_size, struct sha512_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_prefix, sizeof(sha512_prefix),
			SHA512_DIGEST_SIZE);

	if (p != NULL) {
		sha512_digest(hash, p, SHA512_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha512_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_prefix, sizeof(sha512_prefix),
			SHA512_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA512_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha512-224 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 5}
 */
static const uint8_t
sha512_224_prefix[] = {
	/* 19 octets prefix, 28 octets hash, 47 octets total */
	0x30, 45,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x05,
			0x05, 0x00,	/* NULL */
		0x04, 28	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha512_224_encode(mpz_ptr m, size_t key_size, struct sha512_224_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_224_prefix, sizeof(sha512_224_prefix),
			SHA512_224_DIGEST_SIZE);

	if (p != NULL) {
		sha512_224_digest(hash, p, SHA512_224_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha512_224_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_224_prefix, sizeof(sha512_224_prefix),
			SHA512_224_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA512_224_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

/* From RFC 8017, Public-Key Cryptography Standards (PKCS) #1: RSA
 * Cryptography Specifications Version 2.2.
 *
 *   id-sha512-256 OBJECT IDENTIFIER ::=
 *     {joint-iso-itu-t(2) country(16) US(840) organization(1) gov(101)
 *       csor(3) nistalgorithm(4) hashalgs(2) 6}
 */
static const uint8_t
sha512_256_prefix[] = {
	/* 19 octets prefix, 32 octets hash, 51 octets total */
	0x30, 49,	/* SEQUENCE */
		0x30, 13,	/* SEQUENCE */
			0x06, 9,	/* OBJECT IDENTIFIER */
				0x60, 0x86, 0x48, 0x01, 0x65, 0x03, 0x04, 0x02, 0x06,
			0x05, 0x00,	/* NULL */
		0x04, 32	/* OCTET STRING */
			/* raw hash value follows */
};

int
pkcs1_rsa_sha512_256_encode(mpz_ptr m, size_t key_size, struct sha512_256_ctx *hash) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_256_prefix, sizeof(sha512_256_prefix),
			SHA512_256_DIGEST_SIZE);

	if (p != NULL) {
		sha512_256_digest(hash, p, SHA512_256_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

int
pkcs1_rsa_sha512_256_encode_digest(mpz_ptr m, size_t key_size, const uint8_t *digest) {
	uint8_t *p;
	TMP_DECL(em, uint8_t);
	TMP_ALLOC(em, key_size);

	p = _pkcs1_signature_prefix(em, key_size,
			sha512_256_prefix, sizeof(sha512_256_prefix),
			SHA512_256_DIGEST_SIZE);

	if (p != NULL) {
		memcpy(p, digest, SHA512_256_DIGEST_SIZE);
		crypto_mpz_set_str_256_u(m, em, key_size);
		TMP_FREE(em);
		return 1;
	} else {
		TMP_FREE(em);
		return 0;
	}
}

