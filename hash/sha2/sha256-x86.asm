	.file	"sha256-x86.asm"
	.text

	.macro LOAD off
	movl	\off+0x00(%esi), %eax
	movl	\off+0x04(%esi), %ebx
	movl	\off+0x08(%esi), %ecx
	movl	\off+0x0C(%esi), %edx
	bswapl	%eax
	bswapl	%ebx
	bswapl	%ecx
	bswapl	%edx
	movl	%eax, \off+0x00(%esp)
	movl	%ebx, \off+0x04(%esp)
	movl	%ecx, \off+0x08(%esp)
	movl	%edx, \off+0x0C(%esp)
	.endm

	.macro	EXPAND	i, j, k, l
	movl	\i(%esp), %eax
	movl	\j(%esp), %ebx
	movl	\k(%esp), %edx
	movl	%ebx, %ecx
	movl	%edx, %edi
	shrl	$10, %ebx
	shrl	$3, %edx
	roll	$13, %ecx
	roll	$14, %edi
	xorl	%ecx, %ebx
	xorl	%edi, %edx
	roll	$2, %ecx
	roll	$11, %edi
	xorl	%ecx, %ebx
	xorl	%edi, %edx
	addl	\l(%esp), %eax
	addl	%ebx, %eax
	addl	%edx, %eax
	movl	%eax, \i(%esp)
	.endm

	.macro	NOEXPND	i
	movl	\i(%esp), %eax
	.endm

	/* h += S1(e) + Choice(e,f,g) + k + data [%eax] */
	/* d += h */
	/* h += S0(a) + Majority(a,b,c) */
	.macro	RND	a, b, c, d, e, f, g, h, k
	movl	\h+64(%esp), %edi
	addl	\k(%ebp), %edi		/* h += k */
	/* S1(e) = (e <<< 26) ^ (e <<< 21) ^ (e <<< 7) */
	movl	\e+64(%esp), %edx	/* e -> edx */
	movl	%edx, %ebx			/* e -> ebx */
	movl	%edx, %ecx			/* e -> ecx */
	addl	%eax, %edi			/* h += data */
	roll	$7, %ebx			/* (e <<< 7) -> ebx */
	roll	$21, %ecx			/* (e <<< 21) -> ecx */
	xorl	%ebx, %ecx			/* (e <<< 21) ^ (e <<< 7) -> ecx */
	roll	$19, %ebx			/* (e <<< 26) -> ebx */
	xorl	%ebx, %ecx			/* S1(e) -> ecx */
	addl	%ecx, %edi			/* h += S1(e) */
	/* Choice(e,f,g) = ((g ^ f) & e) ^ g */
	movl	\g+64(%esp), %ebx	/* g -> ebx */
	movl	%ebx, %ecx			/* g -> ecx */
	xorl	\f+64(%esp), %ebx	/* (g ^ f) -> ebx */
	andl	%edx, %ebx			/* ((g ^ f) & e) -> ebx */
	xorl	%ecx, %ebx			/* Choice(e,f,g) -> ebx */
	addl	%ebx, %edi			/* h += Choice(e,f,g) */

	addl	%edi, \d+64(%esp)	/* d += h */
	/* S0(a) = (a <<< 30) ^ (a <<< 19) ^ (a <<< 10) */
	movl	\a+64(%esp), %edx	/* a -> edx */
	movl	%edx, %ebx			/* a -> ebx */
	movl	%edx, %ecx			/* a -> ecx */
	roll	$10, %ebx			/* (a <<< 10) -> ebx */
	roll	$19, %ecx			/* (a <<< 19) -> ecx */
	xorl	%ebx, %ecx			/* (a <<< 19) ^ (a <<< 10) -> ecx */
	roll	$20, %ebx			/* (a <<< 30) -> ebx */
	xorl	%ebx, %ecx			/* S0(a) -> ecx */
	addl	%ecx, %edi			/* h += S0(a) */
	/* Majority(a,b,c) = (a & b) ^ ((a ^ b) & c) */
	movl	\b+64(%esp), %ebx	/* b -> ebx */
	movl	%edx, %eax			/* a -> eax */
	xorl	%ebx, %edx			/* (a ^ b) -> edx */
	andl	%ebx, %eax			/* (a & b) -> eax */
	andl	\c+64(%esp), %edx	/* ((a ^ b) & c) -> edx */
	xorl	%eax, %edx			/* Majority(a,b,c) -> edx */
	addl	%edx, %edi			/* h += Majority(a,b,c) */
	movl	%edi, \h+64(%esp)
	.endm

	.macro ROUND	a, b, c, d, e, f, g, h, i
	EXPAND	"((\i*4)%64)", "(((\i-2)*4)%64)", "(((\i-15)*4)%64)", "(((\i-7)*4)%64)"
	RND		"(\a*4)","(\b*4)","(\c*4)","(\d*4)","(\e*4)","(\f*4)","(\g*4)","(\h*4)","(\i*4)"
	.endm

	.macro	ROUND_NOEXP	a, b, c, d, e, f, g, h, i
	NOEXPND	"((\i*4)%64)"
	RND		"(\a*4)","(\b*4)","(\c*4)","(\d*4)","(\e*4)","(\f*4)","(\g*4)","(\h*4)","(\i*4)"
	.endm

	.text
	.globl	__crypto_sha256_compress_x86
	.def	__crypto_sha256_compress_x86;	.scl	2;	.type	32;	.endef
	/* void _sha256_compress_x86(uint32_t *state [4(%esp)], */
	/*                           const uint8_t *data [8(%esp)], */
	/*                           size_t length [12(%esp)], */
	/*                           const uint32_t *K [16(%esp)]) */
	.align	16
__crypto_sha256_compress_x86:
	/* Clobber Registers */
	/*			128(%esp)	K */
	/*			124(%esp)	length */
	/*			120(%esp)	data */
	/*			116(%esp)	state */
	/*			112(%esp)	Return Address */
	pushl	%edi		/* 108(%esp) */
	pushl	%esi		/* 104(%esp) */
	pushl	%ebp		/* 100(%esp) */
	pushl	%ebx		/* 96(%esp) */
	subl	$96, %esp	/* %esp = W, %esp + 64 = S */

	shrl	$6, 124(%esp)	/* length /= 64 */
	jz		.Lend		/* length < 64, so no full block to process */

	movl	128(%esp), %ebp	/* K -> ebp */
	.align	16
.Ltop:	/* start of loop */
	movl	120(%esp), %esi		/* get data pointer */
	/* Read input */
	LOAD	0
	LOAD	16
	LOAD	32
	LOAD	48
	addl	$64, 120(%esp)		/* offset data pointer */
	movl	116(%esp), %esi		/* get state pointer */
	/* load the state vector */
	movl	0x00(%esi), %eax
	movl	0x04(%esi), %ebx
	movl	0x08(%esi), %ecx
	movl	0x0C(%esi), %edx
	movl	%eax, 64(%esp)		/* 64(%esp) = A */
	movl	%ebx, 68(%esp)		/* 68(%esp) = B */
	movl	%ecx, 72(%esp)		/* 72(%esp) = C */
	movl	%edx, 76(%esp)		/* 76(%esp) = D */
	movl	0x10(%esi), %eax
	movl	0x14(%esi), %ebx
	movl	0x18(%esi), %ecx
	movl	0x1C(%esi), %edx
	movl	%eax, 80(%esp)		/* 80(%esp) = E */
	movl	%ebx, 84(%esp)		/* 84(%esp) = F */
	movl	%ecx, 88(%esp)		/* 88(%esp) = G */
	movl	%edx, 92(%esp)		/* 92(%esp) = H */
	/* Begin Processing */
	ROUND_NOEXP	0, 1, 2, 3, 4, 5, 6, 7,		0
	ROUND_NOEXP	7, 0, 1, 2, 3, 4, 5, 6,		1
	ROUND_NOEXP	6, 7, 0, 1, 2, 3, 4, 5,		2
	ROUND_NOEXP	5, 6, 7, 0, 1, 2, 3, 4,		3
	ROUND_NOEXP	4, 5, 6, 7, 0, 1, 2, 3,		4
	ROUND_NOEXP	3, 4, 5, 6, 7, 0, 1, 2,		5
	ROUND_NOEXP	2, 3, 4, 5, 6, 7, 0, 1,		6
	ROUND_NOEXP	1, 2, 3, 4, 5, 6, 7, 0,		7

	ROUND_NOEXP	0, 1, 2, 3, 4, 5, 6, 7,		8
	ROUND_NOEXP	7, 0, 1, 2, 3, 4, 5, 6,		9
	ROUND_NOEXP	6, 7, 0, 1, 2, 3, 4, 5,		10
	ROUND_NOEXP	5, 6, 7, 0, 1, 2, 3, 4,		11
	ROUND_NOEXP	4, 5, 6, 7, 0, 1, 2, 3,		12
	ROUND_NOEXP	3, 4, 5, 6, 7, 0, 1, 2,		13
	ROUND_NOEXP	2, 3, 4, 5, 6, 7, 0, 1,		14
	ROUND_NOEXP	1, 2, 3, 4, 5, 6, 7, 0,		15

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		16
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		17
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		18
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		19
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		20
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		21
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		22
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		23

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		24
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		25
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		26
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		27
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		28
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		29
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		30
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		31

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		32
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		33
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		34
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		35
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		36
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		37
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		38
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		39

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		40
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		41
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		42
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		43
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		44
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		45
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		46
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		47

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		48
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		49
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		50
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		51
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		52
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		53
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		54
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		55

	ROUND	0, 1, 2, 3, 4, 5, 6, 7,		56
	ROUND	7, 0, 1, 2, 3, 4, 5, 6,		57
	ROUND	6, 7, 0, 1, 2, 3, 4, 5,		58
	ROUND	5, 6, 7, 0, 1, 2, 3, 4,		59
	ROUND	4, 5, 6, 7, 0, 1, 2, 3,		60
	ROUND	3, 4, 5, 6, 7, 0, 1, 2,		61
	ROUND	2, 3, 4, 5, 6, 7, 0, 1,		62
	ROUND	1, 2, 3, 4, 5, 6, 7, 0,		63

	/* Update the state vector */
	movl	64(%esp), %eax
	movl	68(%esp), %ebx
	movl	72(%esp), %ecx
	movl	76(%esp), %edx
	addl	%eax, 0x00(%esi)	/* A += a */
	addl	%ebx, 0x04(%esi)	/* B += b */
	addl	%ecx, 0x08(%esi)	/* C += c */
	addl	%edx, 0x0C(%esi)	/* D += d */
	movl	80(%esp), %eax
	movl	84(%esp), %ebx
	movl	88(%esp), %ecx
	movl	92(%esp), %edx
	addl	%eax, 0x10(%esi)	/* E += e */
	addl	%ebx, 0x14(%esi)	/* F += f */
	addl	%ecx, 0x18(%esi)	/* G += g */
	addl	%edx, 0x1C(%esi)	/* H += h */

	decl	124(%esp)		/* --length */
	jnz		.Ltop
.Lend:
	addl	$96, %esp
	popl	%ebx
	popl	%ebp
	popl	%esi
	popl	%edi
	ret

