/*
 * util.h
 *
 *  Created on: Sep 5, 2019, 11:46:34 AM
 *      Author: Joshua Fehrenbach
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "crypto.h"

#define DECLSPEC __CRYPTO_DECLSPEC

#ifdef __cplusplus
extern "C" {
#endif

DECLSPEC size_t binary_encode(unsigned char *dst, const uint8_t *src, size_t length);
DECLSPEC size_t octal_encode(unsigned char *dst, const uint8_t *src, size_t length);
DECLSPEC size_t hex_encode(unsigned char *dst, const uint8_t *src, size_t length);
DECLSPEC size_t base64_encode(unsigned char *dst, const uint8_t *src, size_t length);

DECLSPEC size_t binary_decode(uint8_t *dst, const unsigned char *src);
DECLSPEC size_t octal_decode(uint8_t *dst, const unsigned char *src);
DECLSPEC size_t hex_decode(uint8_t *dst, const unsigned char *src);
DECLSPEC size_t base64_decode(uint8_t *dst, const unsigned char *src);

DECLSPEC size_t binary_encode_size(size_t bytes) ATTRIBUTE(const);
DECLSPEC size_t octal_encode_size(size_t bytes) ATTRIBUTE(const);
DECLSPEC size_t hex_encode_size(size_t bytes) ATTRIBUTE(const);
DECLSPEC size_t base64_encode_size(size_t bytes) ATTRIBUTE(const);

DECLSPEC size_t binary_decode_size(const unsigned char *src) ATTRIBUTE(pure);
DECLSPEC size_t octal_decode_size(const unsigned char *src) ATTRIBUTE(pure);
DECLSPEC size_t hex_decode_size(const unsigned char *src) ATTRIBUTE(pure);
DECLSPEC size_t base64_decode_size(const unsigned char *src) ATTRIBUTE(pure);

struct encode_struct {
	const char *name;
	size_t (*encode_func)(unsigned char*,const uint8_t*,size_t);
	size_t (*decode_func)(uint8_t*,const unsigned char*);

	size_t (*ATTRIBUTE(const) encoded_size)(size_t);
	size_t (*ATTRIBUTE(pure) decoded_size)(const unsigned char*);
};

DECLSPEC extern const struct encode_struct binary;
DECLSPEC extern const struct encode_struct octal;
DECLSPEC extern const struct encode_struct hexadecimal;
DECLSPEC extern const struct encode_struct base64;

DECLSPEC extern const struct encode_struct *encoder;

DECLSPEC void encode_filename(char *dst, const char *src);
DECLSPEC void decode_filename(char *dst, const char *src);

DECLSPEC void print_encoded_filename(const char *filename);
DECLSPEC void inplace_decode_filename(char *filename);

typedef enum hash_alg_enum {
	/* Implicit enums for MD6-nnn (1 <= nnn <= 512) algorithm versions */
	MD6_1 = 1, MD6_256 = 256, MD6_512 = 512,
	MD2, MD4, MD5,
	SHA1, SHA224, SHA256, SHA384, SHA512, SHA512_224,
	SHA512_256, SHA3_224, SHA3_256, SHA3_384, SHA3_512
} hash_alg;

DECLSPEC const char* get_hash_digest_tag(hash_alg alg) ATTRIBUTE(const);

/* Return 1 for success, 0 for failure (either no input, or invalid format) */
DECLSPEC int get_hash_check(FILE *in, const char *in_name, hash_alg alg,
		char *filename, char *hashval);


struct hash_struct {
	hash_alg alg;
	void *ctx;
	crypto_hash_update_func *update;
	crypto_hash_digest_func *digest;
	uint8_t *hashval;
	size_t digest_size;
};


DECLSPEC void hash_dir(struct hash_struct *hash, const char *dirname);

DECLSPEC void hash_file(struct hash_struct *hash, const char *filename);

DECLSPEC void hash_filep(struct hash_struct *hash, FILE *in);

DECLSPEC void print_file_hash(const struct hash_struct *hash, const char *filename);

DECLSPEC void print_hash(const struct hash_struct *hash);

#ifdef __cplusplus
}
#endif

#endif /* UTIL_H_ */
