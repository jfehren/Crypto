	.file	"aes-x86_64.asm"
	.text

	/* Takes the key in eax, applies the sbox to it, and rotates */
	/* the result right by 8 bits, storing the result in edx. */
	/* Uses registers rax, rbx, rcx, rdx, rbp, rsi, rdi, and r13. */
	/* Also reads from r12, but does not write to it. */
	.macro AES128_SET_KEY_SUBROTR8
	movzbl	%al, %ebp
	movzbl	%ah, %esi
	shrl	$16, %eax
	movzbl	%al, %r13d
	movzbl	%ah, %edi
	movzbl	(%r12,%rbp), %eax
	movzbl	(%r12,%rsi), %ebx
	movzbl	(%r12,%r13), %ecx
	movzbl	(%r12,%rdi), %edx
	movb	%cl, %bh
	movb	%al, %dh
	shll	$16, %ebx
	orl		%ebx, %edx
	.endm

	.align	16
	.globl	_crypto_aes128_set_key_x86_64
	.def	_crypto_aes128_set_key_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes128_set_key_x86_64(uint32_t* subkeys [%rdi], */
	/*                             const uint8_t* key [%rsi]) */
_crypto_aes128_set_key_x86_64:
	/* Clobber Registers */
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	/* Offset and store output pointer */
	leaq	0x30(%rdi), %r14
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %r12
	/* Copy first 4 keys */
	movl	0x0(%rsi), %r8d
	movl	0x4(%rsi), %r9d
	movl	0x8(%rsi), %r10d
	movl	0xC(%rsi), %r11d
	movl	%r8d,  -0x30(%r14)
	movl	%r9d,  -0x2C(%r14)
	movl	%r10d, -0x28(%r14)
	movl	%r11d, -0x24(%r14)
/* Key Set 1 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x01, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  -0x20(%r14)
	movl	%r9d,  -0x1C(%r14)
	movl	%r10d, -0x18(%r14)
	movl	%r11d, -0x14(%r14)
/* Key Set 2 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x02, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  -0x10(%r14)
	movl	%r9d,  -0x0C(%r14)
	movl	%r10d, -0x08(%r14)
	movl	%r11d, -0x04(%r14)
/* Key Set 3 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x04, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x00(%r14)
	movl	%r9d,  0x04(%r14)
	movl	%r10d, 0x08(%r14)
	movl	%r11d, 0x0C(%r14)
/* Key Set 4 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x08, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x10(%r14)
	movl	%r9d,  0x14(%r14)
	movl	%r10d, 0x18(%r14)
	movl	%r11d, 0x1C(%r14)
/* Key Set 5 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x10, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x20(%r14)
	movl	%r9d,  0x24(%r14)
	movl	%r10d, 0x28(%r14)
	movl	%r11d, 0x2C(%r14)
/* Key Set 6 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x20, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x30(%r14)
	movl	%r9d,  0x34(%r14)
	movl	%r10d, 0x38(%r14)
	movl	%r11d, 0x3C(%r14)
/* Key Set 7 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x40, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x40(%r14)
	movl	%r9d,  0x44(%r14)
	movl	%r10d, 0x48(%r14)
	movl	%r11d, 0x4C(%r14)
/* Key Set 8 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x80, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x50(%r14)
	movl	%r9d,  0x54(%r14)
	movl	%r10d, 0x58(%r14)
	movl	%r11d, 0x5C(%r14)
/* Key Set 9 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x1B, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x60(%r14)
	movl	%r9d,  0x64(%r14)
	movl	%r10d, 0x68(%r14)
	movl	%r11d, 0x6C(%r14)
/* Key Set 10 */
	/* Combined Substitute and Rotate */
	movl	%r11d, %eax
	AES128_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x36, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x70(%r14)
	movl	%r9d,  0x74(%r14)
	movl	%r10d, 0x78(%r14)
	movl	%r11d, 0x7C(%r14)
	/* Restore clobbered registers and return */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret


	/* Takes the key in eax, applies the sbox to it, and rotates */
	/* the result right by 8 bits, storing the result in ebx. */
	/* Uses registers rax, rbx, rcx, rdx, rbp, rsi, and rdi. */
	/* Also reads from r14, but does not write to it. */
	.macro AES192_SET_KEY_SUBROTR8
	movzbl	%al, %ebp
	movzbl	%ah, %esi
	shrl	$16, %eax
	movzbl	%ah, %edi
	movzbl	%al, %eax
	movzbl	(%r14,%rbp), %edx
	movzbl	(%r14,%rsi), %ecx
	movzbl	(%r14,%rdi), %ebx
	movzbl	(%r14,%rax), %eax
	movb	%al, %ch
	movb	%dl, %bh
	shll	$16, %ecx
	orl		%ecx, %ebx
	.endm

	.align	16
	.globl	_crypto_aes192_set_key_x86_64
	.def	_crypto_aes192_set_key_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes192_set_key_x86_64(uint32_t* subkeys [%rdi], */
	/*                             const uint8_t* key [%rsi]) */
_crypto_aes192_set_key_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	/* Offset and store output pointer */
	leaq	0x50(%rdi), %r15
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %r14
	/* Copy first 6 keys */
	movl	0x00(%rsi), %r8d
	movl	0x04(%rsi), %r9d
	movl	0x08(%rsi), %r10d
	movl	0x0C(%rsi), %r11d
	movl	0x10(%rsi), %r12d
	movl	0x14(%rsi), %r13d
	movl	%r8d,  -0x50(%r15)
	movl	%r9d,  -0x4C(%r15)
	movl	%r10d, -0x48(%r15)
	movl	%r11d, -0x44(%r15)
	movl	%r12d, -0x40(%r15)
	movl	%r13d, -0x3C(%r15)
/* Key Set 1 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x01, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  -0x38(%r15)
	movl	%r9d,  -0x34(%r15)
	movl	%r10d, -0x30(%r15)
	movl	%r11d, -0x2C(%r15)
	movl	%r12d, -0x28(%r15)
	movl	%r13d, -0x24(%r15)
/* Key Set 2 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x02, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  -0x20(%r15)
	movl	%r9d,  -0x1C(%r15)
	movl	%r10d, -0x18(%r15)
	movl	%r11d, -0x14(%r15)
	movl	%r12d, -0x10(%r15)
	movl	%r13d, -0x0C(%r15)
/* Key Set 3 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x04, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  -0x08(%r15)
	movl	%r9d,  -0x04(%r15)
	movl	%r10d, -0x00(%r15)
	movl	%r11d,  0x04(%r15)
	movl	%r12d,  0x08(%r15)
	movl	%r13d,  0x0C(%r15)
/* Key Set 4 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x08, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  0x10(%r15)
	movl	%r9d,  0x14(%r15)
	movl	%r10d, 0x18(%r15)
	movl	%r11d, 0x1C(%r15)
	movl	%r12d, 0x20(%r15)
	movl	%r13d, 0x24(%r15)
/* Key Set 5 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x10, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  0x28(%r15)
	movl	%r9d,  0x2C(%r15)
	movl	%r10d, 0x30(%r15)
	movl	%r11d, 0x34(%r15)
	movl	%r12d, 0x38(%r15)
	movl	%r13d, 0x3C(%r15)
/* Key Set 6 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x20, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  0x40(%r15)
	movl	%r9d,  0x44(%r15)
	movl	%r10d, 0x48(%r15)
	movl	%r11d, 0x4C(%r15)
	movl	%r12d, 0x50(%r15)
	movl	%r13d, 0x54(%r15)
/* Key Set 7 */
	/* Combined Substitute and Rotate */
	movl	%r13d, %eax
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x40, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	xorl	%r11d, %r12d
	xorl	%r12d, %r13d
	/* Store keys */
	movl	%r8d,  0x58(%r15)
	movl	%r9d,  0x5C(%r15)
	movl	%r10d, 0x60(%r15)
	movl	%r11d, 0x64(%r15)
	movl	%r12d, 0x68(%r15)
	movl	%r13d, 0x6C(%r15)
/* Key Set 8 */
	/* Combined Substitute and Rota */
	AES192_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x40, %r8d
	xorl	%ebx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x70(%r15)
	movl	%r9d,  0x74(%r15)
	movl	%r10d, 0x78(%r15)
	movl	%r11d, 0x7C(%r15)
	/* Restore clobbered registers and return */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	/* Takes the key in edx, applies the sbox to it, and rotates */
	/* the result right by 8 bits, storing the result in edx. */
	/* Uses registers rax, rbx, rcx, and rdx. */
	/* Also reads from rsi, but does not write to it. */
	.macro AES256_SET_KEY_SUBROTR8
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shll	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %edx
	movzbl	(%rsi,%rax), %eax
	movzbl	(%rsi,%rbx), %ebx
	movzbl	(%rsi,%rcx), %ecx
	movzbl	(%rsi,%rdx), %edx
	movb	%cl, %bh
	movb	%al, %dh
	shll	$16, %ebx
	orl		%ebx, %edx
	.endm

	/* Takes the key in edx and applies the sbox to it, */
	/* storing the result in eax. */
	/* Uses registers rax, rbx, rcx, and rdx. */
	/* Also reads from rsi, but does not write to it. */
	.macro AES256_SET_KEY_SUB
	movzbl	%dl, %eax
	movzbl	%dh, %ebx
	shll	$16, %edx
	movzbl	%dl, %ecx
	movzbl	%dh, %edx
	movzbl	(%rsi,%rax), %eax
	movzbl	(%rsi,%rbx), %ebx
	movzbl	(%rsi,%rcx), %ecx
	movzbl	(%rsi,%rdx), %edx
	movb	%bl, %ah
	movb	%dl, %ch
	shll	$16, %ecx
	orl		%ecx, %eax
	.endm

	.align	16
	.globl	_crypto_aes256_set_key_x86_64
	.def	_crypto_aes256_set_key_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes256_set_key_x86_64(uint32_t* subkeys [%rdi], */
	/*                             const uint8_t* key [%rsi]) */
_crypto_aes256_set_key_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	/* Offset output pointer */
	addq	$0x70, %rdi
	/* Copy first 8 keys */
	movl	0x00(%rsi), %r8d
	movl	0x04(%rsi), %r9d
	movl	0x08(%rsi), %r10d
	movl	0x0C(%rsi), %r11d
	movl	0x10(%rsi), %r12d
	movl	0x14(%rsi), %r13d
	movl	0x18(%rsi), %r14d
	movl	0x1C(%rsi), %r15d
	movl	%r8d,  -0x70(%rdi)
	movl	%r9d,  -0x6C(%rdi)
	movl	%r10d, -0x68(%rdi)
	movl	%r11d, -0x64(%rdi)
	movl	%r12d, -0x60(%rdi)
	movl	%r13d, -0x5C(%rdi)
	movl	%r14d, -0x58(%rdi)
	movl	%r15d, -0x54(%rdi)
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %rsi
/* Key Set 1 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x01, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  -0x50(%rdi)
	movl	%r9d,  -0x4C(%rdi)
	movl	%r10d, -0x48(%rdi)
	movl	%r11d, -0x44(%rdi)
	movl	%r12d, -0x40(%rdi)
	movl	%r13d, -0x3C(%rdi)
	movl	%r14d, -0x38(%rdi)
	movl	%r15d, -0x34(%rdi)
/* Key Set 2 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x02, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  -0x30(%rdi)
	movl	%r9d,  -0x2C(%rdi)
	movl	%r10d, -0x28(%rdi)
	movl	%r11d, -0x24(%rdi)
	movl	%r12d, -0x20(%rdi)
	movl	%r13d, -0x1C(%rdi)
	movl	%r14d, -0x18(%rdi)
	movl	%r15d, -0x14(%rdi)
/* Key Set 3 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x04, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  -0x10(%rdi)
	movl	%r9d,  -0x0C(%rdi)
	movl	%r10d, -0x08(%rdi)
	movl	%r11d, -0x04(%rdi)
	movl	%r12d,  0x00(%rdi)
	movl	%r13d,  0x04(%rdi)
	movl	%r14d,  0x08(%rdi)
	movl	%r15d,  0x0C(%rdi)
/* Key Set 4 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x08, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  0x10(%rdi)
	movl	%r9d,  0x14(%rdi)
	movl	%r10d, 0x18(%rdi)
	movl	%r11d, 0x1C(%rdi)
	movl	%r12d, 0x20(%rdi)
	movl	%r13d, 0x24(%rdi)
	movl	%r14d, 0x28(%rdi)
	movl	%r15d, 0x2C(%rdi)
/* Key Set 5 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x10, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  0x30(%rdi)
	movl	%r9d,  0x34(%rdi)
	movl	%r10d, 0x38(%rdi)
	movl	%r11d, 0x3C(%rdi)
	movl	%r12d, 0x40(%rdi)
	movl	%r13d, 0x44(%rdi)
	movl	%r14d, 0x48(%rdi)
	movl	%r15d, 0x4C(%rdi)
/* Key Set 6 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute first 4 keys of set */
	xorl	$0x20, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Substitute */
	movl	%r11d, %edx
	AES256_SET_KEY_SUB
	/* Compute last 4 keys of set */
	xorl	%eax, %r12d
	xorl	%r12d, %r13d
	xorl	%r13d, %r14d
	xorl	%r14d, %r15d
	/* Store keys */
	movl	%r8d,  0x50(%rdi)
	movl	%r9d,  0x54(%rdi)
	movl	%r10d, 0x58(%rdi)
	movl	%r11d, 0x5C(%rdi)
	movl	%r12d, 0x60(%rdi)
	movl	%r13d, 0x64(%rdi)
	movl	%r14d, 0x68(%rdi)
	movl	%r15d, 0x6C(%rdi)
/* Key Set 7 */
	/* Combined Substitute and Rotate */
	movl	%r15d, %edx
	AES256_SET_KEY_SUBROTR8
	/* Compute keys */
	xorl	$0x40, %r8d
	xorl	%edx, %r8d
	xorl	%r8d, %r9d
	xorl	%r9d, %r10d
	xorl	%r10d, %r11d
	/* Store keys */
	movl	%r8d,  0x70(%rdi)
	movl	%r9d,  0x74(%rdi)
	movl	%r10d, 0x78(%rdi)
	movl	%r11d, 0x7C(%rdi)
	/* Restore clobbered registers and return */
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret




	/* Invert the 4 keys in eax, ebx, ecx, and edx, storing */
	/* the results in r8d, r9d, r10d, and r11d, respectively. */
	/* Uses registers rax, rbx, rcx, rdx, r8, r9, r10, r11, r12, r13, r14, and r15. */
	/* Also reads from rbp, but does not write to it. */
	.macro AES_INVERT4
	movzbl	%al, %r12d
	movzbl	%bl, %r13d
	movzbl	%cl, %r14d
	movzbl	%dl, %r15d
	movl	0x000(%rbp,%r12,4), %r8d
	movl	0x000(%rbp,%r13,4), %r9d
	movl	0x000(%rbp,%r14,4), %r10d
	movl	0x000(%rbp,%r15,4), %r11d
	movzbl	%ah, %r12d
	movzbl	%bh, %r13d
	movzbl	%ch, %r14d
	movzbl	%dh, %r15d
	xorl	0x400(%rbp,%r12,4), %r8d
	xorl	0x400(%rbp,%r13,4), %r9d
	xorl	0x400(%rbp,%r14,4), %r10d
	xorl	0x400(%rbp,%r15,4), %r11d
	shrl	$16, %eax
	shrl	$16, %ebx
	shrl	$16, %ecx
	shrl	$16, %edx
	movzbl	%al, %r12d
	movzbl	%bl, %r13d
	movzbl	%cl, %r14d
	movzbl	%dl, %r15d
	movl	0x800(%rbp,%r12,4), %r8d
	movl	0x800(%rbp,%r13,4), %r9d
	movl	0x800(%rbp,%r14,4), %r10d
	movl	0x800(%rbp,%r15,4), %r11d
	movzbl	%ah, %r12d
	movzbl	%bh, %r13d
	movzbl	%ch, %r14d
	movzbl	%dh, %r15d
	xorl	0xC00(%rbp,%r12,4), %r8d
	xorl	0xC00(%rbp,%r13,4), %r9d
	xorl	0xC00(%rbp,%r14,4), %r10d
	xorl	0xC00(%rbp,%r15,4), %r11d
	.endm

	.align	16
	.globl	_crypto_aes128_invert_x86_64
	.def	_crypto_aes128_invert_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes128_invert_x86_64(uint32_t* dst [%rdi], */
	/*                            const uint32_t* src [%rsi]) */
_crypto_aes128_invert_x86_64:
	/* Clobber registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	addq	$0x30, %rdi		/* Offset dst pointer (to allow for 1-byte offsets) */
	addq	$0x30, %rsi		/* Offset src pointer (to allow for 1-byte offsets) */
	movq	.refptr._crypto_aes_mtable(%rip), %rbp	/* Load mtable pointer into rbp */
	cmpq	%rdi, %rsi
	jne		.Linvert128
	/* Copy first 4 and last 4 keys without inverting */
	/* Round Key 0 */
	movl	-0x30(%rsi), %eax
	movl	-0x2C(%rsi), %ebx
	movl	-0x28(%rsi), %ecx
	movl	-0x24(%rsi), %edx
	/* Round Key 10 */
	movl	0x70(%rsi), %r12d
	movl	0x74(%rsi), %r13d
	movl	0x78(%rsi), %r14d
	movl	0x7C(%rsi), %r15d
	/* Round Key 0 */
	movl	%eax, -0x30(%rdi)
	movl	%ebx, -0x2C(%rdi)
	movl	%ecx, -0x28(%rdi)
	movl	%edx, -0x24(%rdi)
	/* Round Key 10 */
	movl	%r12d, 0x70(%rdi)
	movl	%r13d, 0x74(%rdi)
	movl	%r14d, 0x78(%rdi)
	movl	%r15d, 0x7C(%rdi)
	.align	16
.Linvert128:
	/* Round Key 1 */
	movl	-0x20(%rsi), %eax
	movl	-0x1C(%rsi), %ebx
	movl	-0x18(%rsi), %ecx
	movl	-0x14(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x20(%rdi)
	movl	%r9d,  -0x1C(%rdi)
	movl	%r10d, -0x18(%rdi)
	movl	%r11d, -0x14(%rdi)
	/* Round Key 2 */
	movl	-0x10(%rsi), %eax
	movl	-0x0C(%rsi), %ebx
	movl	-0x08(%rsi), %ecx
	movl	-0x04(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x10(%rdi)
	movl	%r9d,  -0x0C(%rdi)
	movl	%r10d, -0x08(%rdi)
	movl	%r11d, -0x04(%rdi)
	/* Round Key 3 */
	movl	0x00(%rsi), %eax
	movl	0x04(%rsi), %ebx
	movl	0x08(%rsi), %ecx
	movl	0x0C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x00(%rdi)
	movl	%r9d,  0x04(%rdi)
	movl	%r10d, 0x08(%rdi)
	movl	%r11d, 0x0C(%rdi)
	/* Round Key 4 */
	movl	0x10(%rsi), %eax
	movl	0x14(%rsi), %ebx
	movl	0x18(%rsi), %ecx
	movl	0x1C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x10(%rdi)
	movl	%r9d,  0x14(%rdi)
	movl	%r10d, 0x18(%rdi)
	movl	%r11d, 0x1C(%rdi)
	/* Round Key 5 */
	movl	0x20(%rsi), %eax
	movl	0x24(%rsi), %ebx
	movl	0x28(%rsi), %ecx
	movl	0x2C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x20(%rdi)
	movl	%r9d,  0x24(%rdi)
	movl	%r10d, 0x28(%rdi)
	movl	%r11d, 0x2C(%rdi)
	/* Round Key 6 */
	movl	0x30(%rsi), %eax
	movl	0x34(%rsi), %ebx
	movl	0x38(%rsi), %ecx
	movl	0x3C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x30(%rdi)
	movl	%r9d,  0x34(%rdi)
	movl	%r10d, 0x38(%rdi)
	movl	%r11d, 0x3C(%rdi)
	/* Round Key 7 */
	movl	0x40(%rsi), %eax
	movl	0x44(%rsi), %ebx
	movl	0x48(%rsi), %ecx
	movl	0x4C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x40(%rdi)
	movl	%r9d,  0x44(%rdi)
	movl	%r10d, 0x48(%rdi)
	movl	%r11d, 0x4C(%rdi)
	/* Round Key 8 */
	movl	0x50(%rsi), %eax
	movl	0x54(%rsi), %ebx
	movl	0x58(%rsi), %ecx
	movl	0x5C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x50(%rdi)
	movl	%r9d,  0x54(%rdi)
	movl	%r10d, 0x58(%rdi)
	movl	%r11d, 0x5C(%rdi)
	/* Round Key 9 */
	movl	0x60(%rsi), %eax
	movl	0x64(%rsi), %ebx
	movl	0x68(%rsi), %ecx
	movl	0x6C(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  0x60(%rdi)
	movl	%r9d,  0x64(%rdi)
	movl	%r10d, 0x68(%rdi)
	movl	%r11d, 0x6C(%rdi)
	/* Restore clobbered registers and return */
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	.align	16
	.globl	_crypto_aes192_invert_x86_64
	.def	_crypto_aes192_invert_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes192_invert_x86_64(uint32_t* dst [%rdi], */
	/*                            const uint32_t* src [%rsi]) */
_crypto_aes192_invert_x86_64:
	/* Clobber registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	addq	$0x50, %rdi		/* Offset dst pointer (to allow for 1-byte offsets) */
	addq	$0x50, %rsi		/* Offset src pointer (to allow for 1-byte offsets) */
	movq	.refptr._crypto_aes_mtable(%rip), %rbp	/* Load mtable pointer into rbp */
	cmpq	%rdi, %rsi
	jne		.Linvert192
	/* Copy first 4 and last 4 keys without inverting */
	/* Round Key 0 */
	movl	-0x50(%rsi), %eax
	movl	-0x4C(%rsi), %ebx
	movl	-0x48(%rsi), %ecx
	movl	-0x44(%rsi), %edx
	/* Round Key 12 */
	movl	0x70(%rsi), %r12d
	movl	0x74(%rsi), %r13d
	movl	0x78(%rsi), %r14d
	movl	0x7C(%rsi), %r15d
	/* Round Key 0 */
	movl	%eax, -0x50(%rdi)
	movl	%ebx, -0x4C(%rdi)
	movl	%ecx, -0x48(%rdi)
	movl	%edx, -0x44(%rdi)
	/* Round Key 12 */
	movl	%r12d, 0x70(%rdi)
	movl	%r13d, 0x74(%rdi)
	movl	%r14d, 0x78(%rdi)
	movl	%r15d, 0x7C(%rdi)
.Linvert192:
	/* Round Key 1 */
	movl	-0x40(%rsi), %eax
	movl	-0x3C(%rsi), %ebx
	movl	-0x38(%rsi), %ecx
	movl	-0x34(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x40(%rdi)
	movl	%r9d,  -0x3C(%rdi)
	movl	%r10d, -0x38(%rdi)
	movl	%r11d, -0x34(%rdi)
	/* Round Key 2 */
	movl	-0x30(%rsi), %eax
	movl	-0x2C(%rsi), %ebx
	movl	-0x28(%rsi), %ecx
	movl	-0x24(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x30(%rdi)
	movl	%r9d,  -0x2C(%rdi)
	movl	%r10d, -0x28(%rdi)
	movl	%r11d, -0x24(%rdi)
	/* Recycle 128-bit invert code for remaining 9 round keys */
	jmp		.Linvert128


	.globl	_crypto_aes256_invert_x86_64
	.def	_crypto_aes256_invert_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes256_invert_x86_64(uint32_t* dst [%rdi], */
	/*                            const uint32_t* src [%rsi]) */
_crypto_aes256_invert_x86_64:
	/* Clobber registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	addq	$0x70, %rdi		/* Offset dst pointer (to allow for 1-byte offsets) */
	addq	$0x70, %rsi		/* Offset src pointer (to allow for 1-byte offsets) */
	movq	.refptr._crypto_aes_mtable(%rip), %rbp	/* Load mtable pointer into rbp */
	cmpq	%rdi, %rsi
	jne		.Linvert256
	/* Copy first 4 and last 4 keys without inverting */
	/* Round Key 0 */
	movl	-0x70(%rsi), %eax
	movl	-0x6C(%rsi), %ebx
	movl	-0x68(%rsi), %ecx
	movl	-0x64(%rsi), %edx
	/* Round Key 14 */
	movl	0x70(%rsi), %r12d
	movl	0x74(%rsi), %r13d
	movl	0x78(%rsi), %r14d
	movl	0x7C(%rsi), %r15d
	/* Round Key 0 */
	movl	%eax, -0x70(%rdi)
	movl	%ebx, -0x6C(%rdi)
	movl	%ecx, -0x68(%rdi)
	movl	%edx, -0x64(%rdi)
	/* Round Key 14 */
	movl	%r12d, 0x70(%rdi)
	movl	%r13d, 0x74(%rdi)
	movl	%r14d, 0x78(%rdi)
	movl	%r15d, 0x7C(%rdi)
.Linvert256:
	/* Round Key 1 */
	movl	-0x60(%rsi), %eax
	movl	-0x5C(%rsi), %ebx
	movl	-0x58(%rsi), %ecx
	movl	-0x54(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x60(%rdi)
	movl	%r9d,  -0x5C(%rdi)
	movl	%r10d, -0x58(%rdi)
	movl	%r11d, -0x54(%rdi)
	/* Round Key 2 */
	movl	-0x50(%rsi), %eax
	movl	-0x4C(%rsi), %ebx
	movl	-0x48(%rsi), %ecx
	movl	-0x44(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x50(%rdi)
	movl	%r9d,  -0x4C(%rdi)
	movl	%r10d, -0x48(%rdi)
	movl	%r11d, -0x44(%rdi)
	/* Round Key 3 */
	movl	-0x40(%rsi), %eax
	movl	-0x3C(%rsi), %ebx
	movl	-0x38(%rsi), %ecx
	movl	-0x34(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x40(%rdi)
	movl	%r9d,  -0x3C(%rdi)
	movl	%r10d, -0x38(%rdi)
	movl	%r11d, -0x34(%rdi)
	/* Round Key 4 */
	movl	-0x30(%rsi), %eax
	movl	-0x2C(%rsi), %ebx
	movl	-0x28(%rsi), %ecx
	movl	-0x24(%rsi), %edx
	AES_INVERT4
	movl	%r8d,  -0x30(%rdi)
	movl	%r9d,  -0x2C(%rdi)
	movl	%r10d, -0x28(%rdi)
	movl	%r11d, -0x24(%rdi)
	/* Recycle 128-bit invert code for remaining 9 round keys */
	jmp		.Linvert128




	.macro	AES_ENCRYPT_ROUND
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movl	0x000(%r13,%rbp,4), %r8d
	movl	0x000(%r13,%rsi,4), %r9d
	movl	0x000(%r13,%rdi,4), %r10d
	movl	0x000(%r13,%r12,4), %r11d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	xorl	0x400(%r13,%rbp,4), %r11d
	xorl	0x400(%r13,%rsi,4), %r8d
	xorl	0x400(%r13,%rdi,4), %r9d
	xorl	0x400(%r13,%r12,4), %r10d
	shrl	$16, %eax
	shrl	$16, %ebx
	shrl	$16, %ecx
	shrl	$16, %edx
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	xorl	0x800(%r13,%rbp,4), %r10d
	xorl	0x800(%r13,%rsi,4), %r11d
	xorl	0x800(%r13,%rdi,4), %r8d
	xorl	0x800(%r13,%r12,4), %r9d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	xorl	0xC00(%r13,%rbp,4), %r9d
	xorl	0xC00(%r13,%rsi,4), %r10d
	xorl	0xC00(%r13,%rdi,4), %r11d
	xorl	0xC00(%r13,%r12,4), %r8d
	.endm

	.macro AES_ENCRYPT_ROUND_FINAL
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movzbl	(%r13,%rbp), %r8d
	movzbl	(%r13,%rsi), %r9d
	movzbl	(%r13,%rdi), %r10d
	movzbl	(%r13,%r12), %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	rorl	$8, %r10d
	rorl	$8, %r11d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	movb	(%r13,%rbp), %r11b
	movb	(%r13,%rsi), %r8b
	movb	(%r13,%rdi), %r9b
	movb	(%r13,%r12), %r10b
	rorl	$8, %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	rorl	$8, %r10d
	shrl	$16, %eax
	shrl	$16, %ebx
	shrl	$16, %ecx
	shrl	$16, %edx
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movb	(%r13,%rbp), %r10b
	movb	(%r13,%rsi), %r11b
	movb	(%r13,%rdi), %r8b
	movb	(%r13,%r12), %r9b
	rorl	$8, %r10d
	rorl	$8, %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	movb	(%r13,%rbp), %r9b
	movb	(%r13,%rsi), %r10b
	movb	(%r13,%rdi), %r11b
	movb	(%r13,%r12), %r8b
	rorl	$8, %r9d
	rorl	$8, %r10d
	rorl	$8, %r11d
	rorl	$8, %r8d
	.endm

	.align	16
	.globl	_crypto_aes128_encrypt_x86_64
	.def	_crypto_aes128_encrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes128_encrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes128_encrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Lencrypt128_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x30(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Lencrypt128_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store dtable pointer */
	movq	.refptr._crypto_aes_dtable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add first round key */
	xorl	-0x30(%r14), %eax
	xorl	-0x2C(%r14), %ebx
	xorl	-0x28(%r14), %ecx
	xorl	-0x24(%r14), %edx
	/* Round 1 */
	AES_ENCRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_ENCRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_ENCRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_ENCRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_ENCRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_ENCRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_ENCRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_ENCRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 9 */
	AES_ENCRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 10, Final Round */
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %r13
	AES_ENCRYPT_ROUND_FINAL
	xorl	0x70(%r14), %r8d
	xorl	0x74(%r14), %r9d
	xorl	0x78(%r14), %r10d
	xorl	0x7C(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Lencrypt128_loop
	.align	16
.Lencrypt128_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	.align	16
	.globl	_crypto_aes192_encrypt_x86_64
	.def	_crypto_aes192_encrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes192_encrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes192_encrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Lencrypt192_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x50(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Lencrypt192_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store dtable pointer */
	movq	.refptr._crypto_aes_dtable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add first round key */
	xorl	-0x50(%r14), %eax
	xorl	-0x4C(%r14), %ebx
	xorl	-0x48(%r14), %ecx
	xorl	-0x44(%r14), %edx
	/* Round 1 */
	AES_ENCRYPT_ROUND
	movl	-0x40(%r14), %eax
	movl	-0x3C(%r14), %ebx
	movl	-0x38(%r14), %ecx
	movl	-0x34(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_ENCRYPT_ROUND
	movl	-0x30(%r14), %eax
	movl	-0x2C(%r14), %ebx
	movl	-0x28(%r14), %ecx
	movl	-0x24(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_ENCRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_ENCRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_ENCRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_ENCRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_ENCRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_ENCRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 9 */
	AES_ENCRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 10 */
	AES_ENCRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 11 */
	AES_ENCRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 12, Final Round */
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %r13
	AES_ENCRYPT_ROUND_FINAL
	xorl	0x70(%r14), %r8d
	xorl	0x74(%r14), %r9d
	xorl	0x78(%r14), %r10d
	xorl	0x7C(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Lencrypt192_loop
	.align	16
.Lencrypt192_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	.align	16
	.globl	_crypto_aes256_encrypt_x86_64
	.def	_crypto_aes256_encrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes256_encrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes256_encrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Lencrypt256_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x70(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Lencrypt256_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store dtable pointer */
	movq	.refptr._crypto_aes_dtable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add first round key */
	xorl	-0x70(%r14), %eax
	xorl	-0x6C(%r14), %ebx
	xorl	-0x68(%r14), %ecx
	xorl	-0x64(%r14), %edx
	/* Round 1 */
	AES_ENCRYPT_ROUND
	movl	-0x60(%r14), %eax
	movl	-0x5C(%r14), %ebx
	movl	-0x58(%r14), %ecx
	movl	-0x54(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_ENCRYPT_ROUND
	movl	-0x50(%r14), %eax
	movl	-0x4C(%r14), %ebx
	movl	-0x48(%r14), %ecx
	movl	-0x44(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_ENCRYPT_ROUND
	movl	-0x40(%r14), %eax
	movl	-0x3C(%r14), %ebx
	movl	-0x38(%r14), %ecx
	movl	-0x34(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_ENCRYPT_ROUND
	movl	-0x30(%r14), %eax
	movl	-0x2C(%r14), %ebx
	movl	-0x28(%r14), %ecx
	movl	-0x24(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_ENCRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_ENCRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_ENCRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_ENCRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 9 */
	AES_ENCRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 10 */
	AES_ENCRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 11 */
	AES_ENCRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 12 */
	AES_ENCRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 13 */
	AES_ENCRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 14, Final Round */
	/* Store sbox pointer */
	movq	.refptr._crypto_aes_sbox(%rip), %r13
	AES_ENCRYPT_ROUND_FINAL
	xorl	0x70(%r14), %r8d
	xorl	0x74(%r14), %r9d
	xorl	0x78(%r14), %r10d
	xorl	0x7C(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Lencrypt256_loop
	.align	16
.Lencrypt256_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret




	.macro	AES_DECRYPT_ROUND
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movl	0x000(%r13,%rbp,4), %r8d
	movl	0x000(%r13,%rsi,4), %r9d
	movl	0x000(%r13,%rdi,4), %r10d
	movl	0x000(%r13,%r12,4), %r11d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	xorl	0x400(%r13,%rbp,4), %r9d
	xorl	0x400(%r13,%rsi,4), %r10d
	xorl	0x400(%r13,%rdi,4), %r11d
	xorl	0x400(%r13,%r12,4), %r8d
	shrl	$16, %eax
	shrl	$16, %ebx
	shrl	$16, %ecx
	shrl	$16, %edx
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	xorl	0x800(%r13,%rbp,4), %r10d
	xorl	0x800(%r13,%rsi,4), %r11d
	xorl	0x800(%r13,%rdi,4), %r8d
	xorl	0x800(%r13,%r12,4), %r9d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	xorl	0xC00(%r13,%rbp,4), %r11d
	xorl	0xC00(%r13,%rsi,4), %r8d
	xorl	0xC00(%r13,%rdi,4), %r9d
	xorl	0xC00(%r13,%r12,4), %r10d
	.endm

	.macro AES_DECRYPT_ROUND_FINAL
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movzbl	(%r13,%rbp), %r8d
	movzbl	(%r13,%rsi), %r9d
	movzbl	(%r13,%rdi), %r10d
	movzbl	(%r13,%r12), %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	rorl	$8, %r10d
	rorl	$8, %r11d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	movb	(%r13,%rbp), %r9b
	movb	(%r13,%rsi), %r10b
	movb	(%r13,%rdi), %r11b
	movb	(%r13,%r12), %r8b
	rorl	$8, %r9d
	rorl	$8, %r10d
	rorl	$8, %r11d
	rorl	$8, %r8d
	shrl	$16, %eax
	shrl	$16, %ebx
	shrl	$16, %ecx
	shrl	$16, %edx
	movzbl	%al, %ebp
	movzbl	%bl, %esi
	movzbl	%cl, %edi
	movzbl	%dl, %r12d
	movb	(%r13,%rbp), %r10b
	movb	(%r13,%rsi), %r11b
	movb	(%r13,%rdi), %r8b
	movb	(%r13,%r12), %r9b
	rorl	$8, %r10d
	rorl	$8, %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	movzbl	%ah, %ebp
	movzbl	%bh, %esi
	movzbl	%ch, %edi
	movzbl	%dh, %r12d
	movb	(%r13,%rbp), %r11b
	movb	(%r13,%rsi), %r8b
	movb	(%r13,%rdi), %r9b
	movb	(%r13,%r12), %r10b
	rorl	$8, %r11d
	rorl	$8, %r8d
	rorl	$8, %r9d
	rorl	$8, %r10d
	.endm

	.align	16
	.globl	_crypto_aes128_decrypt_x86_64
	.def	_crypto_aes128_decrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes128_decrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes128_decrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Ldecrypt128_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x30(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Ldecrypt128_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store itable pointer */
	movq	.refptr._crypto_aes_itable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add last round key */
	xorl	0x70(%r14), %eax
	xorl	0x74(%r14), %ebx
	xorl	0x78(%r14), %ecx
	xorl	0x7C(%r14), %edx
	/* Round 9 */
	AES_DECRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_DECRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_DECRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_DECRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_DECRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_DECRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_DECRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_DECRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 1 */
	AES_DECRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 0, Final Round */
	/* Store ibox pointer */
	movq	.refptr._crypto_aes_ibox(%rip), %r13
	AES_DECRYPT_ROUND_FINAL
	xorl	-0x30(%r14), %r8d
	xorl	-0x2C(%r14), %r9d
	xorl	-0x28(%r14), %r10d
	xorl	-0x24(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Ldecrypt128_loop
	.align	16
.Ldecrypt128_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	.align	16
	.globl	_crypto_aes192_decrypt_x86_64
	.def	_crypto_aes192_decrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes192_decrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes192_decrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Ldecrypt192_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x50(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Ldecrypt192_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store itable pointer */
	movq	.refptr._crypto_aes_itable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add last round key */
	xorl	0x70(%r14), %eax
	xorl	0x74(%r14), %ebx
	xorl	0x78(%r14), %ecx
	xorl	0x7C(%r14), %edx
	/* Round 11 */
	AES_DECRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 10 */
	AES_DECRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 9 */
	AES_DECRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_DECRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_DECRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_DECRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_DECRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_DECRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_DECRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_DECRYPT_ROUND
	movl	-0x30(%r14), %eax
	movl	-0x2C(%r14), %ebx
	movl	-0x28(%r14), %ecx
	movl	-0x24(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 1 */
	AES_DECRYPT_ROUND
	movl	-0x40(%r14), %eax
	movl	-0x3C(%r14), %ebx
	movl	-0x38(%r14), %ecx
	movl	-0x34(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 0, Final Round */
	/* Store ibox pointer */
	movq	.refptr._crypto_aes_ibox(%rip), %r13
	AES_DECRYPT_ROUND_FINAL
	xorl	-0x50(%r14), %r8d
	xorl	-0x4C(%r14), %r9d
	xorl	-0x48(%r14), %r10d
	xorl	-0x44(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Ldecrypt192_loop
	.align	16
.Ldecrypt192_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret


	.align	16
	.globl	_crypto_aes256_decrypt_x86_64
	.def	_crypto_aes256_decrypt_x86_64;	.scl	2;	.type	32;	.endef
	/* void _aes256_decrypt_x86_64(const uint32_t* key [%rdi], */
	/*                             uint8_t* dst [%rsi], */
	/*                             const uint8_t* src [%rdx], */
	/*                             size_t length [%rcx]) */
_crypto_aes256_decrypt_x86_64:
	/* Clobber Registers */
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$16, %rsp	/* Allocate 16 bytes on stack for loop counter and src pointer */
	cmpq	$16, %rcx
	jb		.Ldecrypt256_end	/* skip to return if there is less than 16 bytes to process */
	shrq	$4, %rcx			/* divide length by 16 */
	movq	%rsi, %r15			/* Move output pointer */
	leaq	0x70(%rdi), %r14	/* Offset and store key pointer */
	movq	%rcx, 0(%rsp)		/* store loop counter on stack */
	movq	%rdx, 8(%rsp)		/* store src pointer on stack */
	.align	16
.Ldecrypt256_loop:
	movq	8(%rsp), %rdx		/* Load src pointer */
	/* Store itable pointer */
	movq	.refptr._crypto_aes_itable(%rip), %r13
	/* Load state */
	movl	0x0(%rdx), %eax
	movl	0x4(%rdx), %ebx
	movl	0x8(%rdx), %ecx
	movl	0xC(%rdx), %edx
	addq	$16, 8(%rsp)		/* Offset src pointer */
	/* Add last round key */
	xorl	0x70(%r14), %eax
	xorl	0x74(%r14), %ebx
	xorl	0x78(%r14), %ecx
	xorl	0x7C(%r14), %edx
	/* Round 13 */
	AES_DECRYPT_ROUND
	movl	0x60(%r14), %eax
	movl	0x64(%r14), %ebx
	movl	0x68(%r14), %ecx
	movl	0x6C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 12 */
	AES_DECRYPT_ROUND
	movl	0x50(%r14), %eax
	movl	0x54(%r14), %ebx
	movl	0x58(%r14), %ecx
	movl	0x5C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 11 */
	AES_DECRYPT_ROUND
	movl	0x40(%r14), %eax
	movl	0x44(%r14), %ebx
	movl	0x48(%r14), %ecx
	movl	0x4C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 10 */
	AES_DECRYPT_ROUND
	movl	0x30(%r14), %eax
	movl	0x34(%r14), %ebx
	movl	0x38(%r14), %ecx
	movl	0x3C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 9 */
	AES_DECRYPT_ROUND
	movl	0x20(%r14), %eax
	movl	0x24(%r14), %ebx
	movl	0x28(%r14), %ecx
	movl	0x2C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 8 */
	AES_DECRYPT_ROUND
	movl	0x10(%r14), %eax
	movl	0x14(%r14), %ebx
	movl	0x18(%r14), %ecx
	movl	0x1C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 7 */
	AES_DECRYPT_ROUND
	movl	0x00(%r14), %eax
	movl	0x04(%r14), %ebx
	movl	0x08(%r14), %ecx
	movl	0x0C(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 6 */
	AES_DECRYPT_ROUND
	movl	-0x10(%r14), %eax
	movl	-0x0C(%r14), %ebx
	movl	-0x08(%r14), %ecx
	movl	-0x04(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 5 */
	AES_DECRYPT_ROUND
	movl	-0x20(%r14), %eax
	movl	-0x1C(%r14), %ebx
	movl	-0x18(%r14), %ecx
	movl	-0x14(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 4 */
	AES_DECRYPT_ROUND
	movl	-0x30(%r14), %eax
	movl	-0x2C(%r14), %ebx
	movl	-0x28(%r14), %ecx
	movl	-0x24(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 3 */
	AES_DECRYPT_ROUND
	movl	-0x40(%r14), %eax
	movl	-0x3C(%r14), %ebx
	movl	-0x38(%r14), %ecx
	movl	-0x34(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 2 */
	AES_DECRYPT_ROUND
	movl	-0x50(%r14), %eax
	movl	-0x4C(%r14), %ebx
	movl	-0x48(%r14), %ecx
	movl	-0x44(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 1 */
	AES_DECRYPT_ROUND
	movl	-0x60(%r14), %eax
	movl	-0x5C(%r14), %ebx
	movl	-0x58(%r14), %ecx
	movl	-0x54(%r14), %edx
	xorl	%r8d,  %eax
	xorl	%r9d,  %ebx
	xorl	%r10d, %ecx
	xorl	%r11d, %edx
	/* Round 0, Final Round */
	/* Store ibox pointer */
	movq	.refptr._crypto_aes_ibox(%rip), %r13
	AES_DECRYPT_ROUND_FINAL
	xorl	-0x70(%r14), %r8d
	xorl	-0x6C(%r14), %r9d
	xorl	-0x68(%r14), %r10d
	xorl	-0x64(%r14), %r11d
	/* Store output */
	movl	%r8d,  0x0(%r15)
	movl	%r9d,  0x4(%r15)
	movl	%r10d, 0x8(%r15)
	movl	%r11d, 0xC(%r15)
	addq	$16, %r15			/* Increment dst pointer */
	/* Decrement loop counter and check loop condition */
	decq	(%rsp)
	jnz		.Ldecrypt256_loop
	.align	16
.Ldecrypt256_end:
	/* Free allocated stack space, restore clobbered registers, and return */
	addq	$16, %rsp	/* Free allocated stack space */
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	ret




	.ident	"GCC: (GNU) 7.3.0"

	.section	.rdata$.refptr._crypto_aes_sbox, "dr"
	.globl	.refptr._crypto_aes_sbox
	.linkonce	discard
.refptr._crypto_aes_sbox:
	.quad	_crypto_aes_sbox

	.section	.rdata$.refptr._crypto_aes_ibox, "dr"
	.globl	.refptr._crypto_aes_ibox
	.linkonce	discard
.refptr._crypto_aes_ibox:
	.quad	_crypto_aes_ibox

	.section	.rdata$.refptr._crypto_aes_dtable, "dr"
	.globl	.refptr._crypto_aes_dtable
	.linkonce	discard
.refptr._crypto_aes_dtable:
	.quad	_crypto_aes_dtable

	.section	.rdata$.refptr._crypto_aes_itable, "dr"
	.globl	.refptr._crypto_aes_itable
	.linkonce	discard
.refptr._crypto_aes_itable:
	.quad	_crypto_aes_itable

	.section	.rdata$.refptr._crypto_aes_mtable, "dr"
	.globl	.refptr._crypto_aes_mtable
	.linkonce	discard
.refptr._crypto_aes_mtable:
	.quad	_crypto_aes_mtable
