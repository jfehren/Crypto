	.file	"sha256-x64.asm"

	.macro LOAD off
	movl	\off+0x00(%rsi), %eax
	movl	\off+0x04(%rsi), %ebx
	movl	\off+0x08(%rsi), %ecx
	movl	\off+0x0C(%rsi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, \off+0x00(%rsp)
	movl	%ebx, \off+0x04(%rsp)
	movl	%ecx, \off+0x08(%rsp)
	movl	%edx, \off+0x0C(%rsp)
	.endm

	/* W[i] = W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) */
	.macro	EXPAND	i, j, k, l
	movl	\i(%rsp), %r15d		/* W[i-16] -> r15 */
	/* s1(W[i-2]) = (W[i-2] <<< 15) ^ (W[i-2] <<< 13) ^ (W[i-2] >> 10) */
	movl	\j(%rsp), %r12d		/* W[i-2] -> r12 */
	movl	%r12d, %r13d		/* W[i-2] -> r13 */
	shrl	$10, %r12d			/* (W[i-2] >> 10) -> r12 */
	roll	$13, %r13d			/* (W[i-2] <<< 13) -> r13 */
	xorl	%r13d, %r12d		/* (W[i-2] <<< 13) ^ (W[i-2] >> 10) -> r12 */
	roll	$2, %r13d			/* (W[i-2] <<< 15) -> r13 */
	xorl	%r13d, %r12d		/* s1(W[i-2]) -> r12 */
	addl	%r12d, %r15d		/* W[i-16] + s1(W[i-2]) -> r15 */
	/* s0(W[i-15]) = (W[i-15] <<< 25) ^ (W[i-15] <<< 14) ^ (W[i-15] >> 3) */
	movl	\k(%rsp), %r12d		/* W[i-15] -> r12 */
	movl	%r12d, %r13d		/* W[i-15] -> r13 */
	shrl	$3, %r12d			/* (W[i-15] >> 3) -> r12 */
	roll	$14, %r13d			/* (W[i-15] <<< 14) -> r13 */
	xorl	%r13d, %r12d		/* (W[i-15] <<< 14) ^ (W[i-15] >> 3) -> r12 */
	roll	$11, %r13d			/* (W[i-15] <<< 25) -> r13 */
	xorl	%r13d, %r12d		/* s0(W[i-15]) -> r12 */
	addl	%r12d, %r15d		/* W[i-16] + s1(W[i-2]) + s0(W[i-15]) -> r15 */
	addl	\l(%rsp), %r15d		/* W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) -> r15 */
	movl	%r15d, \i(%rsp)		/* W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) -> W[i] */
	.endm

	/* h += S1(e) + Choice(e,f,g) + k + data [%eax] */
	/* d += h */
	/* h += S0(a) + Majority(a,b,c) */
	.macro	RND	a, b, c, d, e, f, g, h, k, data
	/* S1(e) = (e <<< 26) ^ (e <<< 21) ^ (e <<< 7) */
	movl	\e, %r12d		/* e -> r12 */
	movl	\e, %r13d		/* e -> r13 */
	roll	$7, %r12d		/* (e <<< 7) -> r12 */
	roll	$21, %r13d		/* (e <<< 21) -> r13 */
	addl	\data, \h		/* h += data */
	xorl	%r12d, %r13d	/* (e <<< 21) ^ (e <<< 7) -> r13 */
	roll	$19, %r12d		/* (e <<< 26) -> r12 */
	xorl	%r12d, %r13d	/* S1(e) -> r13 */
	addl	%r13d, \h		/* h += S1(e) */
	/* Choice(e,f,g) = ((g ^ f) & e) ^ g */
	movl	\g, %r12d		/* g -> r12 */
	xorl	\f, %r12d		/* (g ^ f) -> r12 */
	addl	\k(%rbp), \h	/* h += k */
	andl	\e, %r12d		/* ((g ^ f) & e) -> r12 */
	xorl	\g, %r12d		/* Choice(e,f,g) -> r12 */
	addl	%r12d, \h		/* h += Choice(e,f,g) */
	/* S0(a) = (a <<< 30) ^ (a <<< 19) ^ (a <<< 10) */
	movl	\a, %r12d		/* a -> r12 */
	movl	\a, %r13d		/* a -> r13 */
	roll	$10, %r12d		/* (a <<< 10) -> r12 */
	roll	$19, %r13d		/* (a <<< 19) -> r13 */
	addl	\h, \d			/* d += h */
	xorl	%r12d, %r13d	/* (a <<< 19) ^ (a <<< 10) -> r13 */
	roll	$20, %r12d		/* (a <<< 30) -> r12 */
	xorl	%r12d, %r13d	/* S0(a) -> r13 */
	addl	%r13d, \h		/* h += S0(a) */
	/* Majority(a,b,c) = (a & b) + ((a ^ b) & c) */
	movl	\a, %r12d		/* a -> r12 */
	movl	\a, %r13d		/* a -> r13 */
	andl	\b, %r12d		/* (a & b) -> r12 */
	xorl	\b, %r13d		/* (a ^ b) -> r13 */
	andl	\c, %r13d		/* ((a ^ b) & c) -> r13 */
	xorl	%r13d, %r12d	/* Majority(a,b,c) -> r12 */
	addl	%r12d, \h		/* h += Majority(a,b,c) */
	.endm

	.macro	ROUND	a, b, c, d, e, f, g, h, i
	EXPAND	"((\i*4)%64)", "(((\i-2)*4)%64)", "(((\i-15)*4)%64)", "(((\i-7)*4)%64)"
	RND		\a, \b, \c, \d, \e, \f, \g, \h, "(\i*4)", %r15d
	.endm

	.macro	RND_NOEXP	a, b, c, d, e, f, g, h, k, data
	RND		\a, \b, \c, \d, \e, \f, \g, \h, \k, \data(%rsp)
	.endm

	.macro	ROUND_NOEXP	a, b, c, d, e, f, g, h, i
	RND_NOEXP	\a, \b, \c, \d, \e, \f, \g, \h, "(\i*4)", "((\i*4)%64)"
	.endm

	.text
	.globl	_crypto_sha256_compress_x64
	.def	_crypto_sha256_compress_x64;	.scl	2;	.type	32;	.endef
	/* void _sha256_compress_x64(uint32_t *state [%rdi], */
	/*                           const uint8_t *data [%rsi], */
	/*                           size_t length [%rdx], */
	/*                           const uint32_t *K [%rcx]) */
	.align	16
_crypto_sha256_compress_x64:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$64, %rsp		/* %rsp = W */

	shrq	$6, %rdx	/* length /= 64 */
	jz		.Lend		/* length < 64, so no full block to process */
	movq	%rcx, %rbp	/* K -> rbp */
	movq	%rdx, %r14	/* length -> r14 */
	.align	16
.Ltop:	/* start of loop */
	/* Read input */
	LOAD	0x00
	LOAD	0x10
	LOAD	0x20
	LOAD	0x30
	addq	$64, %rsi	/* offset data pointer */
	/* load state vector */
	movl	0x00(%rdi), %eax
	movl	0x04(%rdi), %ebx
	movl	0x08(%rdi), %ecx
	movl	0x0C(%rdi), %edx
	movl	0x10(%rdi), %r8d
	movl	0x14(%rdi), %r9d
	movl	0x18(%rdi), %r10d
	movl	0x1C(%rdi), %r11d
	/* Begin Processing */
	ROUND_NOEXP	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 0
	ROUND_NOEXP	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 1
	ROUND_NOEXP	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  2
	ROUND_NOEXP	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  3
	ROUND_NOEXP	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  4
	ROUND_NOEXP	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  5
	ROUND_NOEXP	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  6
	ROUND_NOEXP	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  7

	ROUND_NOEXP	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 8
	ROUND_NOEXP	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 9
	ROUND_NOEXP	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  10
	ROUND_NOEXP	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  11
	ROUND_NOEXP	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  12
	ROUND_NOEXP	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  13
	ROUND_NOEXP	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  14
	ROUND_NOEXP	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  15

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 16
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 17
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  18
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  19
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  20
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  21
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  22
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  23

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 24
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 25
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  26
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  27
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  28
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  29
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  30
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  31

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 32
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 33
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  34
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  35
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  36
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  37
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  38
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  39

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 40
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 41
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  42
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  43
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  44
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  45
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  46
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  47

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 48
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 49
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  50
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  51
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  52
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  53
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  54
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  55

	ROUND	%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d, 56
	ROUND	%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d, %r10d, 57
	ROUND	%r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d, %r9d,  58
	ROUND	%r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx, %r8d,  59
	ROUND	%r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx, %edx,  60
	ROUND	%edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx, %ecx,  61
	ROUND	%ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax, %ebx,  62
	ROUND	%ebx, %ecx, %edx, %r8d, %r9d, %r10d,%r11d,%eax,  63

	/* Update the state vector */
	addl	%eax, 0x00(%rdi)
	addl	%ebx, 0x04(%rdi)
	addl	%ecx, 0x08(%rdi)
	addl	%edx, 0x0C(%rdi)
	addl	%r8d, 0x10(%rdi)
	addl	%r9d, 0x14(%rdi)
	addl	%r10d,0x18(%rdi)
	addl	%r11d,0x1C(%rdi)

	decq	%r14	/* --length */
	jnz		.Ltop
.Lend:
	addq	$64, %rsp
	pushq	%rbx
	pushq	%rbp
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	ret

