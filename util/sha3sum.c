/*
 * sha3sum.c
 *
 *  Created on: Sep 12, 2019, 10:23:27 AM
 *      Author: Joshua Fehrenbach
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define __CRYPTO_LIBCRYPTO_DLL 0

#include "sha3.h"

#undef __CRYPTO_DECLSPEC
#define __CRYPTO_DECLSPEC __CRYPTO_DECLSPEC_IMPORT

#include "util.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

const char help_string[] =
"sha3sum [OPTIONS] [file1 file2  file3...]\n"
"Options:\n"
"\t-b            Subsequent hashes are input/output in binary.\n"
"\t-o            Subsequent hashes are input/output in octal.\n"
"\t-x            Subsequent hashes are input/output in hexadecimal (default).\n"
"\t-64           Subsequent hashes are input/output in base 64.\n"
"\t-dnnn         Subsequent hashes are computed using SHA3-nnn, where \'nnn\'\n"
"\t                  is the one of 224, 256, 384, or 512 (default).\n"
"\t-d nnn        Alternative form of -dnnn.\n"
"\t-c file hash  Checks the hash of each file against the expected value.\n"
"\t-C hashfile   Checks the validity of the hashes in the given file.\n"
"\t                  The file provided is expected to have the hashes stored\n"
"\t                  in the same format as is output by sha3sum.\n"
"\t-h            Prints this help message, ignoring all following arguments.\n"
"\n"
"Options are case-sensitive and are processed in order.\n"
"\n"
"For each file given, sha3sum, using the option -dnnn, provides output of the form:\n"
"Name: filename\n"
"SHA3-nnn-Digest: hashvalue\n"
"\n"
"When not using -c or -C, file names can include wildcards. For example,\n"
"sha3sum -d256 *.txt\n"
"will output the SHA3-256 Digest of all files in the current working directory\n"
"that have a .txt extension. In addition, the contents of directories are recursively\n"
"processed as well so, for example,\n"
"sha3sum -d256 *\n"
"will output the SHA3-256 Digest of all files in the current working directory and\n"
"all files in any sub-directories in the current working directory.\n"
"\n"
"In the case that no input file is given, or the file given is '-', sha3sum,\n"
"using the option -dnnn or -d nnn, provides output of the form:\n"
"Name: -\n"
"SHA3-nnn-Digest: hashvalue\n"
"\n"
"If file is \'-\', or no files are given, standard input is used.\n"
"\n"
"Successful completion of execution returns a value of 0.\n"
"Unsuccessful completion will return a negative integer.\n"
"In the case of the -c and/or -C option(s) being used, the number of failed hash\n"
"checks is returned upon successful completion, or 0 if all hash checks pass.\n"
;


struct sha3_224_ctx ctx224;
struct sha3_256_ctx ctx256;
struct sha3_384_ctx ctx384;
struct sha3_512_ctx ctx512;
uint8_t hashval[SHA3_512_DIGEST_SIZE];	/* Max digest size */

struct hash_struct hash = {
	SHA3_512, &ctx512,
	(crypto_hash_update_func*)&sha3_512_update,
	(crypto_hash_digest_func*)&sha3_512_digest,
	hashval, SHA3_512_DIGEST_SIZE
};

const char *algname = "SHA3-512";

const char *ATTRIBUTE(const) get_algname(hash_alg alg) {
	switch (alg) {
		case SHA3_224: return "SHA3-224";
		case SHA3_256: return "SHA3-256";
		case SHA3_384: return "SHA3-384";
		case SHA3_512: return "SHA3-512";
		default: return NULL;
	}
}


void optb() {
	encoder = &binary;
}

void opto() {
	encoder = &octal;
}

void optx() {
	encoder = &hexadecimal;
}

void opt64() {
	encoder = &base64;
}

void optd(const char *d) {
	if (*d == '\0') {
		fputs("No argument given for -d option\n", stderr);
		exit(-1);
	} else if (strcmp(d, "224") == 0) {
		sha3_224_init(&ctx224);
		hash.alg = SHA3_224;
		hash.ctx = &ctx224;
		hash.update = (crypto_hash_update_func*)&sha3_224_update;
		hash.digest = (crypto_hash_digest_func*)&sha3_224_digest;
		hash.digest_size = SHA3_224_DIGEST_SIZE;
	} else if (strcmp(d, "256") == 0) {
		sha3_256_init(&ctx256);
		hash.alg = SHA3_256;
		hash.ctx = &ctx256;
		hash.update = (crypto_hash_update_func*)&sha3_256_update;
		hash.digest = (crypto_hash_digest_func*)&sha3_256_digest;
		hash.digest_size = SHA3_256_DIGEST_SIZE;
	} else if (strcmp(d, "384") == 0) {
		sha3_384_init(&ctx384);
		hash.alg = SHA3_384;
		hash.ctx = &ctx384;
		hash.update = (crypto_hash_update_func*)&sha3_384_update;
		hash.digest = (crypto_hash_digest_func*)&sha3_384_digest;
		hash.digest_size = SHA3_384_DIGEST_SIZE;
	} else if (strcmp(d, "512") == 0) {
		sha3_512_init(&ctx512);
		hash.alg = SHA3_512;
		hash.ctx = &ctx512;
		hash.update = (crypto_hash_update_func*)&sha3_512_update;
		hash.digest = (crypto_hash_digest_func*)&sha3_512_digest;
		hash.digest_size = SHA3_512_DIGEST_SIZE;
	} else {
		fprintf(stderr, "Invalid argument for -d option: %s;"
				" must be one of 224, 256, 384, or 512\n", d);
		exit (-1);
	}
	algname = get_algname(hash.alg);
}

void optdalt(int argc, const char *argv[], int i) {
	if (i == argc-1) {
		fputs("No argument given for -d option\n", stderr);
		exit(-1);
	}
	optd(argv[i+1]);
}


void hash_stdin() {
	hash_filep(&hash, stdin);
	print_file_hash(&hash, "-");
}

int check_hash(const char *filename, const char *oldhash) {
	uint8_t decoded[64];
	FILE *in;
	struct stat st;
	if (encoder->decoded_size((const unsigned char*)oldhash) != hash.digest_size) {
		fprintf(stderr, "Invalid %s %s Digest: %s\n",
				encoder->name, algname, oldhash);
		exit(-1);
	}
	encoder->decode_func(decoded, (const unsigned char*)oldhash);
	/* Can't call hash_file here, because the input file here must not be a directory */
	if (stat(filename, &st)) {
		/* Could not read file attributes */
check_hash_err:
		fprintf(stderr, "File \"%s\" can't be opened\n", filename);
		exit(-1);
	} else if (S_ISDIR(st.st_mode)) {
		/* Input file is a directory */
		fprintf(stderr, "Cannot check the hash of \"%s\"; file is a directory\n", filename);
		exit(-1);
	}
	in = fopen(filename, "rb");
	if (in == NULL) {
		goto check_hash_err;
	}
	hash_filep(&hash, in);
	fclose(in);
	return memcmp(hashval, decoded, hash.digest_size) != 0;
}


int optc(int argc, const char *argv[], int i) {
	if (i >= argc-2) {
		fputs("No file and/or hash given for -c option\n", stderr);
		exit(-1);
	}
	if (check_hash(argv[i+1], argv[i+2])) {
		printf("Hash check failure for \"%s\"\n", argv[i+1]);
		printf("%s != ", argv[i+2]); print_hash(&hash); puts("\n");
		return 1;
	}
	return 0;
}

int optC(int argc, const char *argv[], int i) {
	char filename[FILENAME_MAX+1];
	char hashcheck[SHA3_512_DIGEST_SIZE*8+1];	/* Max length (in case of binary format) */
	FILE *checkfile;
	long fp;
	int ret;
	if (i == argc-1) {
		fputs("No file given for -C option\n", stderr);
		exit(-1);
	}
	checkfile = fopen(argv[i+1], "r");
	if (checkfile == NULL) {
		fprintf(stderr, "Hash check file \"%s\" can't be opened\n", argv[i+1]);
		exit(-1);
	}
	ret = 0;
	while (1) {
		fp = ftell(checkfile);
		if (fp == -1L) {
			break;
		}
		do {
			if (!get_hash_check(checkfile, argv[i+1], hash.alg, filename, hashcheck)) {
				goto optC_end;
			}
		} while (fp == ftell(checkfile));	/* Ensure that we actually read something */
		if (encoder->decoded_size((const unsigned char*)hashcheck) != hash.digest_size) {
			fprintf(stderr, "Invalid %s Digest in hash check file \"%s\";"
					" skipping the remainder of the file\n", algname, argv[i+1]);
			break;
		}
		if (check_hash(filename, hashcheck)) {
			printf("Hash check failure for \"%s\"\n", filename);
			printf("%s != ", hashcheck); print_hash(&hash); puts("\n");
			ret++;
		}
	}
optC_end:
	fclose(checkfile);
	return ret;
}

void opth() {
	fputs(help_string, stdout);
}


int main(int argc, char *argv[]) {
	int ret;
	encoder = &hexadecimal;
	sha3_512_init(&ctx512);
	if (argc == 1) {
		/* No arguments, so hash standard input */
		hash_stdin();
		return 0;
	}
	ret = 0;
	for (int i = 1; i < argc; i++) {
		if (strlen(argv[i]) == 0) {
			/* empty argument, so skip it */
			continue;
		}
		if (argv[i][0] != '-') {
			/* argument is a filename */
			hash_file(&hash, argv[i]);
		} else {
			/* argument is an option */
			if (strlen(argv[i]) == 1) {
				/* hash standard input */
				hash_stdin();
				continue;
			}
			if (strlen(argv[i]) == 2) {
				switch (argv[i][1]) {
					/* Set to binary input/output */
					case 'b': optb(); continue;
					/* Set to octal input/output */
					case 'o': opto(); continue;
					/* Set to hexadecimal input/output */
					case 'x': optx(); continue;
					/* Set the algorithm version */
					case 'd': optdalt(argc, (const char**)argv, i); i += 1; continue;
					/* Check a file's hash */
					case 'c': ret += optc(argc, (const char**)argv, i); i += 2; continue;
					/* Check the hashes of many files */
					case 'C': ret += optC(argc, (const char**)argv, i); i += 1; continue;
					/* Print the help message and return */
					case 'h': opth(); return ret;
				}
			} else if (argv[i][1] == '6' && argv[i][2] == '4' && argv[i][3] == '\0') {
				/* Set to base64 input/output */
				opt64();
				continue;
			} else if (argv[i][1] == 'd') {
				/* Set the algorithm version */
				optd(argv[i] + 2);
				continue;
			}
			fprintf(stderr, "Invalid Option: %s\n", argv[i]);
			return -1;
		}
	}

	return ret;
}

