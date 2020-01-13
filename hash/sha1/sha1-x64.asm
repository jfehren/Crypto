	.file	"sha1-x64.asm"

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

	.macro	EXPAND	i, j, k, l
	movl	\i(%rsp), %r9d
	xorl	\j(%rsp), %r9d
	xorl	\k(%rsp), %r9d
	xorl	\l(%rsp), %r9d
	roll	$1, %r9d
	movl	%r9d, \i(%rsp)
	.endm

	.macro F1	a, b, c, d, e, i, k
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %r10d
	xorl	\e, %r10d
	andl	\b, %r10d
	xorl	\d, %r10d
	roll	$30, \b
	addl	%r9d, \e
	addl	\k, \e
	movl	\a, %r9d
	roll	$5, %r9d
	addl	%r9d, \e
	addl	%r10d, \e
	.endm

	.macro F1_NOEXP	a, b, c, d, e, i, k
	movl	\d, %r10d
	xorl	\c, %r10d
	movl	\a, %r9d
	andl	\b, %r10d
	addl	\i(%rsp), \e
	xorl	\d, %r10d
	addl	%r10d, \e
	roll	$30, \b
	roll	$5, %r9d
	addl	%r9d, \e
	addl	\k, \e
	.endm

	.macro F2	a, b, c, d, e, i, k
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %r10d
	xorl	\c, %r10d
	xorl	\b, %r10d
	roll	$30, \b
	addl	%r9d, \e
	addl	\k, \e
	movl	\a, %r9d
	roll	$5, %r9d
	addl	%r9d, \e
	addl	%r10d, \e
	.endm

	.macro F3	a, b, c, d, e, i, k
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %r10d
	andl	\c, %r10d
	addl	%r9d, \e
 	addl	\k, \e
	movl	\d, %r9d
	xorl	\c, %r9d
	andl	\b, %r9d
	addl	%r10d, \e
	roll	$30, \b
	movl	\a, %r10d
	roll	$5, %r10d
	addl	%r9d, \e
	addl	%r10d, \e
	.endm

	.macro	ROUND_F1	a, b, c, d, e, i
	F1		\a, \b, \c, \d, \e, "((\i*4) % 64)", %r11d
	.endm

	.macro	ROUND_F1_NOEXP	a, b, c, d, e, i
	F1_NOEXP	\a, \b, \c, \d, \e, "((\i*4) % 64)", %r11d
	.endm

	.macro	ROUND_F2	a, b, c, d, e, i
	F2		\a, \b, \c, \d, \e, "((\i*4) % 64)", %r12d
	.endm

	.macro	ROUND_F3	a, b, c, d, e, i
	F3		\a, \b, \c, \d, \e, "((\i*4) % 64)", %r13d
	.endm

	.macro	ROUND_F4	a, b, c, d, e, i
	F2		\a, \b, \c, \d, \e, "((\i*4) % 64)", %r14d
	.endm

	.text
	.globl	_crypto_sha1_compress_x64
	.def	_crypto_sha1_compress_x64;	.scl	2;	.type	32;	.endef
	/* void _sha1_compress_x64(uint32_t *state [%rdi], */
	/*                         const uint8_t *data [%rsi], */
	/*                          size_t length [%rdx]) */
	.align	16
_crypto_sha1_compress_x64:
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushq	%rbp
	subq	$64, %rsp	/* %rsp = W */

	shrq	$6, %rdx	/* length /= 64 */
	jz		.Lend
	movq	%rdx, %r8
	movl	$0x5A827999, %r11d	/* load first K value */
	movl	$0x6ED9EBA1, %r12d	/* load second K value */
	movl	$0x8F1BBCDC, %r13d	/* load third K value */
	movl	$0xCA62C1D6, %r14d	/* load fourth K value */
	.align	16
.Ltop:	/* start of loop */
	/* Read input */
	LOAD	0x00
	LOAD	0x10
	LOAD	0x20
	LOAD	0x30
	addl	$64, %rsi		/* offset data pointer */
	/* load the state vector */
	movl	0x00(%rdi), %eax
	movl	0x00(%rdi), %ebx
	movl	0x00(%rdi), %ecx
	movl	0x00(%rdi), %edx
	movl	0x00(%rdi), %ebp
	/* 4 sub-rounds of 20 iterations each */
	/* the first 16 iterations of the first sub-round have no data expansion */
	ROUND_F1_NOEXP	%eax, %ebx, %ecx, %edx, %ebp, 0
	ROUND_F1_NOEXP	%ebp, %eax, %ebx, %ecx, %edx, 1
	ROUND_F1_NOEXP	%edx, %ebp, %eax, %ebx, %ecx, 2
	ROUND_F1_NOEXP	%ecx, %edx, %ebp, %eax, %ebx, 3
	ROUND_F1_NOEXP	%ebx, %ecx, %edx, %ebp, %eax, 4

	ROUND_F1_NOEXP	%eax, %ebx, %ecx, %edx, %ebp, 5
	ROUND_F1_NOEXP	%ebp, %eax, %ebx, %ecx, %edx, 6
	ROUND_F1_NOEXP	%edx, %ebp, %eax, %ebx, %ecx, 7
	ROUND_F1_NOEXP	%ecx, %edx, %ebp, %eax, %ebx, 8
	ROUND_F1_NOEXP	%ebx, %ecx, %edx, %ebp, %eax, 9

	ROUND_F1_NOEXP	%eax, %ebx, %ecx, %edx, %ebp, 10
	ROUND_F1_NOEXP	%ebp, %eax, %ebx, %ecx, %edx, 11
	ROUND_F1_NOEXP	%edx, %ebp, %eax, %ebx, %ecx, 12
	ROUND_F1_NOEXP	%ecx, %edx, %ebp, %eax, %ebx, 13
	ROUND_F1_NOEXP	%ebx, %ecx, %edx, %ebp, %eax, 14

	ROUND_F1_NOEXP	%eax, %ebx, %ecx, %edx, %ebp, 15
	ROUND_F1	%ebp, %eax, %ebx, %ecx, %edx, 16
	ROUND_F1	%edx, %ebp, %eax, %ebx, %ecx, 17
	ROUND_F1	%ecx, %edx, %ebp, %eax, %ebx, 18
	ROUND_F1	%ebx, %ecx, %edx, %ebp, %eax, 19

	ROUND_F2	%eax, %ebx, %ecx, %edx, %ebp, 20
	ROUND_F2	%ebp, %eax, %ebx, %ecx, %edx, 21
	ROUND_F2	%edx, %ebp, %eax, %ebx, %ecx, 22
	ROUND_F2	%ecx, %edx, %ebp, %eax, %ebx, 23
	ROUND_F2	%ebx, %ecx, %edx, %ebp, %eax, 24

	ROUND_F2	%eax, %ebx, %ecx, %edx, %ebp, 25
	ROUND_F2	%ebp, %eax, %ebx, %ecx, %edx, 26
	ROUND_F2	%edx, %ebp, %eax, %ebx, %ecx, 27
	ROUND_F2	%ecx, %edx, %ebp, %eax, %ebx, 28
	ROUND_F2	%ebx, %ecx, %edx, %ebp, %eax, 29

	ROUND_F2	%eax, %ebx, %ecx, %edx, %ebp, 30
	ROUND_F2	%ebp, %eax, %ebx, %ecx, %edx, 31
	ROUND_F2	%edx, %ebp, %eax, %ebx, %ecx, 32
	ROUND_F2	%ecx, %edx, %ebp, %eax, %ebx, 33
	ROUND_F2	%ebx, %ecx, %edx, %ebp, %eax, 34

	ROUND_F2	%eax, %ebx, %ecx, %edx, %ebp, 35
	ROUND_F2	%ebp, %eax, %ebx, %ecx, %edx, 36
	ROUND_F2	%edx, %ebp, %eax, %ebx, %ecx, 37
	ROUND_F2	%ecx, %edx, %ebp, %eax, %ebx, 38
	ROUND_F2	%ebx, %ecx, %edx, %ebp, %eax, 39

	ROUND_F3	%eax, %ebx, %ecx, %edx, %ebp, 40
	ROUND_F3	%ebp, %eax, %ebx, %ecx, %edx, 41
	ROUND_F3	%edx, %ebp, %eax, %ebx, %ecx, 42
	ROUND_F3	%ecx, %edx, %ebp, %eax, %ebx, 43
	ROUND_F3	%ebx, %ecx, %edx, %ebp, %eax, 44

	ROUND_F3	%eax, %ebx, %ecx, %edx, %ebp, 45
	ROUND_F3	%ebp, %eax, %ebx, %ecx, %edx, 46
	ROUND_F3	%edx, %ebp, %eax, %ebx, %ecx, 47
	ROUND_F3	%ecx, %edx, %ebp, %eax, %ebx, 48
	ROUND_F3	%ebx, %ecx, %edx, %ebp, %eax, 49

	ROUND_F3	%eax, %ebx, %ecx, %edx, %ebp, 50
	ROUND_F3	%ebp, %eax, %ebx, %ecx, %edx, 51
	ROUND_F3	%edx, %ebp, %eax, %ebx, %ecx, 52
	ROUND_F3	%ecx, %edx, %ebp, %eax, %ebx, 53
	ROUND_F3	%ebx, %ecx, %edx, %ebp, %eax, 54

	ROUND_F3	%eax, %ebx, %ecx, %edx, %ebp, 55
	ROUND_F3	%ebp, %eax, %ebx, %ecx, %edx, 56
	ROUND_F3	%edx, %ebp, %eax, %ebx, %ecx, 57
	ROUND_F3	%ecx, %edx, %ebp, %eax, %ebx, 58
	ROUND_F3	%ebx, %ecx, %edx, %ebp, %eax, 59

	ROUND_F4	%eax, %ebx, %ecx, %edx, %ebp, 60
	ROUND_F4	%ebp, %eax, %ebx, %ecx, %edx, 61
	ROUND_F4	%edx, %ebp, %eax, %ebx, %ecx, 62
	ROUND_F4	%ecx, %edx, %ebp, %eax, %ebx, 63
	ROUND_F4	%ebx, %ecx, %edx, %ebp, %eax, 64

	ROUND_F4	%eax, %ebx, %ecx, %edx, %ebp, 65
	ROUND_F4	%ebp, %eax, %ebx, %ecx, %edx, 66
	ROUND_F4	%edx, %ebp, %eax, %ebx, %ecx, 67
	ROUND_F4	%ecx, %edx, %ebp, %eax, %ebx, 68
	ROUND_F4	%ebx, %ecx, %edx, %ebp, %eax, 69

	ROUND_F4	%eax, %ebx, %ecx, %edx, %ebp, 70
	ROUND_F4	%ebp, %eax, %ebx, %ecx, %edx, 71
	ROUND_F4	%edx, %ebp, %eax, %ebx, %ecx, 72
	ROUND_F4	%ecx, %edx, %ebp, %eax, %ebx, 73
	ROUND_F4	%ebx, %ecx, %edx, %ebp, %eax, 74

	ROUND_F4	%eax, %ebx, %ecx, %edx, %ebp, 75
	ROUND_F4	%ebp, %eax, %ebx, %ecx, %edx, 76
	ROUND_F4	%edx, %ebp, %eax, %ebx, %ecx, 77
	ROUND_F4	%ecx, %edx, %ebp, %eax, %ebx, 78
	ROUND_F4	%ebx, %ecx, %edx, %ebp, %eax, 79

	/* Update the state vector */
	addl	%eax, 0x00(%rdi)
	addl	%ebx, 0x04(%rdi)
	addl	%ecx, 0x08(%rdi)
	addl	%edx, 0x0C(%rdi)
	addl	%ebp, 0x10(%rdi)

	decq	%r8		/* --length */
	jnz		.Ltop
.Lend:
	/* Restore clobbered registers and return */
	addq	$64, %rsp
	popq	%rbp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	ret

