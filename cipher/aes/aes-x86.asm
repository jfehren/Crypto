	.file	"aes-x86.asm"

	.macro AES_LOAD a, b, c, d, src, key
	movl	0x0(\src),\a
	movl	0x4(\src),\b
	movl	0x8(\src),\c
	movl	0xC(\src),\d

	xorl	0x0(\key),\a
	xorl	0x4(\key),\b
	xorl	0x8(\key),\c
	xorl	0xC(\key),\d
	.endm

	.macro AES_STORE a, b, c, d, key, dst
	xorl	0x0(\key),\a
	xorl	0x4(\key),\b
	xorl	0x8(\key),\c
	xorl	0xC(\key),\d

	movl	\a,0x0(\dst)
	movl	\b,0x4(\dst)
	movl	\c,0x8(\dst)
	movl	\d,0xC(\dst)
	.endm

	/* Requires that a is one of %al, %bl, %cl, or %dl,
	   and that b is one of %ah, %bh, %ch, %dh */
	.macro AES_ROUND t, a, b, c, d, out, ptr
	movzbl	\a, \ptr
	movl	\t(,\ptr,4), \out
	movzbl	\b, \ptr
	xorl	\t+0x400(,\ptr,4), \out
	movl	\c, \ptr
	shrl	$16, \ptr
	andl	$0xff, \ptr
	xorl	\t+0x800(,\ptr,4), \out
	movl	\d, \ptr
	shrl	$24, \ptr
	xorl	\t+0xC00(,\ptr,4), \out
	.endm

	/* Requires that a is one of %al, %bl, %cl, or %dl */
	.macro AES_FINAL_ROUND a, b, c, d, t, out, tmp
	movzbl	\a, \out
	movzbl	\t(\out), \out
	movl	\b, \tmp
	andl	$0x0000ff00,\tmp
	orl		\tmp, \out
	movl	\c, \tmp
	andl	$0x00ff0000,\tmp
	orl		\tmp, \out
	movl	\d, \tmp
	andl	$0xff000000,\tmp
	orl		\tmp, \out
	roll	$8, \out
	.endm

	/* Requires that al, bl, cl, and dl are the low 8 bits of a, b, c, and d respectively */
	.macro AES_SUB_BYTE_ROTR8 a, b, c, d, al, bl, cl, dl, t, ptr
	movzbl  \al, \ptr
	movb	\t(\ptr), \al
	roll	$8, \a

	movzbl  \bl, \ptr
	movb	\t(\ptr), \bl
	roll	$8, \b

	movzbl  \cl, \ptr
	movb	\t(\ptr), \cl
	roll	$8, \c

	movzbl  \dl, \ptr
	movb	\t(\ptr), \dl
	.endm

	/* Requires that al, bl, cl, and dl are the low 8 bits of a, b, c, and d respectively */
	.macro AES_SUB_BYTE a, b, c, d, al, bl, cl, dl, t, ptr
	AES_SUB_BYTE_ROTR8 \a, \b, \c, \d, \al, \bl, \cl, \dl, \t, \ptr
	roll	$8, \d
	.endm

	.macro AES_INVERT x, xl, xh, out, ptr
	movzbl	\xl, \ptr
	movl	0x000+__crypto_aes_mtable(,\ptr,4), \out
	movzbl	\xh, \ptr
	xorl	0x400+__crypto_aes_mtable(,\ptr,4), \out
	shrl	$16, \x

	movzbl	\xl, \ptr
	xorl	0x800+__crypto_aes_mtable(,\ptr,4), \out
	movzbl	\xh, \ptr
	xorl	0xC00+__crypto_aes_mtable(,\ptr,4), \out
	.endm



	.text
	.globl	__crypto_aes128_set_key_x86
	.def	__crypto_aes128_set_key_x86;	.scl	2;	.type	32;	.endef
	/* void _aes128_set_key_x86(uint32_t* subkeys [4(%esp)], */
	/*                          const uint8_t* key [8(%esp)]) */
__crypto_aes128_set_key_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %edi		/* Load subkeys pointer into edi */
	addl	$0x50, %edi			/* Offset subkeys pointer (to allow for 1-byte offsets) */
	/* Load initial keys */
	movl	24(%esp), %esi		/* Load keys pointer into esi */
	movl	0x0(%esi), %eax
	movl	0x4(%esi), %ebx
	movl	0x8(%esi), %ecx
	movl	0xC(%esi), %edx
	/* Store initial keys */
	movl	%eax, -0x50(%edi)
	movl	%ebx, -0x4C(%edi)
	movl	%ecx, -0x48(%edi)
	movl	%edx, -0x44(%edi)
/* Key Set 1 */
	movl	%edx, %esi		/* Save 4th subkey from last round in esi */
	/* Combined substutute and rotate */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x01, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, -0x40(%edi)
	movl	%ebx, -0x3C(%edi)
	movl	%ecx, -0x38(%edi)
	movl	%esi, -0x34(%edi)
/* Key Set 2 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x02, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, -0x30(%edi)
	movl	%ebx, -0x2C(%edi)
	movl	%ecx, -0x28(%edi)
	movl	%esi, -0x24(%edi)
/* Key Set 3 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x04, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, -0x20(%edi)
	movl	%ebx, -0x1C(%edi)
	movl	%ecx, -0x18(%edi)
	movl	%esi, -0x14(%edi)
/* Key Set 4 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x08, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, -0x10(%edi)
	movl	%ebx, -0x0C(%edi)
	movl	%ecx, -0x08(%edi)
	movl	%esi, -0x04(%edi)
/* Key Set 5 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x10, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x00(%edi)
	movl	%ebx, 0x04(%edi)
	movl	%ecx, 0x08(%edi)
	movl	%esi, 0x0C(%edi)
/* Key Set 6 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x20, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x10(%edi)
	movl	%ebx, 0x14(%edi)
	movl	%ecx, 0x18(%edi)
	movl	%esi, 0x1C(%edi)
/* Key Set 7 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x40, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x20(%edi)
	movl	%ebx, 0x24(%edi)
	movl	%ecx, 0x28(%edi)
	movl	%esi, 0x2C(%edi)
/* Key Set 8 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x80, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x30(%edi)
	movl	%ebx, 0x34(%edi)
	movl	%ecx, 0x38(%edi)
	movl	%esi, 0x3C(%edi)
/* Key Set 9 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x1B, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x40(%edi)
	movl	%ebx, 0x44(%edi)
	movl	%ecx, 0x48(%edi)
	movl	%esi, 0x4C(%edi)
/* Key Set 10 */
	movl	%esi, %edx		/* Move 4th subkey from last round in edx (for byte access) */
	AES_SUB_BYTE_ROTR8 %edx,%edx,%edx,%edx, %dl%,%dl,%dl,%dl, __crypto_aes_sbox, %ebp
	/* Compute Keys */
	xorl	$0x36, %eax			/* Add round constant */
	xorl	%edx, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %esi
	/* Store round keys */
	movl	%eax, 0x50(%edi)
	movl	%ebx, 0x54(%edi)
	movl	%ecx, 0x58(%edi)
	movl	%esi, 0x5C(%edi)
	/* Restore clobbered registers and return */
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes192_set_key_x86
	.def	__crypto_aes1292_set_key_x86;	.scl	2;	.type	32;	.endef
	/* void _aes192_set_key_x86(uint32_t* subkeys [4(%esp)], */
	/*                          const uint8_t* key [8(%esp)]) */
__crypto_aes192_set_key_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %edi		/* Load subkeys pointer into edi */
	addl	$0x60, %edi			/* Offset subkeys pointer (to allow for 1-byte offsets) */
	/* Load initial keys */
	movl	24(%esp), %eax		/* Load keys pointer into eax */
	movl	0x00(%eax), %esi
	movl	0x04(%eax), %ebp
	movl	0x08(%eax), %edx
	movl	0x0C(%eax), %ecx
	movl	0x10(%eax), %ebx
	movl	0x14(%eax), %eax
	/* Store initial keys */
	movl	%esi, -0x60(%edi)
	movl	%ebp, -0x5C(%edi)
	movl	%edx, -0x58(%edi)
	movl	%ecx, -0x54(%edi)
	movl	%ebx, -0x50(%edi)
	movl	%eax, -0x4C(%edi)
/* Key Set 1 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x01, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	-0x58(%edi), %edx
	movl	-0x54(%edi), %ecx
	movl	-0x50(%edi), %ebx
	movl	-0x4C(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, -0x48(%edi)
	movl	%ebp, -0x44(%edi)
	movl	%edx, -0x40(%edi)
	movl	%ecx, -0x3C(%edi)
	movl	%ebx, -0x38(%edi)
	movl	%eax, -0x34(%edi)
/* Key Set 2 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x02, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	-0x40(%edi), %edx
	movl	-0x3C(%edi), %ecx
	movl	-0x38(%edi), %ebx
	movl	-0x34(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, -0x30(%edi)
	movl	%ebp, -0x2C(%edi)
	movl	%edx, -0x28(%edi)
	movl	%ecx, -0x24(%edi)
	movl	%ebx, -0x20(%edi)
	movl	%eax, -0x1C(%edi)
/* Key Set 3 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x04, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	-0x28(%edi), %edx
	movl	-0x24(%edi), %ecx
	movl	-0x10(%edi), %ebx
	movl	-0x1C(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, -0x18(%edi)
	movl	%ebp, -0x14(%edi)
	movl	%edx, -0x10(%edi)
	movl	%ecx, -0x0C(%edi)
	movl	%ebx, -0x08(%edi)
	movl	%eax, -0x04(%edi)
/* Key Set 4 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x08, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	-0x10(%edi), %edx
	movl	-0x0C(%edi), %ecx
	movl	-0x08(%edi), %ebx
	movl	-0x04(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, 0x00(%edi)
	movl	%ebp, 0x04(%edi)
	movl	%edx, 0x08(%edi)
	movl	%ecx, 0x0C(%edi)
	movl	%ebx, 0x10(%edi)
	movl	%eax, 0x14(%edi)
/* Key Set 5 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x10, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	0x08(%edi), %edx
	movl	0x0C(%edi), %ecx
	movl	0x10(%edi), %ebx
	movl	0x14(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, 0x18(%edi)
	movl	%ebp, 0x1C(%edi)
	movl	%edx, 0x20(%edi)
	movl	%ecx, 0x24(%edi)
	movl	%ebx, 0x28(%edi)
	movl	%eax, 0x2C(%edi)
/* Key Set 6 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x20, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	0x20(%edi), %edx
	movl	0x24(%edi), %ecx
	movl	0x28(%edi), %ebx
	movl	0x2C(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, 0x30(%edi)
	movl	%ebp, 0x34(%edi)
	movl	%edx, 0x38(%edi)
	movl	%ecx, 0x3C(%edi)
	movl	%ebx, 0x40(%edi)
	movl	%eax, 0x44(%edi)
/* Key Set 7 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Compute first two of current key set */
	xorl	$0x40, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	/* Load last 4 keys of previous key set */
	movl	0x38(%edi), %edx
	movl	0x3C(%edi), %ecx
	movl	0x40(%edi), %ebx
	movl	0x44(%edi), %eax
	/* Compute remaining key set */
	xorl	%ebp, %edx
	xorl	%edx, %ecx
	xorl	%ecx, %ebx
	xorl	%ebx, %eax
	/* Store key set */
	movl	%esi, 0x48(%edi)
	movl	%ebp, 0x4C(%edi)
	movl	%edx, 0x50(%edi)
	movl	%ecx, 0x54(%edi)
	movl	%ebx, 0x58(%edi)
	movl	%eax, 0x5C(%edi)
/* Key Set 8 */
	/* Combined substitution and rotation */
	movzbl	%al, %edx
	movzbl	%ah, %ecx
	shrl	$16, %eax
	movzbl	__crypto_aes_sbox(%edx), %edx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$24, %edx
	orl		%ecx, %edx
	movzbl	%al, %ebx
	movzbl	%al, %ecx
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	shll	$8, %ebx
	shll	$16, %ecx
	orl		%ebx, %edx
	orl		%ecx, %edx
	/* Load middle 2 keys of previous key set */
	movl	0x20(%edi), %eax
	movl	0x24(%edi), %ebx
	/* Compute final (partial) key set */
	xorl	$0x80, %esi		/* Add round constant */
	xorl	%edx, %esi
	xorl	%esi, %ebp
	xorl	%ebp, %eax
	xorl	%eax, %ebx
	/* Store final (partial) key set */
	movl	%esi, 0x30(%edi)
	movl	%ebp, 0x34(%edi)
	movl	%eax, 0x38(%edi)
	movl	%ebx, 0x3C(%edi)
	/* Restore clobbered registers and return */
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes256_set_key_x86
	.def	__crypto_aes256_set_key_x86;	.scl	2;	.type	32;	.endef
	/* void _aes256_set_key_x86(uint32_t* subkeys [4(%esp)], */
	/*                          const uint8_t* key [8(%esp)]) */
__crypto_aes256_set_key_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	16(%esp), %edi		/* Load subkeys pointer into edi */
	addl	$0x70, %edi			/* Offset subkeys pointer (to allow for 1-byte offsets) */
	/* Copy initial 8 keys */
	movl	20(%esp), %esi		/* Load keys pointer into esi */
	movl	0x00(%esi), %eax
	movl	0x04(%esi), %ebx
	movl	0x08(%esi), %ecx
	movl	0x0C(%esi), %edx
	movl	%eax, -0x70(%edi)
	movl	%ebx, -0x6C(%edi)
	movl	%ecx, -0x68(%edi)
	movl	%edx, -0x64(%edi)
	movl	0x10(%esi), %eax
	movl	0x14(%esi), %ebx
	movl	0x18(%esi), %ecx
	movl	0x1C(%esi), %edx
	movl	%eax, -0x60(%edi)
	movl	%ebx, -0x5C(%edi)
	movl	%ecx, -0x58(%edi)
	movl	%edx, -0x54(%edi)
/* Key Set 1 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	-0x70(%edi), %eax
	movl	-0x6C(%edi), %ebx
	movl	-0x68(%edi), %ecx
	movl	-0x64(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x01, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, -0x50(%edi)
	movl	%ebx, -0x4C(%edi)
	movl	%ecx, -0x48(%edi)
	movl	%edx, -0x44(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	-0x60(%edi), %esi
	movl	-0x5C(%edi), %ebx
	movl	-0x58(%edi), %ecx
	movl	-0x54(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, -0x40(%edi)
	movl	%ebx, -0x3C(%edi)
	movl	%ecx, -0x38(%edi)
	movl	%edx, -0x34(%edi)
/* Key Set 2 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	-0x50(%edi), %eax
	movl	-0x4C(%edi), %ebx
	movl	-0x48(%edi), %ecx
	movl	-0x44(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x02, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, -0x30(%edi)
	movl	%ebx, -0x2C(%edi)
	movl	%ecx, -0x28(%edi)
	movl	%edx, -0x24(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	-0x40(%edi), %esi
	movl	-0x3C(%edi), %ebx
	movl	-0x38(%edi), %ecx
	movl	-0x34(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, -0x20(%edi)
	movl	%ebx, -0x1C(%edi)
	movl	%ecx, -0x18(%edi)
	movl	%edx, -0x14(%edi)
/* Key Set 3 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	-0x30(%edi), %eax
	movl	-0x2C(%edi), %ebx
	movl	-0x28(%edi), %ecx
	movl	-0x24(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x04, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, -0x10(%edi)
	movl	%ebx, -0x0C(%edi)
	movl	%ecx, -0x08(%edi)
	movl	%edx, -0x04(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	-0x20(%edi), %esi
	movl	-0x1C(%edi), %ebx
	movl	-0x18(%edi), %ecx
	movl	-0x14(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, 0x00(%edi)
	movl	%ebx, 0x04(%edi)
	movl	%ecx, 0x08(%edi)
	movl	%edx, 0x0C(%edi)
/* Key Set 4 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	-0x10(%edi), %eax
	movl	-0x0C(%edi), %ebx
	movl	-0x08(%edi), %ecx
	movl	-0x04(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x08, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, 0x10(%edi)
	movl	%ebx, 0x14(%edi)
	movl	%ecx, 0x18(%edi)
	movl	%edx, 0x1C(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	0x00(%edi), %esi
	movl	0x04(%edi), %ebx
	movl	0x08(%edi), %ecx
	movl	0x0C(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, 0x20(%edi)
	movl	%ebx, 0x24(%edi)
	movl	%ecx, 0x28(%edi)
	movl	%edx, 0x2C(%edi)
/* Key Set 5 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	0x10(%edi), %eax
	movl	0x14(%edi), %ebx
	movl	0x18(%edi), %ecx
	movl	0x1C(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x10, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, 0x30(%edi)
	movl	%ebx, 0x34(%edi)
	movl	%ecx, 0x38(%edi)
	movl	%edx, 0x3C(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	0x20(%edi), %esi
	movl	0x24(%edi), %ebx
	movl	0x28(%edi), %ecx
	movl	0x2C(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, 0x40(%edi)
	movl	%ebx, 0x44(%edi)
	movl	%ecx, 0x48(%edi)
	movl	%edx, 0x4C(%edi)
/* Key Set 6 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	0x30(%edi), %eax
	movl	0x34(%edi), %ebx
	movl	0x38(%edi), %ecx
	movl	0x3C(%edi), %edx
	/* Compute first 4 keys of current set */
	xorl	$0x20, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store first 4 keys of current set */
	movl	%eax, 0x50(%edi)
	movl	%ebx, 0x54(%edi)
	movl	%ecx, 0x58(%edi)
	movl	%edx, 0x5C(%edi)
	/* Substutite */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ebx
	shll	$16, %ecx
	shll	$24, %esi
	orl		%ebx, %eax
	orl		%ecx, %eax
	orl		%esi, %eax
	/* Load last 4 keys of previous set */
	movl	0x40(%edi), %esi
	movl	0x44(%edi), %ebx
	movl	0x48(%edi), %ecx
	movl	0x4C(%edi), %edx
	/* Compute last 4 keys of current set */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store last 4 keys of current set */
	movl	%eax, 0x60(%edi)
	movl	%ebx, 0x64(%edi)
	movl	%ecx, 0x68(%edi)
	movl	%edx, 0x6C(%edi)
/* Key Set 7 */
	/* Combined substutute and rotate */
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shrl	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %esi
	movzbl	__crypto_aes_sbox(%eax), %eax
	movzbl	__crypto_aes_sbox(%ebx), %ebx
	movzbl	__crypto_aes_sbox(%ecx), %ecx
	movzbl	__crypto_aes_sbox(%esi), %esi
	shll	$8, %ecx
	shll	$16, %esi
	shll	$24, %eax
	orl		%ebx, %esi
	orl		%ecx, %esi
	orl		%eax, %esi
	/* Load first 4 keys of previous set */
	movl	0x50(%edi), %eax
	movl	0x54(%edi), %ebx
	movl	0x58(%edi), %ecx
	movl	0x5C(%edi), %edx
	/* Compute 4 keys of current (partial) set */
	xorl	$0x20, %eax			/* Add round constant */
	xorl	%esi, %eax
	xorl	%eax, %ebx
	xorl	%ebx, %ecx
	xorl	%ecx, %edx
	/* Store 4 keys of current (partial) set */
	movl	%eax, 0x70(%edi)
	movl	%ebx, 0x74(%edi)
	movl	%ecx, 0x78(%edi)
	movl	%edx, 0x7C(%edi)
	/* Restore clobbered registers and return */
	popl	%edi
	popl	%esi
	popl	%ebx
	ret




	.globl	__crypto_aes128_invert_x86
	.def	__crypto_aes128_invert_x86;	.scl	2;	.type	32;	.endef
	/* void _aes128_invert_x86(uint32_t* dst [4(%esp)], */
	/*                         const uint32_t* src [8(%esp)]) */
__crypto_aes128_invert_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %edi	/* Load dst pointer into edi */
	movl	24(%esp), %esi	/* Load src pointer into esi */
	addl	$0x30, %edi			/* Offset dst pointer (to allow for 1-byte offsets) */
	addl	$0x30, %esi			/* Offset src pointer (to allow for 1-byte offsets) */
	cmpl	%edi, %esi
	jne		.Linvert128
	movl	-0x30(%esi), %eax
	movl	-0x2C(%esi), %ebx
	movl	-0x28(%esi), %ecx
	movl	-0x24(%esi), %edx
	movl	%eax, -0x30(%edi)
	movl	%ebx, -0x2C(%edi)
	movl	%ecx, -0x28(%edi)
	movl	%edx, -0x24(%edi)
	movl	0x70(%esi), %eax
	movl	0x74(%esi), %ebx
	movl	0x78(%esi), %ecx
	movl	0x7C(%esi), %edx
	movl	%eax, 0x70(%edi)
	movl	%ebx, 0x74(%edi)
	movl	%ecx, 0x78(%edi)
	movl	%edx, 0x7C(%edi)
	.align	16
.Linvert128:
	movl	-0x20(%esi), %eax
	movl	-0x1C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x20(%edi)
	movl	%edx, -0x1C(%edi)
	movl	-0x18(%esi), %eax
	movl	-0x14(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x18(%edi)
	movl	%edx, -0x14(%edi)

	movl	-0x10(%esi), %eax
	movl	-0x0C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x10(%edi)
	movl	%edx, -0x0C(%edi)
	movl	-0x08(%esi), %eax
	movl	-0x04(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x08(%edi)
	movl	%edx, -0x04(%edi)

	movl	0x00(%esi), %eax
	movl	0x04(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x00(%edi)
	movl	%edx, 0x04(%edi)
	movl	0x08(%esi), %eax
	movl	0x0C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x08(%edi)
	movl	%edx, 0x0C(%edi)

	movl	0x10(%esi), %eax
	movl	0x14(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x10(%edi)
	movl	%edx, 0x14(%edi)
	movl	0x18(%esi), %eax
	movl	0x1C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x18(%edi)
	movl	%edx, 0x1C(%edi)

	movl	0x20(%esi), %eax
	movl	0x24(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x20(%edi)
	movl	%edx, 0x24(%edi)
	movl	0x28(%esi), %eax
	movl	0x2C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x28(%edi)
	movl	%edx, 0x2C(%edi)

	movl	0x30(%esi), %eax
	movl	0x34(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x30(%edi)
	movl	%edx, 0x34(%edi)
	movl	0x38(%esi), %eax
	movl	0x3C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x38(%edi)
	movl	%edx, 0x3C(%edi)

	movl	0x40(%esi), %eax
	movl	0x44(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x40(%edi)
	movl	%edx, 0x44(%edi)
	movl	0x48(%esi), %eax
	movl	0x4C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x48(%edi)
	movl	%edx, 0x4C(%edi)

	movl	0x50(%esi), %eax
	movl	0x54(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x50(%edi)
	movl	%edx, 0x54(%edi)
	movl	0x58(%esi), %eax
	movl	0x5C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x58(%edi)
	movl	%edx, 0x5C(%edi)

	movl	0x60(%esi), %eax
	movl	0x64(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x60(%edi)
	movl	%edx, 0x64(%edi)
	movl	0x68(%esi), %eax
	movl	0x6C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, 0x68(%edi)
	movl	%edx, 0x6C(%edi)
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes192_invert_x86
	.def	__crypto_aes192_invert_x86;	.scl	2;	.type	32;	.endef
	/* void _aes192_invert_x86(uint32_t* dst [4(%esp)], */
	/*                         const uint32_t* src [8(%esp)]) */
__crypto_aes192_invert_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %edi	/* Load dst pointer into edi */
	movl	24(%esp), %esi	/* Load src pointer into esi */
	addl	$0x50, %edi			/* Offset dst pointer (to allow for 1-byte offsets) */
	addl	$0x50, %esi			/* Offset src pointer (to allow for 1-byte offsets) */
	cmpl	%edi, %esi
	jne		.Linvert192
	movl	-0x50(%esi), %eax
	movl	-0x4C(%esi), %ebx
	movl	-0x48(%esi), %ecx
	movl	-0x44(%esi), %edx
	movl	%eax, -0x50(%edi)
	movl	%ebx, -0x4C(%edi)
	movl	%ecx, -0x48(%edi)
	movl	%edx, -0x44(%edi)
	movl	0x70(%esi), %eax
	movl	0x74(%esi), %ebx
	movl	0x78(%esi), %ecx
	movl	0x7C(%esi), %edx
	movl	%eax, 0x70(%edi)
	movl	%ebx, 0x74(%edi)
	movl	%ecx, 0x78(%edi)
	movl	%edx, 0x7C(%edi)
.Linvert192:
	movl	-0x40(%esi), %eax
	movl	-0x3C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x40(%edi)
	movl	%edx, -0x3C(%edi)
	movl	-0x38(%esi), %eax
	movl	-0x34(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x38(%edi)
	movl	%edx, -0x34(%edi)

	movl	-0x30(%esi), %eax
	movl	-0x2C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x30(%edi)
	movl	%edx, -0x2C(%edi)
	movl	-0x28(%esi), %eax
	movl	-0x24(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x28(%edi)
	movl	%edx, -0x24(%edi)
	jmp		.Linvert128


	.globl	__crypto_aes256_invert_x86
	.def	__crypto_aes256_invert_x86;	.scl	2;	.type	32;	.endef
	/* void _aes256_invert_x86(uint32_t* dst [4(%esp)], */
	/*                         const uint32_t* src [8(%esp)]) */
__crypto_aes256_invert_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	movl	20(%esp), %edi	/* Load dst pointer into edi */
	movl	24(%esp), %esi	/* Load src pointer into esi */
	addl	$0x70, %edi			/* Offset dst pointer (to allow for 1-byte offsets) */
	addl	$0x70, %esi			/* Offset src pointer (to allow for 1-byte offsets) */
	cmpl	%edi, %esi
	jne		.Linvert256
	movl	-0x70(%esi), %eax
	movl	-0x6C(%esi), %ebx
	movl	-0x68(%esi), %ecx
	movl	-0x64(%esi), %edx
	movl	%eax, -0x70(%edi)
	movl	%ebx, -0x6C(%edi)
	movl	%ecx, -0x68(%edi)
	movl	%edx, -0x64(%edi)
	movl	0x70(%esi), %eax
	movl	0x74(%esi), %ebx
	movl	0x78(%esi), %ecx
	movl	0x7C(%esi), %edx
	movl	%eax, 0x70(%edi)
	movl	%ebx, 0x74(%edi)
	movl	%ecx, 0x78(%edi)
	movl	%edx, 0x7C(%edi)
.Linvert256:
	movl	-0x60(%esi), %eax
	movl	-0x5C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x60(%edi)
	movl	%edx, -0x5C(%edi)
	movl	-0x58(%esi), %eax
	movl	-0x54(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x58(%edi)
	movl	%edx, -0x54(%edi)

	movl	-0x50(%esi), %eax
	movl	-0x4C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x50(%edi)
	movl	%edx, -0x4C(%edi)
	movl	-0x48(%esi), %eax
	movl	-0x44(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x48(%edi)
	movl	%edx, -0x44(%edi)

	movl	-0x40(%esi), %eax
	movl	-0x3C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x40(%edi)
	movl	%edx, -0x3C(%edi)
	movl	-0x38(%esi), %eax
	movl	-0x34(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x38(%edi)
	movl	%edx, -0x34(%edi)

	movl	-0x30(%esi), %eax
	movl	-0x2C(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x30(%edi)
	movl	%edx, -0x2C(%edi)
	movl	-0x28(%esi), %eax
	movl	-0x24(%esi), %ebx
	AES_INVERT	%eax, %al, %ah, %ecx, %ebp
	AES_INVERT	%ebx, %bl, %bh, %edx, %ebp
	movl	%ecx, -0x28(%edi)
	movl	%edx, -0x24(%edi)
	jmp		.Linvert128




	.globl	__crypto_aes128_encrypt_x86
	.def	__crypto_aes128_encrypt_x86;	.scl	2;	.type	32;	.endef
	/* void _aes128_encrypt_x86(const uint32_t* key [4(%esp)], */
	/*                          uint8_t* dst [8(%esp)], */
	/*                          const uint8_t* src [12(%esp)], */
	/*                          size_t length [16(%esp)]) */
__crypto_aes128_encrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Lencrypt128_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	.align	16
.Lencrypt128_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	addl	$16, %esi			/* Increment the key pointer */
/* Encrypt Round 1 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 2 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 3 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 4 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 5 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 6 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 7 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 8 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 9 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 10, Final Round */
	AES_FINAL_ROUND	%al,%ebx,%ecx,%edx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%ecx,%edx,%eax, __crypto_aes_sbox, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%edx,%eax,%ebx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%eax,%ebx,%ecx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Apply s-box */
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Lencrypt128_loop
.Lencrypt128_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret



	.globl	__crypto_aes192_encrypt_x86
	.def	__crypto_aes192_encrypt_x86;	.scl	2;	.type	32;	.endef
	/* void _aes192_encrypt_x86(const uint32_t* key [4(%esp)], */
	/*                          uint8_t* dst [8(%esp)], */
	/*                          const uint8_t* src [12(%esp)], */
	/*                          size_t length [16(%esp)]) */
__crypto_aes192_encrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Lencrypt192_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	.align	16
.Lencrypt192_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	movl	__crypto_aes_dtable, %ebp	/* Load the mix-columns table */
	addl	$16, %esi			/* Increment the key pointer */
/* Encrypt Round 1 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 2 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 3 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 4 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 5 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 6 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 7 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 8 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 9 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 10 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 11 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 12, Final Round */
	AES_FINAL_ROUND	%al,%ebx,%ecx,%edx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%ecx,%edx,%eax, __crypto_aes_sbox, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%edx,%eax,%ebx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%eax,%ebx,%ecx, __crypto_aes_sbox, %edi, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Apply s-box */
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Lencrypt192_loop
.Lencrypt192_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes256_encrypt_x86
	.def	__crypto_aes256_encrypt_x86;	.scl	2;	.type	32;	.endef
	/* void _aes256_encrypt_x86(const uint32_t* key [4(%esp)], */
	/*                          uint8_t* dst [8(%esp)], */
	/*                          const uint8_t* src [12(%esp)], */
	/*                          size_t length [16(%esp)]) */
__crypto_aes256_encrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Lencrypt256_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	.align	16
.Lencrypt256_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	movl	__crypto_aes_dtable, %ebp	/* Load the mix-columns table */
	addl	$16, %esi			/* Increment the key pointer */
/* Encrypt Round 1 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 2 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 3 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 4 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 5 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 6 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 7 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 8 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 9 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 10 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 11 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 12 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 13 */
	AES_ROUND	__crypto_aes_dtable, %al,%bh,%ecx,%edx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_dtable, %bl,%ch,%edx,%eax, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_dtable, %cl,%dh,%eax,%ebx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_dtable, %dl,%ah,%ebx,%ecx, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	addl	$16, %esi		/* Increment the key pointer */
/* Encrypt Round 14, Final Round */
	AES_FINAL_ROUND	%al,%ebx,%ecx,%edx, __crypto_aes_sbox, %edi, %ebp	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%ecx,%edx,%eax, __crypto_aes_sbox, %edi, %ebp	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%edx,%eax,%ebx, __crypto_aes_sbox, %edi, %ebp	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%eax,%ebx,%ecx, __crypto_aes_sbox, %edi, %ebp	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_sbox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Lencrypt256_loop
.Lencrypt256_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret




	.globl	__crypto_aes128_decrypt_x86
	.def	__crypto_aes128_decrypt_x86;	.scl	2;	.type	32;	.endef
__crypto_aes128_decrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Ldecrypt128_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	addl	$160, 0x20(%esp)	/* Increment the key pointer to point at the last round key */
	.align	16
.Ldecrypt128_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	subl	$16, %esi			/* Decrement the key pointer */
/* Decrypt Round 1 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 2 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 3 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 4 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 5 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 6 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 7 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 8 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 9 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 10, Final Round */
	AES_FINAL_ROUND	%al,%edx,%ecx,%ebx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%eax,%edx,%ecx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%ebx,%eax,%edx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%ecx,%ebx,%ebx, __crypto_aes_ibox, %edx, %ebp	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Apply s-box */
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Ldecrypt128_loop
.Ldecrypt128_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes192_decrypt_x86
	.def	__crypto_aes192_decrypt_x86;	.scl	2;	.type	32;	.endef
__crypto_aes192_decrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Ldecrypt192_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	addl	$192, 0x20(%esp)	/* Increment the key pointer to point at the last round key */
	.align	16
.Ldecrypt192_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	subl	$16, %esi			/* Decrement the key pointer */
/* Decrypt Round 1 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 2 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 3 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 4 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 5 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 6 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 7 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 8 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 9 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 10 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 11 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 12, Final Round */
	AES_FINAL_ROUND	%al,%edx,%ecx,%ebx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%eax,%edx,%ecx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%ebx,%eax,%edx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%ecx,%ebx,%ebx, __crypto_aes_ibox, %edx, %ebp	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Apply s-box */
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Ldecrypt192_loop
.Ldecrypt192_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret


	.globl	__crypto_aes256_decrypt_x86
	.def	__crypto_aes256_decrypt_x86;	.scl	2;	.type	32;	.endef
__crypto_aes256_decrypt_x86:
	/* Clobber registers */
	pushl	%ebx
	pushl	%ebp
	pushl	%esi
	pushl	%edi
	subl	$12, %esp	/* Allocate 12 bytes of save area on stack */
	/* Stack usage:
	 * 0x2C |     length     |
	 * 0x28 |      src       |
	 * 0x24 |      dst       |
	 * 0x20 |      key       |
	 * 0x1C | return address |
	 * 0x18 |      ebx       |
	 * 0x14 |      ebp       |
	 * 0x10 |      esi       |
	 * 0x0C |      edi       |
	 * 0x08 |     tmp a      |
	 * 0x04 |     tmp b      |
	 * 0x00 |     tmp c      |
	 */
	movl	0x2C(%esp), %ebp	/* length -> ebp */
	cmpl	$16, %ebp
	jb		.Ldecrypt256_end	/* skip to return if there is less than 16 bytes to process */

	shrl	$4, 0x2C(%esp)		/* divide the length by 16 */
	addl	$224, 0x20(%esp)	/* Increment the key pointer to point at the last round key */
	.align	16
.Ldecrypt256_loop:
	movl	0x20(%esp), %esi	/* Copy the key pointer to esi */
	movl	0x28(%esp), %edi	/* Copy the source pointer to edi */
	AES_LOAD	%eax, %ebx, %ecx, %edx, %edi, %esi	/* Load the input and XOR with first key */
	addl	$16, 0x28(%esp)		/* Increment the source pointer by 16 */
	subl	$16, %esi			/* Decrement the key pointer */
/* Decrypt Round 1 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 2 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 3 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 4 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 5 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 6 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 7 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 8 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 9 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 10 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 11 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 12 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 13 */
	AES_ROUND	__crypto_aes_itable, %al,%dh,%ecx,%ebx, %edi, %esi	/* Compute tmp a */
	movl	%edi, 0x08(%esp)	/* Save tmp a on the stack */
	AES_ROUND	__crypto_aes_itable, %bl,%ah,%edx,%ecx, %edi, %esi	/* Compute tmp b */
	movl	%edi, 0x04(%esp)	/* Save tmp b on the stack */
	AES_ROUND	__crypto_aes_itable, %cl,%bh,%eax,%edx, %edi, %esi	/* Compute tmp c */
	movl	%edi, 0x00(%esp)	/* Save tmp c on the stack */
	AES_ROUND	__crypto_aes_itable, %dl,%ch,%ebx,%eax, %edx, %esi	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Add the current round key */
	xorl	0x0(%esi), %eax
	xorl	0x4(%esi), %ebx
	xorl	0x8(%esi), %ecx
	xorl	0xC(%esi), %edx
	subl	$16, %esi		/* Decrement the key pointer */
/* Decrypt Round 14, Final Round */
	AES_FINAL_ROUND	%al,%edx,%ecx,%ebx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp a */
	movl	%edi, 0x8(%esp)	/* Save tmp a on the stack */
	AES_FINAL_ROUND	%bl,%eax,%edx,%ecx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp b */
	movl	%edi, 0x4(%esp)	/* Save tmp b on the stack */
	AES_FINAL_ROUND	%cl,%ebx,%eax,%edx, __crypto_aes_ibox, %edi, %ebp	/* Compute tmp c */
	movl	%edi, 0x0(%esp)	/* Save tmp c on the stack */
	AES_FINAL_ROUND	%dl,%ecx,%ebx,%ebx, __crypto_aes_ibox, %edx, %ebp	/* Compute tmp d */
	/* Restore tmp a, tmp b, and tmp c */
	movl	0x8(%esp), %eax
	movl	0x4(%esp), %ebx
	movl	0x0(%esp), %ecx
	/* Apply s-box */
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	AES_SUB_BYTE	%eax,%ebx,%ecx,%edx, %al,%bl,%cl,%dl, __crypto_aes_ibox, %ebp
	/* Load dst pointer */
	movl	0x24(%esp), %edi
	/* Add last round key and store output */
	AES_STORE	%eax,%ebx,%ecx,%edx, %esi, %edi
	/* Increment dst pointer */
	addl	$16, 0x24(%esp)
	/* Decrement loop counter and check loop condition */
	decl	0x2C(%esp)
	jnz		.Ldecrypt256_loop
.Ldecrypt256_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addl	$12, %esp
	popl	%edi
	popl	%esi
	popl	%ebp
	popl	%ebx
	ret

	.ident	"GCC: (GNU) 6.4.0"
