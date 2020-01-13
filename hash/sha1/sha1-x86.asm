	.file	"sha1-x86.asm"

	.macro	EXPAND	i, j, k, l
	movl	\i(%esp), %edi
	xorl	\j(%esp), %edi
	xorl	\k(%esp), %edi
	xorl	\l(%esp), %edi
	roll	$1, %edi
	movl	%edi, \i(%esp)
	.endm

	.macro F1	a, b, c, d, e, i
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %esi
	xorl	\c, %esi
	andl	\b, %esi
	xorl	\d, %esi
	roll	$30, \b
	leal	0x5A827999(%edi,\e), \e
	movl	\a, %edi
	roll	$5, %edi
	addl	%edi, \e
	addl	%esi, \e
	.endm

	.macro F1_NOEXP	a, b, c, d, e, i
	movl	\d, %esi
	xorl	\c, %esi
	movl	\a, %edi
	andl	\b, %esi
	addl	\i(%esp), \e
	xorl	\d, %esi
	addl	%esi, \e
	roll	$30, \b
	roll	$5, %edi
	leal	0x5A827999(%edi,\e), \e
	.endm

	.macro F2	a, b, c, d, e, i, k
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %esi
	xorl	\c, %esi
	xorl	\b, %esi
	roll	$30, \b
	leal	\k(%edi,\e), \e
	movl	\a, %edi
	roll	$5, %edi
	addl	%edi, \e
	addl	%esi, \e
	.endm

	.macro F3	a, b, c, d, e, i
	EXPAND	\i, "((\i-56) % 64)", "((\i-32) % 64)", "((\i-12) % 64)"
	movl	\d, %esi
	andl	\c, %esi
 	leal	0x8F1BBCDC(%edi,\e), \e
	movl	\d, %edi
	xorl	\c, %edi
	andl	\b, %edi
	addl	%esi, \e
	roll	$30, \b
	movl	\a, %esi
	roll	$5, %esi
	addl	%edi, \e
	addl	%esi, \e
	.endm

	.macro	ROUND_F1	a, b, c, d, e, i
	F1		\a, \b, \c, \d, \e, "((\i*4) % 64)"
	.endm

	.macro	ROUND_F1_NOEXP	a, b, c, d, e, i
	F1_NOEXP	\a, \b, \c, \d, \e, "((\i*4) % 64)"
	.endm

	.macro	ROUND_F2	a, b, c, d, e, i
	F2		\a, \b, \c, \d, \e, "((\i*4) % 64)", 0x6ED9EBA1
	.endm

	.macro	ROUND_F3	a, b, c, d, e, i
	F3		\a, \b, \c, \d, \e, "((\i*4) % 64)"
	.endm

	.macro	ROUND_F4	a, b, c, d, e, i
	F2		\a, \b, \c, \d, \e, "((\i*4) % 64)", 0xCA62C1D6
	.endm

	.text
	.globl	__crypto_sha1_compress_x86
	.def	__crypto_sha1_compress_x86;	.scl	2;	.type	32;	.endef
	/* void _sha1_compress_x86(uint32_t *state [4(%esp)], */
	/*                         const uint8_t *data [4(%esp)], */
	/*                         size_t length [12(%esp)]) */
	.align	16
__crypto_sha1_compress_x86:
	/* Clobber Registers */
	/*			92(%esp)  length */
	/*			88(%esp)  data */
	/*			84(%esp)  state */
	/*			80(%esp)  Return address */
	pushl	%ebx		/* 76(%esp) */
	pushl	%ebp		/* 72(%esp) */
	pushl	%esi		/* 68(%esp) */
	pushl	%edi		/* 64(%esp) */
	subl	$64, %esp	/* %esp = W */

	shrl	$6, 92(%esp)	/* length /= 64 */
	jz		.Lend		/* length < 64, so no full block to process */
	.align	16
.Ltop:	/* start of loop */
	movl	88(%esp), %esi	/* get data pointer */
	movl	84(%esp), %edi	/* get state pointer */
	/* Read input */
	movl	0x00(%esi), %eax
	movl	0x04(%esi), %ebx
	movl	0x08(%esi), %ecx
	movl	0x0C(%esi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, 0x00(%esp)
	movl	%ebx, 0x04(%esp)
	movl	%ecx, 0x08(%esp)
	movl	%edx, 0x0C(%esp)
	movl	0x10(%esi), %eax
	movl	0x14(%esi), %ebx
	movl	0x18(%esi), %ecx
	movl	0x1C(%esi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, 0x10(%esp)
	movl	%ebx, 0x14(%esp)
	movl	%ecx, 0x18(%esp)
	movl	%edx, 0x1C(%esp)
	movl	0x20(%esi), %eax
	movl	0x24(%esi), %ebx
	movl	0x28(%esi), %ecx
	movl	0x2C(%esi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, 0x20(%esp)
	movl	%ebx, 0x24(%esp)
	movl	%ecx, 0x28(%esp)
	movl	%edx, 0x2C(%esp)
	movl	0x30(%esi), %eax
	movl	0x34(%esi), %ebx
	movl	0x38(%esi), %ecx
	movl	0x3C(%esi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, 0x30(%esp)
	movl	%ebx, 0x34(%esp)
	movl	%ecx, 0x38(%esp)
	movl	%edx, 0x3C(%esp)
	addl	$64, 88(%esp)		/* offset data pointer */
	/* load the state vector */
	movl	0x00(%edi), %eax
	movl	0x04(%edi), %ebx
	movl	0x08(%edi), %ecx
	movl	0x0C(%edi), %edx
	movl	0x10(%edi), %ebp
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
	movl	84(%esp), %edi	/* get state pointer */
	addl	%eax, 0x00(%edi)
	addl	%ebx, 0x04(%edi)
	addl	%ecx, 0x08(%edi)
	addl	%edx, 0x0C(%edi)
	addl	%ebp, 0x10(%edi)

	decl	92(%esp)	/* --length */
	jnz		.Ltop
.Lend:
	/* Restore clobbered registers and return */
	addl	$64, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret

