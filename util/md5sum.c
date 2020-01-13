/*
 * md5sum.c
 *
 *  Created on: Sep 12, 2019, 2:42:21 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define __CRYPTO_LIBCRYPTO_DLL 0

#include "md5.h"

#undef __CRYPTO_DECLSPEC
#define __CRYPTO_DECLSPEC __CRYPTO_DECLSPEC_IMPORT

#include "util.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

const char help_string[] =
"md5sum [OPTIONS] [file1 file2  file3...]\n"
"Options:\n"
"\t-b            Subsequent hashes are input/output in binary.\n"
"\t-o            Subsequent hashes are input/output in octal.\n"
"\t-x            Subsequent hashes are input/output in hexadecimal (default).\n"
"\t-64           Subsequent hashes are input/output in base 64.\n"
"\t-c file hash  Checks the hash of each file against the expected value.\n"
"\t-C hashfile   Checks the validity of the hashes in the given file.\n"
"\t                  The file provided is expected to have the hashes stored\n"
"\t                  in the same format as is output by md5sum.\n"
"\t-h            Prints this help message, ignoring all following arguments.\n"
"\n"
"Options are case-sensitive and are processed in order.\n"
"\n"
"For each file given, md5sum provides output of the form:\n"
"Name: filename\n"
"MD5-Digest: hashvalue\n"
"\n"
"When not using -c or -C, file names can include wildcards. For example,\n"
"md5sum *.txt\n"
"will output the MD5 Digest of all files in the current working directory\n"
"that have a .txt extension. In addition, the contents of directories are recursively\n"
"processed as well so, for example,\n"
"md5sum *\n"
"will output the MD5 Digest of all files in the current working directory and\n"
"all files in any sub-directories in the current working directory.\n"
"\n"
"In the case that no input file is given, or the file given is '-', md5sum\n"
"provides output of the form:\n"
"Name: -\n"
"MD5-Digest: hashvalue\n"
"\n"
"If file is \'-\', or no files are given, standard input is used.\n"
"\n"
"Successful completion of execution returns a value of 0.\n"
"Unsuccessful completion will return a negative integer.\n"
"In the case of the -c and/or -C option(s) being used, the number of failed hash\n"
"checks is returned upon successful completion, or 0 if all hash checks pass.\n"
;


struct md5_ctx ctx;
uint8_t hashval[MD5_DIGEST_SIZE];

struct hash_struct hash = {
	MD5, &ctx,
	(crypto_hash_update_func*)&md5_update,
	(crypto_hash_digest_func*)&md5_digest,
	hashval, MD5_DIGEST_SIZE
};

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


void hash_stdin() {
	hash_filep(&hash, stdin);
	print_file_hash(&hash, "-");
}

int check_hash(const char *filename, const char *oldhash) {
	uint8_t decoded[64];	/* 64 bytes in case of invalid hashes */
	FILE *in;
	struct stat st;
	if (encoder->decoded_size((const unsigned char*)oldhash) != MD5_DIGEST_SIZE) {
		fprintf(stderr, "Invalid %s MD5 Digest: %s\n", encoder->name, oldhash);
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
	return memcmp(hashval, decoded, MD5_DIGEST_SIZE) != 0;
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
	char hashcheck[MD5_DIGEST_SIZE*8+1];	/* Max length (in case of binary format) */
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
		if (encoder->decoded_size((const unsigned char*)hashcheck) != MD5_DIGEST_SIZE) {
			fprintf(stderr, "Invalid MD5 Digest in hash check file \"%s\";"
					" skipping the remainder of the file\n", argv[i+1]);
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
	md5_init(&ctx);
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
			}
			fprintf(stderr, "Invalid Option: %s\n", argv[i]);
			return -1;
		}
	}

	return ret;
}

