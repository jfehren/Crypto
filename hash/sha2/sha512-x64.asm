	.file	"sha512-x64.asm"

	.macro LOAD off
	movq	\off+0x00(%rsi), %rax
	movq	\off+0x08(%rsi), %rbx
	movq	\off+0x10(%rsi), %rcx
	movq	\off+0x18(%rsi), %rdx
	bswapq	%rax
	bswapq	%rbx
	bswapq	%rcx
	bswapq	%rdx
	movq	%rax, \off+0x00(%rsp)
	movq	%rbx, \off+0x08(%rsp)
	movq	%rcx, \off+0x10(%rsp)
	movq	%rdx, \off+0x18(%rsp)
	.endm

	/* W[i] = W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) */
	.macro	EXPAND	i, j, k, l
	movq	\i(%rsp), %r15		/* W[i-16] -> r15 */
	/* s1(W[i-2]) = (W[i-2] <<< 45) ^ (W[i-2] <<< 3) ^ (W[i-2] >> 6) */
	movq	\j(%rsp), %r12		/* W[i-2] -> r12 */
	movq	%r12d, %r13			/* W[i-2] -> r13 */
	shrq	$6, %r12			/* (W[i-2] >> 6) -> r12 */
	rolq	$3, %r13			/* (W[i-2] <<< 3) -> r13 */
	xorq	%r13, %r12			/* (W[i-2] <<< 3) ^ (W[i-2] >> 6) -> r12 */
	rolq	$42, %r13			/* (W[i-2] <<< 45) -> r13 */
	xorq	%r13, %r12			/* s1(W[i-2]) -> r12 */
	addq	%r12, %r15			/* W[i-16] + s1(W[i-2]) -> r15 */
	/* s0(W[i-15]) = (W[i-15] <<< 63) ^ (W[i-15] <<< 56) ^ (W[i-15] >> 7) */
	movq	\k(%rsp), %r12		/* W[i-15] -> r12 */
	movq	%r12, %r13			/* W[i-15] -> r13 */
	shrq	$7, %r12			/* (W[i-15] >> 7) -> r12 */
	rolq	$56, %r13			/* (W[i-15] <<< 56) -> r13 */
	xorq	%r13, %r12			/* (W[i-15] <<< 56) ^ (W[i-15] >> 7) -> r12 */
	rolq	$7, %r13			/* (W[i-15] <<< 63) -> r13 */
	xorq	%r13, %r12			/* s0(W[i-15]) -> r12 */
	addq	%r12, %r15			/* W[i-16] + s1(W[i-2]) + s0(W[i-15]) -> r15 */
	addq	\l(%rsp), %r15		/* W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) -> r15 */
	movq	%r15, \i(%rsp)		/* W[i-16] + s1(W[i-2]) + W[i-7] + s0(W[i-15]) -> W[i] */
	.endm

	/* h += S1(e) + Choice(e,f,g) + k + data [%eax] */
	/* d += h */
	/* h += S0(a) + Majority(a,b,c) */
	.macro	RND	a, b, c, d, e, f, g, h, k, data
	/* S1(e) = (e <<< 50) ^ (e <<< 46) ^ (e <<< 23) */
	movq	\e, %r12		/* e -> r12 */
	movq	\e, %r13		/* e -> r13 */
	rolq	$23, %r12		/* (e <<< 23) -> r12 */
	rolq	$46, %r13		/* (e <<< 46) -> r13 */
	addq	\data, \h		/* h += data */
	xorq	%r12, %r13		/* (e <<< 46) ^ (e <<< 23) -> r13 */
	rolq	$27, %r12		/* (e <<< 50) -> r12 */
	xorq	%r12, %r13		/* S1(e) -> r13 */
	addq	%r13, \h		/* h += S1(e) */
	/* Choice(e,f,g) = ((g ^ f) & e) ^ g */
	movq	\g, %r12		/* g -> r12 */
	xorq	\f, %r12		/* (g ^ f) -> r12 */
	addq	\k(%rbp), \h	/* h += k */
	andq	\e, %r12		/* ((g ^ f) & e) -> r12 */
	xorq	\g, %r12		/* Choice(e,f,g) -> r12 */
	addq	%r12, \h		/* h += Choice(e,f,g) */
	/* S0(a) = (a <<< 36) ^ (a <<< 30) ^ (a <<< 25) */
	movq	\a, %r12		/* a -> r12 */
	movq	\a, %r13		/* a -> r13 */
	rolq	$25, %r12		/* (a <<< 25) -> r12 */
	rolq	$30, %r13		/* (a <<< 30) -> r13 */
	addq	\h, \d			/* d += h */
	xorq	%r12, %r13		/* (a <<< 30) ^ (a <<< 25) -> r13 */
	rolq	$11, %r12		/* (a <<< 36) -> r12 */
	xorq	%r12, %r13		/* S0(a) -> r13 */
	addq	%r13, \h		/* h += S0(a) */
	/* Majority(a,b,c) = (a & b) + ((a ^ b) & c) */
	movq	\a, %r12		/* a -> r12 */
	movq	\a, %r13		/* a -> r13 */
	andq	\b, %r12		/* (a & b) -> r12 */
	xorq	\b, %r13		/* (a ^ b) -> r13 */
	andq	\c, %r13		/* ((a ^ b) & c) -> r13 */
	xorq	%r13, %r12		/* Majority(a,b,c) -> r12 */
	addq	%r12, \h		/* h += Majority(a,b,c) */
	.endm

	.macro	ROUND	a, b, c, d, e, f, g, h, i
	EXPAND	"((\i*8)%128)", "(((\i-2)*8)%128)", "(((\i-15)*8)%128)", "(((\i-7)*8)%128)"
	RND		\a, \b, \c, \d, \e, \f, \g, \h, "(\i*8)", %r15
	.endm

	.macro	RND_NOEXP	a, b, c, d, e, f, g, h, k, data
	RND		\a, \b, \c, \d, \e, \f, \g, \h, \k, \data(%rsp)
	.endm

	.macro	ROUND_NOEXP	a, b, c, d, e, f, g, h, i
	RND_NOEXP	\a, \b, \c, \d, \e, \f, \g, \h, "(\i*8)", "((\i*8)%128)"
	.endm

	.text
	.globl	_crypto_sha512_compress_x64
	.def	_crypto_sha512_compress_x64;	.scl	2;	.type	32;	.endef
	/* void _sha512_compress_x64(uint64_t *state [%rdi], */
	/*                           const uint8_t *data [%rsi], */
	/*                           size_t length [%rdx], */
	/*                           const uint64_t *K [%rcx]) */
	.align	16
_crypto_sha512_compress_x64:
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$128, %rsp		/* %rsp = W */

	shrq	$7, %rdx	/* length /= 128 */
	jz		.Lend		/* length < 128, so no full block to process */
	movq	%rcx, %rbp	/* K -> rbp */
	movq	%rdx, %r14	/* length -> r14 */
	.align	16
.Ltop:	/* start of loop */
	/* Read input */
	LOAD	0x00
	LOAD	0x20
	LOAD	0x40
	LOAD	0x60
	addq	$128, %rsi	/* offset data pointer */
	/* load state vector */
	movq	0x00(%rdi), %eax
	movq	0x08(%rdi), %ebx
	movq	0x10(%rdi), %ecx
	movq	0x18(%rdi), %edx
	movq	0x20(%rdi), %r8d
	movq	0x28(%rdi), %r9d
	movq	0x30(%rdi), %r10d
	movq	0x38(%rdi), %r11d
	/* Begin Processing */
	ROUND_NOEXP	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 0
	ROUND_NOEXP	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 1
	ROUND_NOEXP	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  2
	ROUND_NOEXP	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  3
	ROUND_NOEXP	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 4
	ROUND_NOEXP	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 5
	ROUND_NOEXP	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 6
	ROUND_NOEXP	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 7

	ROUND_NOEXP	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 8
	ROUND_NOEXP	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 9
	ROUND_NOEXP	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  10
	ROUND_NOEXP	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  11
	ROUND_NOEXP	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 12
	ROUND_NOEXP	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 13
	ROUND_NOEXP	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 14
	ROUND_NOEXP	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 15

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 16
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 17
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  18
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  19
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 20
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 21
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 22
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 23

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 24
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 25
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  26
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  27
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 28
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 29
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 30
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 31

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 32
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 33
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  34
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  35
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 36
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 37
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 38
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 39

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 40
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 41
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  42
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  43
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 44
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 45
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 46
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 47

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 48
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 49
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  50
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  51
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 52
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 53
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 54
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 55

	ROUND	%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11, 56
	ROUND	%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9, %r10, 57
	ROUND	%r10,%r11,%rax,%rbx,%rcx,%rdx,%r8, %r9,  58
	ROUND	%r9, %r10,%r11,%rax,%rbx,%rcx,%rdx,%r8,  59
	ROUND	%r8, %r9, %r10,%r11,%rax,%rbx,%rcx,%rdx, 60
	ROUND	%rdx,%r8, %r9, %r10,%r11,%rax,%rbx,%rcx, 61
	ROUND	%rcx,%rdx,%r8, %r9, %r10,%r11,%rax,%rbx, 62
	ROUND	%rbx,%rcx,%rdx,%r8, %r9, %r10,%r11,%rax, 63

	/* Update the state vector */
	addq	%rax, 0x00(%rdi)
	addq	%rbx, 0x08(%rdi)
	addq	%rcx, 0x10(%rdi)
	addq	%rdx, 0x18(%rdi)
	addq	%r8,  0x20(%rdi)
	addq	%r9,  0x28(%rdi)
	addq	%r10, 0x30(%rdi)
	addq	%r11, 0x38(%rdi)

	decq	%r14	/* --length */
	jnz		.Ltop
.Lend:
	addq	$128, %rsp
	pushq	%rbx
	pushq	%rbp
	pushq	%r12
	pushq	%r13
	pushq	%r14
	pushq	%r15
	ret

