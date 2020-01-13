	.file	"sha3-avx.asm"
	.text
	.globl	crypto_sha3_permute_avx
	.def	crypto_sha3_permute_avx;	.scl	2;	.type	32;	.endef
	/* void sha3_permute_avx(struct sha3_state *state [%rdi]) */
	.align	16
crypto_sha3_permute_avx:
	/* Clobber registers */
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	subq	$136, %rsp	/* Allocate 16-byte aligned space to clobber xmm8-xmm15 */
	movdqa	%xmm8, (%rsp)
	movdqa	%xmm9, 16(%rsp)
	movdqa	%xmm10, 32(%rsp)
	movdqa	%xmm11, 48(%rsp)
	movdqa	%xmm12, 64(%rsp)
	movdqa	%xmm13, 80(%rsp)
	movdqa	%xmm14, 96(%rsp)
	movdqa	%xmm15, 112(%rsp)

	movl	$24, %r9d
	leaq	.rc-8(%rip), %rsi
	/* Load state */
	movq	(%rdi), %rax		/* A0    -> rax */
	movups	8(%rdi), %xmm0		/* A0102 -> xmm0 */
	movups	24(%rdi), %xmm1		/* A0304 -> xmm1 */
	movq	%rax, %r10			/* A0    -> r10		[C0] */

	movq	40(%rdi), %rcx		/* A05   -> rcx */
	movdqa	%xmm0, %xmm10		/* A0102 -> xmm10	[C12] */
	movups	48(%rdi), %xmm2		/* A0607 -> xmm2 */
	movdqa	%xmm1, %xmm11		/* A0304 -> xmm11	[C34] */
	movups	64(%rdi), %xmm3		/* A0809 -> xmm3 */
	xorq	%rcx, %r10			/* C0 ^= A05 */

	movq	80(%rdi), %rdx		/* A10   -> rdx */
	pxor	%xmm2, %xmm10		/* C12 ^= A0607 */
	movups	88(%rdi), %xmm4		/* A1112 -> xmm4 */
	pxor	%xmm3, %xmm11		/* C34 ^= A0809 */
	movups	104(%rdi), %xmm5	/* A1314 -> xmm5 */
	xorq	%rdx, %r10			/* C0 ^= A10 */

	movq	120(%rdi), %rbp		/* A15   -> rbp */
	pxor	%xmm4, %xmm10		/* C12 ^= A1112 */
	movups	128(%rdi), %xmm6	/* A1617 -> xmm6 */
	pxor	%xmm5, %xmm11		/* C34 ^= A1314 */
	movups	144(%rdi), %xmm7	/* A1819 -> xmm7 */
	xorq	%rbp, %r10			/* C0 ^= A15 */

	movq	160(%rdi), %r8		/* A20   -> r8 */
	pxor	%xmm6, %xmm10		/* C12 ^= A1617 */
	movups	168(%rdi), %xmm8	/* A2122 -> xmm8 */
	pxor	%xmm7, %xmm11		/* C34 ^= A1819 */
	movups	184(%rdi), %xmm9	/* A2324 -> xmm9 */
	xorq	%r8, %r10			/* C0 ^= A20 */
	pxor	%xmm8, %xmm10		/* C12 ^= A2122 */
	pxor	%xmm9, %xmm11		/* C34 ^= A2324 */

	/***************************************************
	 * Register Usage:
	 * rax - A00				rbx - N/A
	 * rcx - A05				rdx - A10
	 * rbp - A15				rsp - stack pointer
	 * rdi - state pointer		rsi - RC
	 * r8  - A20				r9  - counter
	 * r10 - C0/T3				r11 - D0/T4
	 * r12 - T0					r13 - T1
	 * r14 - T2					r15 - N/A
	 *
	 * xmm0  - A0102			xmm1  - A0304
	 * xmm2  - A0607			xmm3  - A0809
	 * xmm4  - A1112			xmm5  - A1314
	 * xmm6  - A1617			xmm7  - A1819
	 * xmm8  - A2122			xmm9  - A2324
	 * xmm10 - C12				xmm11 - C34
	 * xmm12 - D12/W2			xmm13 - D34/W3
	 * xmm14 - W0				xmm15 - W1
	 */

	.align	16
.Ltop:
	/* Theta step - Combine parity bits, then xor to state
	 * D0 = C4 ^ (C1 <<< 1)
	 * D1 = C0 ^ (C2 <<< 1)
	 * D2 = C1 ^ (C3 <<< 1)
	 * D3 = C2 ^ (C4 <<< 1)
	 * D4 = C3 ^ (C0 <<< 1)
	 *
	 * Shift the words around, putting (C0,C1) in D12, (C2,C3) in D34, and (C4,C0) in C34
	 *
	 * punpckhqdq 01, 23 gives 31
	 * punpcklqdq 01, 23 gives 20
	 */

	movq	%r10, (%rdi)			/* store C0 in cached memory */
	pshufd	$0x4e, %xmm11, %xmm11	/* (C4, C3) -> C34 */
	movdqa	%xmm10, %xmm13			/* C12 -> D34 */
	movq	(%rdi), %xmm12			/* (~, C0) -> D12 */
	punpcklqdq	%xmm10, %xmm12		/* (C0, C1) -> D12 */
	punpckhqdq	%xmm11, %xmm13		/* (C2, C3) -> D34 */
	punpcklqdq	%xmm12, %xmm11		/* (C4, C0) -> C34 */
	movq	%xmm10, (%rdi)			/* store C1 in cached memory */
	movq	%xmm11, 16(%rdi)		/* store C4 in cached memory */
	movq	(%rdi), %r11			/* C1 -> D0 */
	movq	16(%rdi), %r12			/* C4 -> T0 */
	rolq	$1, %r11				/* (C1 <<< 1) -> D0 */
	xorq	%r12, %r11				/* C4 ^ (C1 <<< 1) -> D0 */

	/* Can use C12 as a temporary */
	movdqa	%xmm13, %xmm15			/* (C2, C3) -> W1 */
	movdqa	%xmm11, %xmm10			/* (C4, C0) -> C12 */
	vpsllq	$1, %xmm13, %xmm14		/* (C2 << 1, C3 << 1) -> W0 */
	vpsllq	$1, %xmm11, %xmm10		/* (C4 << 1, C0 << 1) -> C12 */
	vpsrlq	$63, %xmm13, %xmm15		/* (C2 >> 63, C3 >> 63) -> W1 */
	vpsrlq	$63, %xmm11, %xmm11		/* (C4 >> 63, C0 >> 63) -> C34 */
	pxor	%xmm14, %xmm12			/* (C0 ^ (C2 << 1), C1 ^ (C3 << 1)) -> D12 */
	pxor	%xmm10, %xmm13			/* (C2 ^ (C4 << 1), C3 ^ (C0 << 1)) -> D34 */
	pxor	%xmm15, %xmm12			/* (C0 ^ (C2 <<< 1), C1 ^ (C3 <<< 1)) -> D12 */
	pxor	%xmm11, %xmm13			/* (C2 ^ (C4 <<< 1), C3 ^ (C0 <<< 1)) -> D34 */

	/* XOR D01234 into the state */
	xorq	%r11, %rax				/* A00   ^= D0 */
	xorq	%r11, %rcx				/* A05   ^= D0 */
	xorq	%r11, %rdx				/* A10   ^= D0 */
	xorq	%r11, %rbp				/* A15   ^= D0 */
	xorq	%r11, %r8				/* A20   ^= D0 */
	pxorq	%xmm12, %xmm0			/* A0102 ^= D12 */
	pxorq	%xmm12, %xmm2			/* A0607 ^= D12 */
	pxorq	%xmm12, %xmm4			/* A1112 ^= D12 */
	pxorq	%xmm12, %xmm6			/* A1617 ^= D12 */
	pxorq	%xmm12, %xmm8			/* A2122 ^= D12 */
	pxorq	%xmm13, %xmm1			/* A0304 ^= D34 */
	pxorq	%xmm13, %xmm3			/* A0809 ^= D34 */
	pxorq	%xmm13, %xmm5			/* A1314 ^= D34 */
	pxorq	%xmm13, %xmm7			/* A1819 ^= D34 */
	pxorq	%xmm13, %xmm9			/* A2324 ^= D34 */

	/* Combined Rho and Pi Steps - Permutate and Transpose
	 *
	 * The combined permutation & transpose gives the following cycles,
	 * with rotation counts in parentheses.
	 *     0  <- 0(0)
	 *     1  <- 3(28) <- 4(27) <- 2(62) <- 1(1)
	 *     5  <- 6(44) <- 9(20) <- 8(55) <- 5(36)
	 *     7  <- 7(6)
	 *     10 <- 12(43) <- 13(25) <- 11(10) <- 10(3)
	 *     14 <- 14(39)
	 *     15 <- 18(21) <- 17(15) <- 19(8) <- 15(41)
	 *     16 <- 16(45)
	 *     20 <- 24(14) <- 21(2) <- 22(61) <- 20(18)
	 *     23 <- 23(56)
	 */

	vpsllq	$62, %xmm0, %xmm15		/* A02 << 62 -> W1 */
	vpsrlq	$63, %xmm0, %xmm14		/* A01 >> 63 -> W0 */
	vpsrlq	$2,  %xmm0, %xmm12		/* A02 >>  2 -> W2 */
	vpsllq	$1,  %xmm0, %xmm0		/* A01 <<  1 -> A0102 */
	por		%xmm15, %xmm12			/* (A02 <<< 62) -> W2		(new A04) */
	por		%xmm0,  %xmm14			/* (A01 <<<  1) -> W0		(new A02) */

	vpsllq	$28, %xmm1, %xmm0		/* A03 << 28 -> A0102 */
	vpsrlq	$36, %xmm1, %xmm15		/* A03 >> 36 -> W1 */
	vpsllq	$27, %xmm1, %xmm13		/* A04 << 27 -> W3 */
	vpsrlq	$37, %xmm1, %xmm1		/* A04 >> 37 -> A0304 */
	por		%xmm15, %xmm0			/* (A03 <<< 28) -> A0102	(new A01) */
	por		%xmm13, %xmm1			/* (A04 <<< 27) -> A0304	(new A03) */

	punpcklqdq	%xmm14, %xmm0		/* (A03 <<< 28, A01 <<<  1) -> A0102 */
	punpcklqdq	%xmm12, %xmm1		/* (A04 <<< 27, A02 <<< 62) -> A0304 */

	/* 5 <- 6(44) <- 9(20) <- 8(55) <- 5(36)
	 * 7 <- 7(6)
	 *
	 * Diagram:
     *      __   _______
	 *  _ L'  ` L_    __`
	 * |5|    |6|7|  |8|9|
	 *   `-_________-^`-^
	 */

	rolq	$36, %rcx				/* (A05 <<< 36) -> A05		(new A08) */
	movq	%xmm2, (%rdi)			/* store A06 in cached memory */
	movq	%rcx, 16(%rdi)			/* store (A05 <<< 36) in cached memory */
	movq	(%rdi), %rcx			/* A06 -> A05 */
	movq	16(%rdi), %xmm14		/* (A05 <<< 36) -> W0 */
	rolq	$44, %rcx				/* (A06 <<< 44) -> A05		(new A05) */
	vpsrlq	$58, %xmm2, %xmm15		/* A07 >> 58 -> W1 */
	vpsllq	$6,  %xmm2, %xmm2		/* A07 <<  6 -> A0607 */
	vpsrlq	$44, %xmm3, %xmm12		/* A09 >> 44 -> W2 */
	por		%xmm2, %xmm15			/* (A07 <<<  6) -> W1		(new A07) */
	vpsllq	$22, %xmm3, %xmm2		/* A09 << 20 -> A0607 */
	vpsrlq	$9,  %xmm3, %xmm13		/* A08 >>  9 -> W3 */
	por		%xmm12, %xmm2			/* (A09 <<< 20) -> W2		(new A06) */
	vpsllq	$55, %xmm3, %xmm3		/* A08 << 55 -> A0809 */
	punpckhqdq	%xmm15, %xmm2		/* (A09 <<< 20, A07 <<<  6) -> A0607 */
	por		%xmm13, %xmm3			/* (A08 <<< 55) -> A0809	(new A09) */
	vpunpcklqdq	%xmm3,%xmm14,%xmm3	/* (A05 <<< 36, A08 <<< 55) -> A0809 */

	/* 10 <- 12(43) <- 13(25) <- 11(10) <- 10(3)
	 * 14 <- 14(39)
	 *
	 * Diagram:
	 *      _____   ___
	 *  __L'   __`_L_  `_____
	 * |10|   |11|12|  |13|14|
	 *   `-___-^`-______-^
	 */

	pshufd	$0x4e, %xmm4, %xmm14	/* (A12, A11) -> W0 */
	rolq	$42, %rdx				/* (A10 <<< 42) -> A10		(later will do A10 <<< 25) */
	movq	%xmm14, (%rdi)			/* store A12 in cached memory */
	movq	%rdx, 16(%rdi)			/* store (A10 <<< 42) in cached memory */
	movq	(%rdi), %rdx			/* A12 -> A10 */
	movq	16(%rdi), %xmm4			/* (A10 <<< 42) -> A1112 */
	rolq	$43, %rdx				/* (A12 <<< 43) -> A10		(new A10) */

	punpcklqdq	%xmm5, %xmm4		/* (A10 <<< 42, A13) -> A1112 */
	vpsrlq	$25, %xmm5, %xmm12		/* A14 >> 25 -> W2 */
	vpsllq	$39, %xmm5, %xmm5		/* A14 << 39 -> A1314 */
	vpsrlq	$39, %xmm4, %xmm15		/* ((A10 <<< 42) >> 39, A13 >> 39) -> W1 */
	vpsllq	$25, %xmm4, %xmm4		/* ((A10 <<< 42) << 25, A13 << 25) -> A1112 */
	por		%xmm5, %xmm12			/* (A14 <<< 39) -> W2		(new A14) */
	vpsrlq	$54, %xmm14, %xmm5		/* A11 >> 54 -> A1314 */
	vpsllq	$10, %xmm14, %xmm14		/* A11 << 10 -> W0 */
	por		%xmm15, %xmm4			/* (A10 <<< 3, A13 <<< 25) -> A1112		(new A11, A12) */
	por		%xmm14, %xmm5			/* (A11 <<< 10) -> A1314	(new A13) */
	punpckhqdq	%xmm12, %xmm5		/* (A11 <<< 10, A14 <<< 39) -> A1314 */

	/* 15 <- 18(21) <- 17(15) <- 19(8) <- 15(41)
	 * 16 <- 16(45)
	 *
	 * Diagram:
	 *      _____________
	 *     /         _______
	 *  _L'    ____L'    |  `_
	 * |15|   |16|17|   |18|19|
	 *   \        `_____-^   ^
	 *    \_________________/
	 */

	pshufq	$0x4e, %xmm7, %xmm14	/* (A19, A18) -> W0 */
	rolq	$41, %rbp				/* (A15 <<< 41) -> A15		(new A19) */
	movq	%xmm7, (%rdi)			/* store A18 in cached memory */
	movq	%rbp, 16(%rdi)			/* store (A15 <<< 41) in cached memory */
	movq	(%rdi), %rbp			/* A18 -> A15 */
	movq	16(%rdi), %xmm15		/* (A15 <<< 41) -> W1 */
	rolq	$21, %rbp				/* (A18 <<< 21) -> A15		(new A15) */
	pshufd	$0x4e, %xmm6, %xmm7		/* (A17, A16) -> A1819 */
	vpsrlq	$19, %xmm6, %xmm12		/* A16 >> 19 -> W2 */
	vpsllq	$45, %xmm6, %xmm6		/* A16 << 45 -> A1617 */
	vpsrlq	$56, %xmm14, %xmm13		/* A19 >> 56 -> W3 */
	vpsllq	$8,  %xmm14, %xmm14		/* A19 <<  8 -> W0 */
	por		%xmm12, %xmm6			/* (A16 <<< 45) -> A1617	(new A16) */
	vpsrlq	$49, %xmm7, %xmm12		/* A17 >> 49 -> W2 */
	vpsllq	$15, %xmm7, %xmm7		/* A17 << 15 -> A1819 */
	por		%xmm13, %xmm14			/* (A19 <<<  8) -> W0		(new A17) */
	por		%xmm12, %xmm7			/* (A17 <<< 15) -> A1819	(new A18) */
	punpcklqdq	%xmm14, %xmm6		/* (A16 <<< 45, A19 <<<  8) -> A1617 */
	punpcklqdq	%xmm15, %xmm7		/* (A17 <<< 15, A15 <<< 41) -> A1819 */

	/* 20 <- 24(14) <- 21(2) <- 22(61) <- 20(18)
	 * 23 <- 23(56)
	 *
	 * Diagram:
	 *      ________________
	 *     /                \
	 *  _L'    _L'\_     ___|_
	 * |20|   |21|22|   |23|24|
	 *   \     `__ ^________-^
	 *    \_______/
	 */

	rolq	$18, %r8				/* (A20 <<< 18) -> A20		(new A22) */
	pshufd	$0x4e, %xmm9, %xmm15	/* (A24, A23) -> W1 */
	movq	%xmm15, (%rdi)			/* store A24 in cached memory */
	pinsrq	$1, %r8, %xmm14			/* (~, A20 <<< 18) -> W0 */
	movq	(%rdi), %r8				/* A24 -> A20 */
	vpsrlq	$8,  %xmm9, %xmm15		/* (A23 >>  8) -> W1 */
	vpsllq	$56, %xmm9, %xmm8		/* (A23 << 56) -> A2324 */
	rolq	$14, %r8				/* (A24 <<< 14) -> A20		(new A20) */
	vpsllq	$2,  %xmm8, %xmm12		/* (A21 <<  2) -> W2 */
	vpsrlq	$62, %xmm8, %xmm13		/* (A21 >> 62) -> W3 */
	por		%xmm15, %xmm9			/* (A23 <<< 56) -> A2324	(new A23) */
	vpsrlq	$3,  %xmm8, %xmm15		/* (A22 >>  3) -> W1 */
	vpsllq	$61, %xmm8, %xmm8		/* (A22 << 61) -> A2122 */
	por		%xmm13, %xmm12			/* (A21 <<<  2) -> W2		(new A24) */
	por		%xmm15, %xmm8			/* (A22 <<< 61) -> A2122	(new A21) */
	punpcklqdq	%xmm12, %xmm9		/* (A23 <<< 56, A21 <<<  2) -> A2324 */
	punpckhqdq	%xmm14, %xmm8		/* (A20 <<< 18, A22 <<< 61) -> A2122 */

	/* Chi step - With the transposed matrix, applied independently to each column */
	movq	%rax, %r12				/* A00 -> T0 */
	movq	%rcx, %r13				/* A05 -> T1 */
	movq	%rdx, %r14				/* A10 -> T2 */
	movq	%rbp, %r10				/* A15 -> T3 */
	movq	%r8,  %r11				/* A20 -> T4 */
	notq	%r12					/* ~A00 -> T0 */
	notq	%r13					/* ~A05 -> T1 */
	notq	%r14					/* ~A10 -> T2 */
	notq	%r10					/* ~A15 -> T3 */
	notq	%r11					/* ~A20 -> T4 */
	andq	%rcx, %r12				/* (~A00) & A05 -> T0 */
	andq	%rdx, %r13				/* (~A05) & A10 -> T1 */
	andq	%rbp, %r14				/* (~A10) & A15 -> T2 */
	andq	%r8,  %r10				/* (~A15) & A20 -> T3 */
	andq	%rax, %r11				/* (~A20) & A00 -> T4 */
	xorq	%r12, %r8				/* A20 ^= ((~A00) & A05) */
	xorq	%r13, %rax				/* A00 ^= ((~A05) & A10) */
	xorq	%r14, %rcx				/* A05 ^= ((~A10) & A15) */
	xorq	%r10, %rdx				/* A10 ^= ((~A15) & A20) */
	xorq	%r11, %rbp				/* A15 ^= ((~A20) & A00) */

	vpandn	%xmm0, %xmm2, %xmm14	/* (~A0102) & A0607 -> W0 */
	vpandn	%xmm2, %xmm4, %xmm15	/* (~A0607) & A1112 -> W1 */
	vpandn	%xmm4, %xmm6, %xmm12	/* (~A1112) & A1617 -> W2 */
	vpandn	%xmm6, %xmm8, %xmm13	/* (~A1617) & A2122 -> W3 */
	vpandn	%xmm8, %xmm0, %xmm10	/* (~A2122) & A0102 -> W4 */
	pxor	%xmm14, %xmm8			/* A2122 ^= ((~A0102) & A0607) */
	pxor	%xmm15, %xmm0			/* A0102 ^= ((~A0607) & A1112) */
	pxor	%xmm12, %xmm2			/* A0607 ^= ((~A1112) & A1617) */
	pxor	%xmm13, %xmm4			/* A1112 ^= ((~A1617) & A2122) */
	pxor	%xmm10, %xmm6			/* A1617 ^= ((~A2122) & A0102) */

	vpandn	%xmm1, %xmm3, %xmm14	/* (~A0304) & A0809 -> W0 */
	vpandn	%xmm3, %xmm5, %xmm15	/* (~A0809) & A1314 -> W1 */
	vpandn	%xmm5, %xmm7, %xmm12	/* (~A1314) & A1819 -> W2 */
	vpandn	%xmm7, %xmm9, %xmm13	/* (~A1819) & A2324 -> W3 */
	vpandn	%xmm9, %xmm1, %xmm10	/* (~A2324) & A0304 -> W4 */
	pxor	%xmm14, %xmm9			/* A2324 ^= ((~A0304) & A0809) */
	pxor	%xmm15, %xmm1			/* A0304 ^= ((~A0809) & A1314) */
	pxor	%xmm12, %xmm3			/* A0809 ^= ((~A1314) & A1819) */
	pxor	%xmm13, %xmm5			/* A1314 ^= ((~A1819) & A2324) */
	pxor	%xmm10, %xmm7			/* A1819 ^= ((~A2324) & A0304) */

	xorq	(%rsi,%r9,8), %rax		/* A00 ^= RC */

	/* Transpose */

	/* Swap (A05, A10) <-> A0102, and (A15, A20) <-> A0304, and also copy to C12 and C34 */
	movq	%rcx, (%rdi)			/* store A05 in cached memory */
	movq	%rbp, 16(%rdi)			/* store A15 in cached memory */
	movq	(%rdi), %xmm10			/* A05 -> C12 */
	movq	16(%rdi), %xmm11		/* A15 -> C34 */
	movq	%rdx, (%rdi)			/* store A10 in cached memory */
	movq	%r8,  16(%rdi)			/* store A20 in cached memory */
	movq	(%rdi), %xmm14			/* A10 -> W0 */
	movq	16(%rdi), %xmm15		/* A20 -> W1 */
	movq	%rax, %r10				/* A00 -> C0 */
	punpcklqdq	%xmm14, %xmm10		/* (A05, A10) -> C12 */
	punpcklqdq	%xmm15, %xmm11		/* (A15, A20) -> C34 */
	movq	%xmm0, (%rdi)			/* store A01 in cached memory */
	movq	%xmm1, 16(%rdi)			/* store A03 in cached memory */
	movq	(%rdi), %rcx			/* A01 -> A05					(new A05) */
	movq	16(%rdi), %rbp			/* A03 -> A15					(new A15) */
	psrldq	$8, %xmm0				/* A02 -> A0102 */
	xorq	%rcx, %r10				/* C0 ^= A01 */
	psrldq	$8, %xmm1				/* A04 -> A0304 */
	xorq	%rbp, %r10				/* C0 ^= A03 */
	movq	%xmm0, (%rdi)			/* store A02 in cached memory */
	movq	%xmm1, 16(%rdi)			/* store A04 in cached memory */
	movq	(%rdi), %rdx			/* A02 -> A10					(new A10) */
	movq	16(%rdi), %r8			/* A04 -> A20					(new A20) */
	movdqa	%xmm10, %xmm0			/* (A05, A10) -> A0102			(new A0102) */
	movdqa	%xmm11, %xmm1			/* (A15, A20) -> A0304			(new A0304) */

	/* Transpose (A0607, A1112) */
	movdqa	%xmm2, %xmm14			/* A0607 -> W0 */
	punpcklqdq	%xmm4, %xmm2		/* (A06, A11) -> A0607			(new A0607) */
	xorq	%rdx, %r10				/* C0 ^= new A10 */
	vpunpckhqdq	%xmm4,%xmm14,%xmm4	/* (A07, A12) -> A1112			(new A1112) */
	xorq	%rbp, %r10				/* C0 ^= new A15				(new C0) */

	/* Transpose (A1819, A2324) */
	movdqa	%xmm7, %xmm14			/* A1819 -> W0 */
	punpcklqdq	%xmm9, %xmm7		/* (A18, A23) -> A1819			(new A1819) */
	pxor	%xmm2, %xmm10			/* C12 ^= new A0607 */
	vpunpckhqdq	%xmm9,%xmm14,%xmm9	/* (A19, A24) -> A2324			(new A2324) */
	pxor	%xmm4, %xmm10			/* C12 ^= new A1112 */

	/* Transpose (A0809, A1314) and (A1617, A2122), and swap */
	movdqa	%xmm3, %xmm14			/* A0809 -> W0 */
	movdqa	%xmm5, %xmm15			/* A1314 -> W1 */
	movdqa	%xmm6, %xmm3			/* A1617 -> A0809 */
	movdqa	%xmm8, %xmm5			/* A2122 -> A1314 */
	pxor	%xmm7, %xmm11			/* C34 ^= new A1819 */
	punpcklqdq	%xmm8, %xmm3		/* (A16, A21) -> A0809			(new A1819) */
	vpunpckhqdq	%xmm5, %xmm6, %xmm5	/* (A17, A22) -> A1314			(new A1314) */
	pxor	%xmm3, %xmm11			/* C34 ^= new A0809 */
	vpunpcklqdq	%xmm15,%xmm14,%xmm6	/* (A08, A13) -> A1617			(new A1617) */
	pxor	%xmm5, %xmm11			/* C34 ^= new A1314				(new C34) */
	vpunpckhqdq	%xmm15,%xmm14,%xmm8	/* (A09, A14) -> A2122			(new A2122) */
	pxor	%xmm6, %xmm10			/* C12 ^= new A1617 */
	pxor	%xmm8, %xmm10			/* C12 ^= new A2122				(new C12) */
	decl	%r9d		/* --counter */
	jnz		.Ltop		/* restart loop if counter != 0 */

	movq	%rax, (%rdi)			/* save A00 */
	movups	%xmm0, 8(%rdi)			/* save A01, A02 */
	movups	%xmm1, 24(%rdi)			/* save A03, A04 */
	movq	%rcx, 40(%rdi)			/* save A05 */
	movups	%xmm2, 48(%rdi)			/* save A06, A07 */
	movups	%xmm3, 64(%rdi)			/* save A08, A09 */
	movq	%rdx, 80(%rdi)			/* save A10 */
	movups	%xmm4, 88(%rdi)			/* save A11, A12 */
	movups	%xmm5, 104(%rdi)		/* save A13, A14 */
	movq	%rbp, 120(%rdi)			/* save A15 */
	movups	%xmm6, 128(%rdi)		/* save A16, A17 */
	movups	%xmm7, 144(%rdi)		/* save A18, A19 */
	movq	%r8, 160(%rdi)			/* save A20 */
	movups	%xmm8, 168(%rdi)		/* save A21, A22 */
	movups	%xmm9, 184(%rdi)		/* save A23, A24 */

	movdqa	112(%rsp), %xmm15
	movdqa	96(%rsp), %xmm14
	movdqa	80(%rsp), %xmm13
	movdqa	64(%rsp), %xmm12
	movdqa	48(%rsp), %xmm11
	movdqa	32(%rsp), %xmm10
	movdqa	16(%rsp), %xmm9
	movdqa	(%rsp), %xmm8
	addq	$136, %rsp
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret

	.section	.rdata, "dr"
	.align	16
.rc:	/* In reverse order */
	.quad	0x8000000080008008
	.quad	0x0000000080000001
	.quad	0x8000000000008080
	.quad	0x8000000080008081
	.quad	0x800000008000000A
	.quad	0x000000000000800A
	.quad	0x8000000000000080
	.quad	0x8000000000008002
	.quad	0x8000000000008003
	.quad	0x8000000000008089
	.quad	0x800000000000008B
	.quad	0x000000008000808B
	.quad	0x000000008000000A
	.quad	0x0000000080008009
	.quad	0x0000000000000088
	.quad	0x000000000000008A
	.quad	0x8000000000008009
	.quad	0x8000000080008081
	.quad	0x0000000080000001
	.quad	0x000000000000808B
	.quad	0x8000000080008000
	.quad	0x800000000000808A
	.quad	0x0000000000008082
	.quad	0x0000000000000001

