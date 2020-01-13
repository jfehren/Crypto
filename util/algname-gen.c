/*
 * md6-names-gen.c
 *
 *  Created on: Sep 12, 2019, 12:35:51 PM
 *      Author: Joshua Fehrenbach
 */

#include <stdio.h>

int main() {
	puts("/* AUTO-GENERATED FILE - DO NOT EDIT! */\n");
	for (int i = 1; i <= 512; i++) {
		printf("\"MD6-%d-Digest\",", i);
		if (i % 5 == 0) {
			putchar('\n');
		}
	}
	putchar('\n');
	puts("\"MD2-Digest\",");
	puts("\"MD4-Digest\",");
	puts("\"MD5-Digest\",");
	puts("\"SHA-1-Digest\",");
	puts("\"SHA-224-Digest\",");
	puts("\"SHA-256-Digest\",");
	puts("\"SHA-384-Digest\",");
	puts("\"SHA-512-Digest\",");
	puts("\"SHA-512/224-Digest\",");
	puts("\"SHA-512/256-Digest\",");
	puts("\"SHA3-224-Digest\",");
	puts("\"SHA3-256-Digest\",");
	puts("\"SHA3-384-Digest\",");
	puts("\"SHA3-512-Digest\",");
	putchar('\n');
}
