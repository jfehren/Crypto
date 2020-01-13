/*
 * md6sum.c
 *
 *  Created on: Sep 12, 2019, 2:43:42 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define __CRYPTO_LIBCRYPTO_DLL 0

#include "md6.h"

#undef __CRYPTO_DECLSPEC
#define __CRYPTO_DECLSPEC __CRYPTO_DECLSPEC_IMPORT

#include "util.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

const char help_string[] =
"md6sum [OPTIONS] [file1 file2  file3...]\n"
"Options:\n"
"\t-b            Subsequent hashes are input/output in binary.\n"
"\t-o            Subsequent hashes are input/output in octal.\n"
"\t-x            Subsequent hashes are input/output in hexadecimal (default).\n"
"\t-64           Subsequent hashes are input/output in base 64.\n"
"\t-dnnn         Subsequent hashes are computed using MD6-nnn, where \'nnn\'\n"
"\t                  is an integer such that 1 <= nnn <= 512 (default is 256).\n"
"\t                  Also adjusts the number of rounds r if -r is not used.\n"
"\t-Kxxxx        Set MD6 key to xxxx (length 0 to 64 bytes); format of xxxx\n"
"\t                  is determined by -b, -o, -x or -64 option. Also adjusts the\n"
"\t                  number of rounds r if -r is not used. Note that an alternative\n"
"\t                  form of this option does not exist.\n"
"\t-Lnnn         Set MD6 mode parameter L to nnn, where \'nnn\' is an integer such\n"
"\t                  that 0 <= nnn <= 255 (default is 64).\n"
"\t-rnnn         Set MD6 round parameter r to nnn, where \'nnn\' is an integer\n"
"\t                  such that 1 <= nnn <= 255 (default is 40+(d/4)). Using the\n"
"\t                  option -r0 will revert the round parameter to the default.\n"
"\t-d nnn        Alternative form of -dnnn.\n"
"\t-L nnn        Alternative form of -Lnnn.\n"
"\t-r nnn        Alternative form of -rnnn.\n"
"\t-c file hash  Checks the hash of each file against the expected value.\n"
"\t-C hashfile   Checks the validity of the hashes in the given file.\n"
"\t                  The file provided is expected to have the hashes stored\n"
"\t                  in the same format as is output by md6sum.\n"
"\t-h            Prints this help message, ignoring all following arguments.\n"
"\n"
"Options are case-sensitive and are processed in order.\n"
"\n"
"For each file given, md6sum, using the option -dnnn, provides output of the form:\n"
"Name: filename\n"
"MD6-nnn-Digest: hashvalue\n"
"\n"
"When not using -c or -C, file names can include wildcards. For example,\n"
"md6sum -d224 *.txt\n"
"will output the MD6-224 Digest of all files in the current working directory\n"
"that have a .txt extension. In addition, the contents of directories are recursively\n"
"processed as well so, for example,\n"
"md6sum -d384 *\n"
"will output the MD6-384 Digest of all files in the current working directory and\n"
"all files in any sub-directories in the current working directory.\n"
"\n"
"In the case that no input file is given, or the file given is '-', md6sum,\n"
"using the option -dnnn, provides output of the form:\n"
"Name: -\n"
"MD6-nnn-Digest: hashvalue\n"
"\n"
"If file is \'-\', or no files are given, standard input is used.\n"
"\n"
"Note that, when using -C, the MD6 parameters are not read from the hash check file,\n"
"but rather from the program arguments.\n"
"\n"
"Successful completion of execution returns a value of 0.\n"
"Unsuccessful completion will return a negative integer.\n"
"In the case of the -c and/or -C option(s) being used, the number of failed hash\n"
"checks is returned upon successful completion, or 0 if all hash checks pass.\n"
;


struct md6_ctx ctx;
uint8_t hashval[MD6_512_DIGEST_SIZE];

struct hash_struct hash = {
	MD6_256, &ctx,
	(crypto_hash_update_func*)&md6_update,
	(crypto_hash_digest_func*)&md6_digest,
	hashval, MD6_256_DIGEST_SIZE
};

/* Default Parameters: d=256, K='', L=64, r=40+(d/4)=104 */
int d = 256;
uint8_t K[64];
size_t klen = 0;
int L = 64;
int r = 0;		/* r = 0 causes md6_init_full to use the default number of rounds */

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

void optd(const char *arg) {
	int t;
	size_t n;
	if (*arg == '\0') {
		fputs("No argument given for -d option\n", stderr);
		exit(-1);
	}
	n = strlen(arg);
	t = (int)strtol(arg, NULL, 10);
	if (n < 1 || n > 3 || t > 512 || t <= 0) {
		fprintf(stderr, "Invalid argument for -d option: %s;"
				" must be an integer in the range of [1, 512]\n", arg);
		exit (-1);
	}
	d = t;
	hash.alg = d;
	hash.digest_size = (d+7)/8;
}

void optK(const char *arg) {
	if (strlen(arg) == 0) {
		/* In case of empty argument (meaning no key used) */
		klen = 0;
		memset(K, '\0', 64);
	}
	klen = encoder->decoded_size((const unsigned char*)arg);
	if (klen > 64 || encoder->encoded_size(klen) != strlen(arg)) {
		/* Check if the key is too long or is not valid */
		fprintf(stderr, "Invalid argument for -K option: %s\n"
				"Must be a %s string between 0 and 64 bits in length\n",
				arg, encoder->name);
		exit(-1);
	}
	encoder->decode_func(K, (const unsigned char *)arg);
	memset(K + klen, '\0', 64 - klen);
}

void optL(const char *arg) {
	int t;
	size_t n;
	if (*arg == '\0') {
		fputs("No argument given for -L option\n", stderr);
		exit(-1);
	}
	n = strlen(arg);
	t = (int)strtol(arg, NULL, 10);
	if (n < 1 || n > 3 || t > 255 || t < 0 || (t == 0 && *arg != '0')) {
		fprintf(stderr, "Invalid argument for -L option: %s;"
				" must be an integer in the range of [0, 255]\n", arg);
		exit (-1);
	}
	L = t;
}

void optr(const char *arg) {
	int t;
	size_t n;
	if (*arg == '\0') {
		fputs("No argument given for -r option\n", stderr);
		exit(-1);
	}
	n = strlen(arg);
	t = (int)strtol(arg, NULL, 10);
	if (n < 1 || n > 3 || t > 255 || t < 0 || (t == 0 && *arg != '0')) {
		fprintf(stderr, "Invalid argument for -r option: %s;"
				" must be an integer in the range of [0, 255]\n", arg);
		exit (-1);
	}
	r = t;
}

void optdalt(int argc, const char *argv[], int i) {
	if (i == argc-1) {
		fputs("No argument given for -d option\n", stderr);
		exit(-1);
	}
	optd(argv[i+1]);
}

void optLalt(int argc, const char *argv[], int i) {
	if (i == argc-1) {
		fputs("No argument given for -L option\n", stderr);
		exit(-1);
	}
	optL(argv[i+1]);
}

void optralt(int argc, const char *argv[], int i) {
	if (i == argc-1) {
		fputs("No argument given for -r option\n", stderr);
		exit(-1);
	}
	optr(argv[i+1]);
}


void hash_stdin() {
	md6_init_full(&ctx, d, K, klen, L, r);
	hash_filep(&hash, stdin);
	print_file_hash(&hash, "-");
}

int check_hash(const char *filename, const char *oldhash) {
	uint8_t decoded[64];	/* 64 bytes in case of invalid hashes */
	FILE *in;
	struct stat st;
	if (encoder->decoded_size((const unsigned char*)oldhash) != hash.digest_size) {
		fprintf(stderr, "Invalid %s MD6-%d Digest: %s\n", encoder->name, d, oldhash);
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
	md6_init_full(&ctx, d, K, klen, L, r);
	hash_filep(&hash, in);
	fclose(in);
	return memcmp(hashval, decoded, hash.digest_size) != 0;
}


int optc(int argc, const char *argv[], int i) {
	if (i >= argc-2) {
		fputs("No file and/or hash given for -c option\n", stderr);
		exit(-1);
	}
	md6_init_full(&ctx, d, K, klen, L, r);
	if (check_hash(argv[i+1], argv[i+2])) {
		printf("Hash check failure for \"%s\"\n", argv[i+1]);
		printf("%s != ", argv[i+2]); print_hash(&hash); puts("\n");
		return 1;
	}
	return 0;
}

int optC(int argc, const char *argv[], int i) {
	char filename[FILENAME_MAX+1];
	char hashcheck[MD6_DIGEST_SIZE*8+1];	/* Max length (in case of binary format) */
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
	md6_init_full(&ctx, d, K, klen, L, r);
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
			fprintf(stderr, "Invalid MD6-%d Digest in hash check file \"%s\";"
					" skipping the remainder of the file\n", d, argv[i+1]);
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
			md6_init_full(&ctx, d, K, klen, L, r);
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
					/* Set the MD6 digest size parameter d */
					case 'd': optdalt(argc, (const char**)argv, i); i += 1; continue;
					/* Set the MD6 key parameter K to the default */
					case 'K': optK(""); continue;
					/* Set the MD6 mode parameter L */
					case 'L': optLalt(argc, (const char**)argv, i); i += 1; continue;
					/* Set the MD6 round parameter r */
					case 'r': optralt(argc, (const char**)argv, i); i += 1; continue;
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
				/* Set the MD6 digest size parameter d */
				optd(argv[i] + 2);
				continue;
			} else if (argv[i][1] == 'K') {
				/* Set the MD6 key parameter K */
				optK(argv[i] + 2);
				continue;
			} else if (argv[i][1] == 'L') {
				/* Set the MD6 mode parameter L */
				optL(argv[i] + 2);
				continue;
			} else if (argv[i][1] == 'r') {
				/* Set the MD6 round parameter r */
				optr(argv[i] + 2);
				continue;
			}
			fprintf(stderr, "Invalid Option: %s\n", argv[i]);
			return -1;
		}
	}

	return ret;
}

