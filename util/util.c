/*
 * util.c
 *
 *  Created on: Sep 5, 2019, 11:46:19 AM
 *      Author: Joshua Fehrenbach
 */

#include "util.h"
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

static const unsigned char * const binary_alphabet = (const unsigned char*)"01";
static const unsigned char * const octal_alphabet = (const unsigned char*)"01234567";
static const unsigned char * const hex_alphabet = (const unsigned char*)
		"0123456789abcdefABCDEF";
static const unsigned char * const base64_alphabet = (const unsigned char*)
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"abcdefghijklmnopqrstuvwxyz"
		"0123456789+/";

#define XX 0xff

static const unsigned char octal_table[64] = {
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0,  1,  2,  3,  4,  5,  6,  7, XX, XX, XX, XX, XX, XX, XX, XX
};
static const unsigned char hex_table[128] = {
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, XX, XX, XX, XX, XX, XX,
	XX, 16, 17, 18, 19, 20, 21, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, 10, 11, 12, 13, 14, 15, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX
};
static const unsigned char base64_table[128] = {
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX,
	XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, XX, 62, XX, XX, XX, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, XX, XX, XX, XX, XX, XX,
	XX,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, XX, XX, XX, XX, XX,
	XX, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, XX, XX, XX, XX, XX
};

#undef XX


size_t binary_encode(unsigned char *dst, const uint8_t *src, size_t length) {
	size_t ret = length*8;
	uint8_t x;
	while (length-- != 0) {
		x = *src++;
		dst[0] = (x & 0x80) ? '1' : '0';
		dst[1] = (x & 0x40) ? '1' : '0';
		dst[2] = (x & 0x20) ? '1' : '0';
		dst[3] = (x & 0x10) ? '1' : '0';
		dst[4] = (x & 0x08) ? '1' : '0';
		dst[5] = (x & 0x04) ? '1' : '0';
		dst[6] = (x & 0x02) ? '1' : '0';
		dst[7] = (x & 0x01) ? '1' : '0';
		dst += 8;
	}
	*dst = '\0';
	return ret;
}

size_t octal_encode(unsigned char *dst, const uint8_t *src, size_t length) {
	size_t ret = (length/3)*8;
	if (length % 3 == 2) {
		dst[0] = octal_alphabet[(src[0] >> 7) & 0x1];
		dst[1] = octal_alphabet[(src[0] >> 4) & 0x7];
		dst[2] = octal_alphabet[(src[0] >> 1) & 0x7];
		dst[3] = octal_alphabet[((src[0] << 2) & 0x4) | ((src[1] >> 6) & 0x3)];
		dst[4] = octal_alphabet[(src[1] >> 3) & 0x7];
		dst[5] = octal_alphabet[(src[1] >> 0) & 0x7];
		dst += 6;
		src += 2;
		length -= 2;
		ret += 6;
	} else if (length % 3 == 1) {
		dst[0] = octal_alphabet[(src[0] >> 6) & 0x3];
		dst[1] = octal_alphabet[(src[0] >> 3) & 0x7];
		dst[2] = octal_alphabet[(src[0] >> 0) & 0x7];
		dst += 3;
		src++;
		length--;
		ret += 3;
	}
	length /= 3;
	while (length-- != 0) {
		dst[0] = octal_alphabet[(src[0] >> 5) & 0x7];
		dst[1] = octal_alphabet[(src[0] >> 2) & 0x7];
		dst[2] = octal_alphabet[((src[0] << 1) & 0x6) | ((src[1] >> 7) & 0x1)];
		dst[3] = octal_alphabet[(src[1] >> 4) & 0x7];
		dst[4] = octal_alphabet[(src[1] >> 1) & 0x7];
		dst[5] = octal_alphabet[((src[1] << 2) & 0x4) | ((src[2] >> 6) & 0x3)];
		dst[6] = octal_alphabet[(src[2] >> 3) & 0x7];
		dst[7] = octal_alphabet[(src[2] >> 0) & 0x7];
		dst += 8;
		src += 3;
	}
	*dst = '\0';
	return ret;
}

size_t hex_encode(unsigned char *dst, const uint8_t *src, size_t length) {
	size_t ret = length*2;
	uint8_t x;
	while (length-- != 0) {
		x = *src++;
		dst[0] = hex_alphabet[(x >> 4) & 0xf];
		dst[1] = hex_alphabet[(x >> 0) & 0xf];
		dst += 2;
	}
	*dst = '\0';
	return ret;
}

size_t base64_encode(unsigned char *dst, const uint8_t *src, size_t length) {
	size_t ret = ((length+2)/3)*4;
	size_t left = length % 3;
	length /= 3;
	while (length-- != 0) {
		dst[0] = base64_alphabet[(src[0] >> 2) & 0x3f];
		dst[1] = base64_alphabet[((src[0] << 4) & 0x30) | ((src[1] >> 4) & 0x0f)];
		dst[2] = base64_alphabet[((src[1] << 2) & 0x3c) | ((src[2] >> 6) & 0x03)];
		dst[3] = base64_alphabet[src[2] & 0x3f];
		dst += 4;
		src += 3;
	}
	if (left > 0) {
		dst[0] = base64_alphabet[(src[0] >> 2) & 0x3f];
		if (left == 1) {
			dst[1] = base64_alphabet[(src[0] << 4) & 0x3f];
			dst[2] = '=';
		} else {
			dst[1] = base64_alphabet[((src[0] << 4) & 0x30) | ((src[1] >> 4) & 0x0f)];
			dst[2] = base64_alphabet[(src[1] << 2) & 0x3f];
		}
		dst[3] = '=';
		dst += 4;
	}
	*dst = '\0';
	return ret;
}


size_t binary_decode(uint8_t *dst, const unsigned char *src) {
	uint8_t x;
	size_t length = strspn((const char*)src, (const char *)binary_alphabet);
	size_t ret = (length+7)/8;
	if (length % 8 != 0) {
		x = 0;
		while (length % 8 != 0) {
			x <<= 1;
			if (*src++ == '1') {
				x |= 1;
			}
			length--;
		}
		*dst++ = x;
	}
	length /= 8;
	while (length-- != 0) {
		x  = (src[0] == '1') << 7;
		x |= (src[1] == '1') << 6;
		x |= (src[2] == '1') << 5;
		x |= (src[3] == '1') << 4;
		x |= (src[4] == '1') << 3;
		x |= (src[5] == '1') << 2;
		x |= (src[6] == '1') << 1;
		x |= (src[7] == '1') << 0;
		*dst++ = x;
		src += 8;
	}
	return ret;
}

size_t octal_decode(uint8_t *dst, const unsigned char *src) {
	size_t length = strspn((const char *)src, (const char *)octal_alphabet);
	size_t ret = (length/8)*3;
	if (length % 8 != 0) {
		uint8_t x, y, z;
		if (length % 8 > 6 || (length % 8 == 6 && octal_table[*src] > 1)) {
			x = y = z = 0;
			while (length % 8 != 0) {
				x = ((x << 3) & 0xf8) | ((y >> 5) & 0x07);
				y = ((y << 3) & 0xf8) | ((z >> 5) & 0x07);
				z = ((z << 3) & 0xf8) | (octal_table[*src++] & 0x07);
				length--;
			}
			dst[0] = x;
			dst[1] = y;
			dst[2] = z;
			dst += 3;
			ret += 3;
		} else if (length % 8 > 3 || (length % 8 == 3 && octal_table[*src] > 3)) {
			x = y = 0;
			while (length % 8 != 0) {
				x = ((x << 3) & 0xf8) | ((y >> 5) & 0x07);
				y = ((y << 3) & 0xf8) | (octal_table[*src++] & 0x07);
				length--;
			}
			dst[0] = x;
			dst[1] = y;
			dst += 2;
			ret += 2;
		} else {
			x = 0;
			while (length % 8 != 0) {
				x = ((x << 3) & 0xf8) | (octal_table[*src++] & 0x07);
				length--;
			}
			*dst++ = x;
			ret++;
		}
	}
	length /= 8;
	while (length-- != 0) {
		dst[0] =	((octal_table[src[0]] << 5) & 0xe0) |
					((octal_table[src[1]] << 2) & 0x1c) |
					((octal_table[src[2]] >> 1) & 0x03);
		dst[1] =	((octal_table[src[2]] << 7) & 0x80) |
					((octal_table[src[3]] << 4) & 0x70) |
					((octal_table[src[4]] << 1) & 0x0e) |
					((octal_table[src[5]] >> 2) & 0x01);
		dst[2] =	((octal_table[src[5]] << 6) & 0xc0) |
					((octal_table[src[6]] << 3) & 0x38) |
					((octal_table[src[7]] << 0) & 0x07);
	}
	return ret;
}

size_t hex_decode(uint8_t *dst, const unsigned char *src) {
	size_t length = strspn((const char *)src, (const char *)hex_alphabet);
	size_t ret = (length+1)/2;
	if (length % 2 != 0) {
		*dst++ = hex_table[*src++] & 0x0f;
	}
	length /= 2;
	while (length-- != 0) {
		*dst++ = ((hex_table[src[0]] << 4) & 0xf0) | (hex_table[src[1]] & 0x0f);
		src += 2;
	}
	return ret;
}

size_t base64_decode(uint8_t *dst, const unsigned char *src) {
	size_t length = strspn((const char *)src, (const char *)base64_alphabet);
	size_t ret = (length/4)*3;
	if (length % 4 == 1) {
		fprintf(stderr, "Invalid Base64 String; Cannot have a length of 1 (mod 4)\n");
		exit(-1);
	} else if (length % 4 != 0) {
		ret += (length % 4) - 1;	/* 2 or 3 extra digits give 1 or 2 extra byte(s) */
	}
	while (length >= 4) {
		dst[0] = ((base64_table[src[0]] << 2) & 0xfc) | ((base64_table[src[1]] >> 4) & 0x03);
		dst[1] = ((base64_table[src[1]] << 4) & 0xf0) | ((base64_table[src[2]] >> 2) & 0x0f);
		dst[2] = ((base64_table[src[2]] << 6) & 0xc0) | (base64_table[src[3]] & 0x3f);
		dst += 3;
		src += 4;
		length -= 4;
	}
	if (length > 0) {
		dst[0] = ((base64_table[src[0]] << 2) & 0xfc) | ((base64_table[src[1]] >> 4) & 0x03);
		if (length == 3) {
			dst[1] = ((base64_table[src[1]] << 4) & 0xf0) |
					((base64_table[src[2]] >> 2) & 0x0f);
		}
	}
	return ret;
}


size_t binary_encode_size(size_t bytes) {
	return 8*bytes;
}

size_t octal_encode_size(size_t bytes) {
	return ((bytes+2)/3)*8;
}

size_t hex_encode_size(size_t bytes) {
	return 2*bytes;
}

size_t base64_encode_size(size_t bytes) {
	return ((bytes+2)/3)*4;
}


size_t binary_decode_size(const unsigned char *src) {
	return (strspn((const char *)src, (const char *)binary_alphabet)+7)/8;
}

size_t octal_decode_size(const unsigned char *src) {
	size_t length = strspn((const char *)src, (const char *)octal_alphabet);
	if (length % 8 > 6 || (length % 8 == 6 && octal_table[*src] > 1)) {
		return 3 + (length/8)*3;
	} else if (length % 8 > 3 || (length % 8 == 3 && octal_table[*src] > 3)) {
		return 2 + (length/8)*3;
	} else if (length % 8 != 0) {
		return 1 + (length/8)*3;
	} else {
		return (length/8)*3;
	}
}

size_t hex_decode_size(const unsigned char *src) {
	return (strspn((const char *)src, (const char *)hex_alphabet)+1)/2;
}

size_t base64_decode_size(const unsigned char *src) {
	size_t length = strspn((const char *)src, (const char *)base64_alphabet);
	if (length % 4 == 0) {
		return (length/4)*3;
	} else if (length % 4 == 1) {
		fprintf(stderr, "Invalid Base64 String; Cannot have a length of 1 (mod 4)\n");
		exit(-1);
	}
	return (length/4)*3 + (length % 4) - 1;
}


const struct encode_struct binary = {
	"Binary",
	&binary_encode, &binary_decode,
	&binary_encode_size, &binary_decode_size
};
const struct encode_struct octal = {
	"Octal",
	&octal_encode, &octal_decode,
	&octal_encode_size, &octal_decode_size
};
const struct encode_struct hexadecimal = {
	"Hexadecimal",
	&hex_encode, &hex_decode,
	&hex_encode_size, &hex_decode_size
};
const struct encode_struct base64 = {
	"Base64",
	&base64_encode, &base64_decode,
	&base64_encode_size, &base64_decode_size
};

const struct encode_struct *encoder;


void encode_filename(char *dst, const char *src) {
	if (*src == '-') {
		*dst++ = '\\';
		*dst++ = '-';
		src++;
	}
	while (*src != '\0') {
		if (*src == '\\') {
			*dst++ = '\\';
			*dst++ = '\\';
		} else if (*src == '\n') {
			*dst++ = '\\';
			*dst++ = 'n';
		} else {
			*dst++ = *src;
		}
		src++;
	}
	*dst = '\0';
}

void decode_filename(char *dst, const char *src) {
	while (*src != '\0') {
		if (*src == '\\') {
			if (*(src+1) == '\\') {
				*dst++ = '\\';
				src++;
			} else if (*(src+1) == 'n') {
				*dst++ = '\n';
				src++;
			} else if (*(src+1) == '-') {
				*dst++ = '-';
				src++;
			} else {
				*dst++ = *src;
			}
		} else {
			*dst++ = *src;
		}
		src++;
	}
	*dst = '\0';
}


void print_encoded_filename(const char *fn) {
	if (*fn == '-') {
		fputs("\\-", stdout);
		++fn;
	}
	while (*fn != '\0') {
		if (*fn == '\\') {
			fputs("\\\\", stdout);
		} else if (*fn == '\n') {
			fputs("\\n", stdout);
		} else {
			putchar(*fn);
		}
		++fn;
	}
}

void inplace_decode_filename(char *fn) {
	while (*fn != '\0') {
		/* Loop for out == fn (no escaped character found yet, so no copy needed) */
		if (*fn++ == '\\') {
			/* Check for escape sequence, delegating to decode_filename if one is found */
			if (*fn == '\\') {
#if 0
				*(fn-1) = '\\';
#endif
				decode_filename(fn, fn+1);
				return;
			} else if (*fn == 'n') {
				*(fn-1) = '\n';
				decode_filename(fn, fn+1);
				return;
			} else if (*fn == '-') {
				*(fn-1) = '-';
				decode_filename(fn, fn+1);
				return;
			}
		}
	}

}


const char* get_hash_digest_tag(hash_alg alg) {
	/* Array of const-pointers to const chars (array of static strings) */
	static const char *const algname[] = { NULL,
#include "algname.h"
	};

	if (0 < alg && alg <= SHA3_512) {
		return algname[alg];
	} else {
		return NULL;
	}
}

int get_hash_check(FILE *in, const char *in_name, hash_alg alg,
		char *filename, char *hashval) {
	char buffer[FILENAME_MAX*3/2+1];
	char *buffp;
	const char *alg_tag;
	size_t length, t;
	do {
		/* Need to use fgets since the file name may contain whitespace */
		if (fgets(buffer, sizeof(buffer)-1, in) == NULL) {
			return 0;	/* No input, or error occurred */
		}
		length = strlen(buffer);
		buffp = buffer;
		/* Remove Leading Whitespace */
		while (isspace(*buffp) && length > 0) {
			++buffp;
			--length;
		}
	} while (length == 0);	/* Skip blank lines */
	/* Remove Trailing Whitespace */
	while (isspace(buffp[length-1])) {
		/* No need to check length in the loop condition,
		 * since we know this is not a blank line */
		buffp[--length] = '\0';
	}
	/* Remove "Name:" identifier before file name */
	if (memcmp("Name:", buffp, 5) != 0) {
		fprintf(stderr, "Hash check file \"%s\" has invalid format;"
				" skipping the remainder of the file\n", in_name);
		return 0;
	}
	buffp += 5;
	length -= 5;
	/* Remove Leading Whitespace again */
	while (isspace(*buffp) && length > 0) {
		++buffp;
		--length;
	}
	if (length == 0) {
		fprintf(stderr, "Hash check file \"%s\" has invalid format;"
				" skipping the remainder of the file\n", in_name);
		return 0;
	}
	/* Decode file name */
	decode_filename(filename, buffp);

	/* Read line with hash value, recycling the input buffer */
	do {
		if (fgets(buffer, sizeof(buffer)-1, in) == NULL) {
			fprintf(stderr, "Hash check file \"%s\" has invalid format;"
					" skipping the remainder of the file\n", in_name);
			return 0;
		}
		length = strlen(buffer);
		buffp = buffer;
		/* Remove Leading Whitespace */
		while (isspace(*buffp) && length > 0) {
			++buffp;
			--length;
		}
	} while (length == 0);	/* Skip blank lines */
	/* Remove Trailing Whitespace */
	while (isspace(buffp[length-1])) {
		/* No need to check length in the loop condition,
		 * since we know this is not a blank line */
		buffp[--length] = '\0';
	}
	/* Check and remove algorithm identifier before hash value */
	alg_tag = get_hash_digest_tag(alg);
	t = strlen(alg_tag);
	if (memcmp(buffp, alg_tag, t) != 0) {
		fprintf(stderr, "Digest type mismatch in hash check file \"%s\";"
				" skipping the remainder of the file\n", in_name);
		return 0;
	}
	buffp += t + 1;	/* +1 to remove ':' after algorithm identifier */
	length -= t + 1;
	/* Remove Leading Whitespace again */
	while (isspace(*buffp) && length > 0) {
		++buffp;
		--length;
	}
	if (length == 0) {
		fprintf(stderr, "Hash check file \"%s\" has invalid format;"
				" skipping the remainder of the file\n", in_name);
		return 0;
	}
	/* Copy hash value hashval */
	strcpy(hashval, buffp);
	return 1;
}


void hash_dir(struct hash_struct *hash, const char *dirname) {
	struct dirent *ent;
	char *path;
	size_t pathlen;
	DIR *dir = opendir(dirname);
	if (dir == NULL) {
		fprintf(stderr, "Directory \"%s\" can't be opened\n", dirname);
		exit(-1);
	}
	/* Set up a buffer with the directory name and room for an entry's name */
	pathlen = strlen(dirname);
	path = malloc(pathlen + NAME_MAX + 2);
	strcpy(path, dirname);
	path[pathlen++] = '/';
	/* Hash the contents of a directory */
	while ((ent = readdir(dir)) != NULL) {
		if (strcmp(".", ent->d_name) == 0 || strcmp("..", ent->d_name) == 0) {
			/* Skip entry for current and parent directory, if they exist */
			continue;
		}
		/* Send the next entry to hash_file, which calls this function again if the
		 * entry was another directory */
		strcpy(path + pathlen, ent->d_name);	/* Copy the entry's name into the buffer */
		hash_file(hash, path);
	}
	closedir(dir);
	free(path);
}

void hash_file(struct hash_struct *hash, const char *filename) {
	struct stat st;
	/* Check whether the file is an actual file */
	if (stat(filename, &st)) {
		/* Could not read file attributes */
hash_file_err:
		fprintf(stderr, "File \"%s\" can't be opened\n", filename);
		exit(-1);
	}
	if (S_ISDIR(st.st_mode)) {
		/* Argument is a directory, so send to hash_dir */
		hash_dir(hash, filename);
	} else {
		/* Argument is a file */
		FILE *in = fopen(filename, "rb");
		if (in == NULL) {
			goto hash_file_err;
		}
		hash_filep(hash, in);
		print_file_hash(hash, filename);
		fclose(in);
	}
}

void hash_filep(struct hash_struct *hash, FILE *in) {
	size_t bytes;
	char buffer[4096];
	if (in == NULL) {
		fputs("hash_filep has NULL input file pointer\n", stderr);
		return;
	}
	/* Read up to 4KB at a time, and process the read bytes */
	while ((bytes = fread(buffer, 1, 4096, in)) != 0) {
		hash->update(hash->ctx, (uint8_t*)buffer, bytes);
	}
	/* Finalize and retrieve the digest value */
	hash->digest(hash->ctx, hash->hashval, hash->digest_size);
}

void print_file_hash(const struct hash_struct *hash, const char *filename) {
	fputs("Name: ", stdout); print_encoded_filename(filename);
	printf("\n%s: ", get_hash_digest_tag(hash->alg)); print_hash(hash);
	puts("\n");
}

void print_hash(const struct hash_struct *hash) {
	unsigned char tmp[512+1];	/* max of 512 binary digits, plus null-terminator */
	encoder->encode_func(tmp, hash->hashval, hash->digest_size);
	fputs((char*)tmp, stdout);
}

