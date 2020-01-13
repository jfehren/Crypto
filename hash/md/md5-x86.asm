	.file	"md5-x86.asm"

	.macro	ROUND1	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\z, %ebp
	xorl	\y, %ebp
	andl	\x, %ebp
	xorl	\z, %ebp
	addl	\k, \w
	addl	%ebp, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND2	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\y, %ebp
	xorl	\x, %ebp
	andl	\z, %ebp
	xorl	\y, %ebp
	addl	\k, \w
	addl	%ebp, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND3	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\x, %ebp
	xorl	\y, %ebp
	xorl	\z, %ebp
	addl	\k, \w
	addl	%ebp, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND4	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\z, %ebp
	notl	%ebp
	orl		\x, %ebp
	xorl	\y, %ebp
	addl	\k, \w
	addl	%ebp, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.text
	.globl	__crypto_md5_compress_x86
	.def	__crypto_md5_compress_x86;	.scl	2;	.type	32;	.endef
	/* void _md5_compress_x86(uint32_t* state [4(%esp)], */
	/*                        const uint8_t* data [8(%esp)]) */
	.align	16
__crypto_md5_compress_x86:
	/* Clobber Registers */
					/* 24(%esp) - data */
					/* 20(%esp) - state */
					/* 16(%esp) - Return Address */
	pushl	%ebx	/* 12(%esp) */
	pushl	%ebp	/*  8(%esp) */
	pushl	%esi	/*  4(%esp) */
	pushl	%edi	/*   (%esp) */

	/* Load the state vector */
	movl	20(%esp), %ebp
	movl	(%ebp), %eax
	movl	4(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	12(%ebp), %edx

	movl	24(%esp), %esi		/* Load pointer to input data */

	/* Compress Input */

	ROUND1	%eax,%ebx,%ecx,%edx,   (%esi), $0xd76aa478, $7
	ROUND1	%edx,%eax,%ebx,%ecx,  4(%esi), $0xe8c7b756, $12
	ROUND1	%ecx,%edx,%eax,%ebx,  8(%esi), $0x242070db, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 12(%esi), $0xc1bdceee, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 16(%esi), $0xf57c0faf, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 20(%esi), $0x4787c62a, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 24(%esi), $0xa8304613, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 28(%esi), $0xfd469501, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 32(%esi), $0x698098d8, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 36(%esi), $0x8b44f7af, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 40(%esi), $0xffff5bb1, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 44(%esi), $0x895cd7be, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 48(%esi), $0x6b901122, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 52(%esi), $0xfd987193, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 56(%esi), $0xa679438e, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 60(%esi), $0x49b40821, $22

	ROUND2	%eax,%ebx,%ecx,%edx,  4(%esi), $0xf61e2562, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 24(%esi), $0xc040b340, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 44(%esi), $0x265e5a51, $14
	ROUND2	%ebx,%ecx,%edx,%eax,   (%esi), $0xe9b6c7aa, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 20(%esi), $0xd62f105d, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 40(%esi), $0x02441453, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 60(%esi), $0xd8a1e681, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 16(%esi), $0xe7d3fbc8, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 36(%esi), $0x21e1cde6, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 56(%esi), $0xc33707d6, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 12(%esi), $0xf4d50d87, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 32(%esi), $0x455a14ed, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 52(%esi), $0xa9e3e905, $5
	ROUND2	%edx,%eax,%ebx,%ecx,  8(%esi), $0xfcefa3f8, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 28(%esi), $0x676f02d9, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 48(%esi), $0x8d2a4c8a, $20

	ROUND3	%eax,%ebx,%ecx,%edx, 20(%esi), $0xfffa3942, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 32(%esi), $0x8771f681, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 44(%esi), $0x6d9d6122, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 56(%esi), $0xfde5380c, $23
	ROUND3	%eax,%ebx,%ecx,%edx,  4(%esi), $0xa4beea44, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 16(%esi), $0x4bdecfa9, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 28(%esi), $0xf6bb4b60, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 40(%esi), $0xbebfbc70, $23
	ROUND3	%eax,%ebx,%ecx,%edx, 52(%esi), $0x289b7ec6, $4
	ROUND3	%edx,%eax,%ebx,%ecx,   (%esi), $0xeaa127fa, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 12(%esi), $0xd4ef3085, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 24(%esi), $0x04881d05, $23
	ROUND3	%eax,%ebx,%ecx,%edx, 36(%esi), $0xd9d4d039, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 48(%esi), $0xe6db99e5, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 60(%esi), $0x1fa27cf8, $16
	ROUND3	%ebx,%ecx,%edx,%eax,  8(%esi), $0xc4ac5665, $23

	ROUND4	%eax,%ebx,%ecx,%edx,   (%esi), $0xf4292244, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 28(%esi), $0x432aff97, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 56(%esi), $0xab9423a7, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 20(%esi), $0xfc93a039, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 48(%esi), $0x655b59c3, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 12(%esi), $0x8f0ccc92, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 40(%esi), $0xffeff47d, $15
	ROUND4	%ebx,%ecx,%edx,%eax,  4(%esi), $0x85845dd1, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 32(%esi), $0x6fa87e4f, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 60(%esi), $0xfe2ce6e0, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 24(%esi), $0xa3014314, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 52(%esi), $0x4e0811a1, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 16(%esi), $0xf7537e82, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 44(%esi), $0xbd3af235, $10
	ROUND4	%ecx,%edx,%eax,%ebx,  8(%esi), $0x2ad7d2bb, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 36(%esi), $0xeb86d391, $21

	/* Update the state vector */
	movl	20(%esp), %ebp
	addl	%eax, (%ebp)
	addl	%ebx, 4(%ebp)
	addl	%ecx, 8(%ebp)
	addl	%edx, 12(%ebp)

	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret

