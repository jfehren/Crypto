	.file	"md5-x64.asm"

	.macro	ROUND1	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\z, %r8d
	xorl	\y, %r8d
	andl	\x, %r8d
	xorl	\z, %r8d
	addl	\k, \w
	addl	%r8d, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND2	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\y, %r8d
	xorl	\x, %r8d
	andl	\z, %r8d
	xorl	\y, %r8d
	addl	\k, \w
	addl	%r8d, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND3	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\x, %r8d
	xorl	\y, %r8d
	xorl	\z, %r8d
	addl	\k, \w
	addl	%r8d, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.macro	ROUND4	w,x,y,z, k, data, s
	addl	\data, \w
	movl	\z, %r8d
	notl	%r8d
	orl		\x, %r8d
	xorl	\y, %r8d
	addl	\k, \w
	addl	%r8d, \w
	roll	\s, \w
	addl	\x, \w
	.endm

	.text
	.globl	_crypto_md5_compress_x64
	.def	_crypto_md5_compress_x64;	.scl	2;	.type	32;	.endef
	/* void _md5_compress_x64(uint32_t* state [%rdi], */
	/*                        const uint8_t* data [%rsi]) */
	.align	16
_crypto_md5_compress_x64:
	pushq	%rbx

	/* Load state vector */
	movl	(%rdi), %eax
	movl	4(%rdi), %ebx
	movl	8(%rdi), %ecx
	movl	12(%rdi), %edx

	/* Compress Input */

	ROUND1	%eax,%ebx,%ecx,%edx,   (%rsi), $0xd76aa478, $7
	ROUND1	%edx,%eax,%ebx,%ecx,  4(%rsi), $0xe8c7b756, $12
	ROUND1	%ecx,%edx,%eax,%ebx,  8(%rsi), $0x242070db, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 12(%rsi), $0xc1bdceee, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 16(%rsi), $0xf57c0faf, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 20(%rsi), $0x4787c62a, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 24(%rsi), $0xa8304613, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 28(%rsi), $0xfd469501, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 32(%rsi), $0x698098d8, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 36(%rsi), $0x8b44f7af, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 40(%rsi), $0xffff5bb1, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 44(%rsi), $0x895cd7be, $22
	ROUND1	%eax,%ebx,%ecx,%edx, 48(%rsi), $0x6b901122, $7
	ROUND1	%edx,%eax,%ebx,%ecx, 52(%rsi), $0xfd987193, $12
	ROUND1	%ecx,%edx,%eax,%ebx, 56(%rsi), $0xa679438e, $17
	ROUND1	%ebx,%ecx,%edx,%eax, 60(%rsi), $0x49b40821, $22

	ROUND2	%eax,%ebx,%ecx,%edx,  4(%rsi), $0xf61e2562, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 24(%rsi), $0xc040b340, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 44(%rsi), $0x265e5a51, $14
	ROUND2	%ebx,%ecx,%edx,%eax,   (%rsi), $0xe9b6c7aa, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 20(%rsi), $0xd62f105d, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 40(%rsi), $0x02441453, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 60(%rsi), $0xd8a1e681, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 16(%rsi), $0xe7d3fbc8, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 36(%rsi), $0x21e1cde6, $5
	ROUND2	%edx,%eax,%ebx,%ecx, 56(%rsi), $0xc33707d6, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 12(%rsi), $0xf4d50d87, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 32(%rsi), $0x455a14ed, $20
	ROUND2	%eax,%ebx,%ecx,%edx, 52(%rsi), $0xa9e3e905, $5
	ROUND2	%edx,%eax,%ebx,%ecx,  8(%rsi), $0xfcefa3f8, $9
	ROUND2	%ecx,%edx,%eax,%ebx, 28(%rsi), $0x676f02d9, $14
	ROUND2	%ebx,%ecx,%edx,%eax, 48(%rsi), $0x8d2a4c8a, $20

	ROUND3	%eax,%ebx,%ecx,%edx, 20(%rsi), $0xfffa3942, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 32(%rsi), $0x8771f681, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 44(%rsi), $0x6d9d6122, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 56(%rsi), $0xfde5380c, $23
	ROUND3	%eax,%ebx,%ecx,%edx,  4(%rsi), $0xa4beea44, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 16(%rsi), $0x4bdecfa9, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 28(%rsi), $0xf6bb4b60, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 40(%rsi), $0xbebfbc70, $23
	ROUND3	%eax,%ebx,%ecx,%edx, 52(%rsi), $0x289b7ec6, $4
	ROUND3	%edx,%eax,%ebx,%ecx,   (%rsi), $0xeaa127fa, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 12(%rsi), $0xd4ef3085, $16
	ROUND3	%ebx,%ecx,%edx,%eax, 24(%rsi), $0x04881d05, $23
	ROUND3	%eax,%ebx,%ecx,%edx, 36(%rsi), $0xd9d4d039, $4
	ROUND3	%edx,%eax,%ebx,%ecx, 48(%rsi), $0xe6db99e5, $11
	ROUND3	%ecx,%edx,%eax,%ebx, 60(%rsi), $0x1fa27cf8, $16
	ROUND3	%ebx,%ecx,%edx,%eax,  8(%rsi), $0xc4ac5665, $23

	ROUND4	%eax,%ebx,%ecx,%edx,   (%rsi), $0xf4292244, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 28(%rsi), $0x432aff97, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 56(%rsi), $0xab9423a7, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 20(%rsi), $0xfc93a039, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 48(%rsi), $0x655b59c3, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 12(%rsi), $0x8f0ccc92, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 40(%rsi), $0xffeff47d, $15
	ROUND4	%ebx,%ecx,%edx,%eax,  4(%rsi), $0x85845dd1, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 32(%rsi), $0x6fa87e4f, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 60(%rsi), $0xfe2ce6e0, $10
	ROUND4	%ecx,%edx,%eax,%ebx, 24(%rsi), $0xa3014314, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 52(%rsi), $0x4e0811a1, $21
	ROUND4	%eax,%ebx,%ecx,%edx, 16(%rsi), $0xf7537e82, $6
	ROUND4	%edx,%eax,%ebx,%ecx, 44(%rsi), $0xbd3af235, $10
	ROUND4	%ecx,%edx,%eax,%ebx,  8(%rsi), $0x2ad7d2bb, $15
	ROUND4	%ebx,%ecx,%edx,%eax, 36(%rsi), $0xeb86d391, $21

	/* Update the state vector */
	addl	%eax, (%rdi)
	addl	%ebx, 4(%rdi)
	addl	%ecx, 8(%rdi)
	addl	%edx, 12(%rdi)

	popq	%rbx
	ret

