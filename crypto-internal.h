/*
 * crypto-internal.h
 *
 *  Created on: Apr 24, 2019, 3:10:30 PM
 *      Author: Joshua Fehrenbach
 */

#ifndef CRYPTO_INTERNAL_H_
#define CRYPTO_INTERNAL_H_

#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#if __IS_x86__ && CRYPTO_FAT

extern int _is_x86_64;
extern int _is_aesni;
extern int _is_avx;
extern int _is_sha;
extern int _is_pclmul;

extern inline int
ATTRIBUTE(gnu_inline,always_inline,artificial)
is_x86_64() {
	if (_is_x86_64 == -1) {
		int is_long;
		/* Check whether long mode is possible */
		__asm__ __volatile__ (
			"	movl	$0x80000000, %%eax\n"
			"	cpuid\n"
			"	cmpl	$0x80000001, %%eax\n"
			: "=@ccnb" (is_long) :
			: "%eax", "%ebx", "%ecx", "%edx");
		if (is_long) {
			/* Check whether long mode can be enabled */
			__asm__ __volatile__ (
				"	movl	$0x80000001, %%eax\n"
				"	cpuid\n"
				"	testl	$0x20000000, %%edx\n"
				: "=@ccne" (is_long) :
				: "%eax", "%ebx", "%ecx", "%edx");
			if (is_long) {
				/* Check whether long mode is enabled */
				__asm__ __volatile__ (
					"	movl	$0xC0000080, %%ecx\n"
					"	rdmsr\n"
					"	testl	$0x00000100, %%eax\n"
					: "=@cce" (is_long) :
					: "%eax", "%ecx", "%edx");
			}
		}
		_is_x86_64 = is_long;
	}
	return _is_x86_64;
}

extern inline int
ATTRIBUTE(gnu_inline,always_inline,artificial)
is_aesni() {
	if (_is_aesni == -1) {
		int aesni;
		__asm__ __volatile__ (
			"	movl	$1, %%eax\n"
			"	cpuid\n"
			"	testl	$0x02000000, %%ecx\n"
			: "=@cce" (aesni) :
			: "%eax", "%ebx", "%ecx", "%edx");
		_is_aesni = aesni;
	}
	return _is_aesni;
}

extern inline int
ATTRIBUTE(gnu_inline,always_inline,artificial)
is_avx() {
	if (_is_avx == -1) {
		int avx;
		__asm__ __volatile__ (
			"	movl	$1, %%eax\n"
			"	cpuid\n"
			"	testl	$0x10000000, %%ecx\n"
			: "=@ccz" (avx) :
			: "%eax", "%ebx", "%ecx", "%edx");
		_is_avx = avx;
	}
	return _is_avx;
}

extern inline int
ATTRIBUTE(gnu_inline,always_inline,artificial)
is_sha() {
	if (_is_sha == -1) {
		int sha;
		__asm__ __volatile__ (
			"	movl	$7, %%eax\n"
			"	xorl	%%ecx, %%ecx\n"
			"	cpuid\n"
			"	testl	$0x20000000, %%ebx\n"
			: "=@cce" (sha) :
			: "%eax", "%ebx", "%ecx", "%edx");
		_is_sha = sha;
	}
	return _is_sha;
}

extern inline int
ATTRIBUTE(gnu_inline,always_inline,artificial)
is_pclmul() {
	if (_is_pclmul == -1) {
		int pclmul;
		__asm__ __volatile__ (
			"	movl	$1, %%eax\n"
			"	cpuid\n"
			"	testl	$2, %%ecx\n"
			: "=@cce" (pclmul) :
			: "%eax", "%ebx", "%ecx", "%edx");
		_is_pclmul = pclmul;
	}
	return _is_pclmul;
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_INTERNAL_H_ */
