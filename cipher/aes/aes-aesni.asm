	.file	"aes-aesni.asm"
	.text

	.macro	AES128_EXPAND	rcon, off
	vaeskeygenassist	$\rcon, %xmm0, %xmm1
	pshufd				$0xFF, %xmm1
	vpslldq				$0x04, %xmm0, %xmm2
	pxor				%xmm2, %xmm0
	pslldq				$0x04, %xmm2
	pxor				%xmm2, %xmm0
	pslldq				$0x04, %xmm2
	pxor				%xmm2, %xmm0
	pxor				%xmm1, %xmm0
	movdqa				%xmm0, \off(%rdi)
	.endm

	.align	16
	.globl	_crypto_aes128_set_key_aesni
	.def	_crypto_aes128_set_key_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes128_set_key_aesni(uint32_t* subkeys [%rdi], */
	/*                            const uint8_t* key [%rsi]) */
_crypto_aes128_set_key_aesni:
	addq			$0x30, %rdi				/* Offset subkeys pointer */
	/* Copy first 4 keys */
	movdqu			0x00(%rsi), %xmm0
	movdqa			%xmm0, -0x30(%rdi)
	/* Key Set 1 */
	AES128_EXPAND	0x01, -0x20
	/* Key Set 2 */
	AES128_EXPAND	0x02, -0x10
	/* Key Set 3 */
	AES128_EXPAND	0x04, 0x00
	/* Key Set 4 */
	AES128_EXPAND	0x08, 0x10
	/* Key Set 5 */
	AES128_EXPAND	0x10, 0x20
	/* Key Set 6 */
	AES128_EXPAND	0x20, 0x30
	/* Key Set 7 */
	AES128_EXPAND	0x40, 0x40
	/* Key Set 8 */
	AES128_EXPAND	0x80, 0x50
	/* Key Set 9 */
	AES128_EXPAND	0x1B, 0x60
	/* Key Set 10 */
	AES128_EXPAND	0x36, 0x70
	/* Return */
	ret


	.macro AES192_EXPAND_A	rcon, off
	movdqa				%xmm2, %xmm4
	vaeskeygenassist	$\rcon, %xmm2, %xmm1
	pshufd				$0x55, %xmm1
	vpslldq				$0x04, %xmm0, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pxor				%xmm1, %xmm0
	vpshufd				$0xFF, %xmm0, %xmm1
	vpslldq				$0x04, %xmm2, %xmm3
	pxor				%xmm3, %xmm2
	pxor				%xmm1, %xmm2
	shufpd				$0x00, %xmm0, %xmm4
	vshufpd				$0x01, %xmm2, %xmm0, %xmm5
	movdqa				%xmm4, \off + 0x00(%rdi)
	movdqa				%xmm5, \off + 0x10(%rdi)
	.endm

	.macro AES192_EXPAND_B	rcon, off
	vaeskeygenassist	$\rcon, %xmm2, %xmm1
	pshufd				$0x55, %xmm1
	vpslldq				$0x04, %xmm0, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pxor				%xmm1, %xmm0
	vpshufd				$0xFF, %xmm0, %xmm1
	vpslldq				$0x04, %xmm2, %xmm3
	pxor				%xmm3, %xmm2
	pxor				%xmm1, %xmm2
	movdqa				%xmm0, \off(%rdi)
	.endm

	.align	16
	.globl	_crypto_aes192_set_key_aesni
	.def	_crypto_aes192_set_key_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes192_set_key_aesni(uint32_t* subkeys [%rdi], */
	/*                            const uint8_t* key [%rsi]) */
_crypto_aes192_set_key_aesni:
	addq			$0x50, %rdi				/* Offset subkeys pointer */
	/* Copy first 6 keys */
	movdqu			0x00(%rsi), %xmm0
	movq			0x10(%rsi), %xmm2
	movdqa			%xmm0, -0x50(%rdi)
	/* Key Set 1 */
	AES192_EXPAND_A	0x01, -0x40
	/* Key Set 2 */
	AES192_EXPAND_B	0x02, -0x20
	/* Key Set 3 */
	AES192_EXPAND_A	0x04, -0x10
	/* Key Set 4 */
	AES192_EXPAND_B	0x08, 0x10
	/* Key Set 5 */
	AES192_EXPAND_A	0x10, 0x20
	/* Key Set 6 */
	AES192_EXPAND_B	0x20, 0x40
	/* Key Set 7 */
	AES192_EXPAND_A	0x40, 0x50
	/* Key Set 8 */
	AES192_EXPAND_B	0x80, 0x70
	/* Return */
	ret


	.macro AES256_EXPAND_A rcon, off
	vaeskeygenassist	$\rcon, %xmm2, %xmm1
	pshufd				$0xFF, %xmm1
	vpslldq				$0x04, %xmm0, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm0
	pxor				%xmm1, %xmm0
	movdqa				%xmm0, \off(%rdi)
	.endm

	.macro AES256_EXPAND rcon, off
	AES256_EXPAND_A		\rcon, \off
	vaeskeygenassist	$0x00, %xmm0, %xmm1
	pshufd				$0xAA, %xmm1
	vpslldq				$0x04, %xmm2, %xmm3
	pxor				%xmm3, %xmm2
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm2
	pslldq				$0x04, %xmm3
	pxor				%xmm3, %xmm2
	pxor				%xmm1, %xmm2
	movdqa				%xmm2, \off + 0x10(%rdi)
	.endm

	.align	16
	.globl	_crypto_aes256_set_key_aesni
	.def	_crypto_aes256_set_key_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes256_set_key_aesni(uint32_t* subkeys [%rdi], */
	/*                            const uint8_t* key [%rsi]) */
_crypto_aes256_set_key_aesni:
	addq			$0x70, %rdi				/* Offset subkeys pointer */
	/* Copy first 8 keys */
	movdqu			0x00(%rsi), %xmm0
	movq			0x10(%rsi), %xmm2
	movdqa			%xmm0, -0x70(%rdi)
	movdqa			%xmm2, -0x60(%rdi)
	/* Key Set 1 */
	AES256_EXPAND	0x01, -0x50
	/* Key Set 2 */
	AES256_EXPAND	0x02, -0x30
	/* Key Set 3 */
	AES256_EXPAND	0x04, -0x10
	/* Key Set 4 */
	AES256_EXPAND	0x08, 0x10
	/* Key Set 5 */
	AES256_EXPAND	0x10, 0x30
	/* Key Set 6 */
	AES256_EXPAND	0x20, 0x50
	/* Key Set 7 */
	AES256_EXPAND_A	0x40, 0x70
	/* Return */
	ret




	.align	16
	.globl	_crypto_aes128_invert_aesni
	.def	_crypto_aes128_invert_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes128_invert_aesni(uint32_t* dst [%rdi], */
	/*                           const uint32_t* src [%rsi]) */
_crypto_aes128_invert_aesni:
	addq	$0x30, %rsi				/* Offset src pointer */
	addq	$0x30, %rdi				/* Offset dst pointer */
	cmpq	%rsi, %rdi
	jne		1f
	/* Copy first 4 and last 4 keys */
	movdqa	-0x30(%rsi), %xmm0
	movdqa	0x70(%rsi), %xmm1
	movdqa	%xmm0, -0x30(%rdi)
	movdqa	%xmm1, 0x70(%rdi)
1:	aesimc	-0x20(%rsi), %xmm0
	aesimc	-0x10(%rsi), %xmm1
	aesimc	0x00(%rsi), %xmm2
	aesimc	0x10(%rsi), %xmm3
	aesimc	0x20(%rsi), %xmm4
	movdqa	%xmm0, -0x20(%rdi)
	movdqa	%xmm1, -0x10(%rdi)
	movdqa	%xmm2,  0x00(%rdi)
	movdqa	%xmm3,  0x10(%rdi)
	movdqa	%xmm4,  0x20(%rdi)
	aesimc	0x30(%rsi), %xmm0
	aesimc	0x40(%rsi), %xmm1
	aesimc	0x50(%rsi), %xmm2
	aesimc	0x60(%rsi), %xmm3
	movdqa	%xmm0, 0x30(%rdi)
	movdqa	%xmm1, 0x40(%rdi)
	movdqa	%xmm2, 0x50(%rdi)
	movdqa	%xmm3, 0x60(%rdi)
	ret


	.align	16
	.globl	_crypto_aes192_invert_aesni
	.def	_crypto_aes192_invert_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes192_invert_aesni(uint32_t* dst [%rdi], */
	/*                           const uint32_t* src [%rsi]) */
_crypto_aes192_invert_aesni:
	addq	$0x50, %rsi				/* Offset src pointer */
	addq	$0x50, %rdi				/* Offset dst pointer */
	cmpq	%rsi, %rdi
	jne		1f
	/* Copy first 4 and last 4 keys */
	movdqa	-0x50(%rsi), %xmm0
	movdqa	0x70(%rsi), %xmm1
	movdqa	%xmm0, -0x50(%rdi)
	movdqa	%xmm1, 0x70(%rdi)
1:	aesimc	-0x40(%rsi), %xmm0
	aesimc	-0x30(%rsi), %xmm1
	aesimc	-0x20(%rsi), %xmm2
	aesimc	-0x10(%rsi), %xmm3
	aesimc	0x00(%rsi), %xmm4
	aesimc	0x10(%rsi), %xmm5
	movdqa	%xmm0, -0x40(%rdi)
	movdqa	%xmm1, -0x30(%rdi)
	movdqa	%xmm2, -0x20(%rdi)
	movdqa	%xmm3, -0x10(%rdi)
	movdqa	%xmm4,  0x00(%rdi)
	movdqa	%xmm5,  0x10(%rdi)
	aesimc	0x20(%rsi), %xmm0
	aesimc	0x30(%rsi), %xmm1
	aesimc	0x40(%rsi), %xmm2
	aesimc	0x50(%rsi), %xmm3
	aesimc	0x60(%rsi), %xmm4
	movdqa	%xmm0, 0x20(%rdi)
	movdqa	%xmm1, 0x30(%rdi)
	movdqa	%xmm2, 0x40(%rdi)
	movdqa	%xmm3, 0x50(%rdi)
	movdqa	%xmm4, 0x60(%rdi)
	ret


	.align	16
	.globl	_crypto_aes256_invert_aesni
	.def	_crypto_aes256_invert_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes256_invert_aesni(uint32_t* dst [%rdi], */
	/*                           const uint32_t* src [%rsi]) */
_crypto_aes256_invert_aesni:
	addq	$0x70, %rsi				/* Offset src pointer */
	addq	$0x70, %rdi				/* Offset dst pointer */
	cmpq	%rsi, %rdi
	jne		1f
	/* Copy first 4 and last 4 keys */
	movdqa	-0x70(%rsi), %xmm0
	movdqa	0x70(%rsi), %xmm1
	movdqa	%xmm0, -0x70(%rdi)
	movdqa	%xmm1, 0x70(%rdi)
1:	aesimc	-0x60(%rsi), %xmm0
	aesimc	-0x50(%rsi), %xmm1
	aesimc	-0x40(%rsi), %xmm2
	aesimc	-0x30(%rsi), %xmm3
	aesimc	-0x20(%rsi), %xmm4
	movdqa	%xmm0, -0x60(%rdi)
	movdqa	%xmm1, -0x50(%rdi)
	movdqa	%xmm2, -0x40(%rdi)
	movdqa	%xmm3, -0x30(%rdi)
	movdqa	%xmm4, -0x20(%rdi)
	aesimc	-0x10(%rsi), %xmm0
	aesimc	0x00(%rsi), %xmm1
	aesimc	0x10(%rsi), %xmm2
	aesimc	0x20(%rsi), %xmm3
	aesimc	0x30(%rsi), %xmm4
	movdqa	%xmm0, -0x10(%rdi)
	movdqa	%xmm1, 0x00(%rdi)
	movdqa	%xmm2, 0x10(%rdi)
	movdqa	%xmm3, 0x20(%rdi)
	movdqa	%xmm4, 0x30(%rdi)
	aesimc	0x40(%rsi), %xmm0
	aesimc	0x50(%rsi), %xmm1
	aesimc	0x60(%rsi), %xmm2
	movdqa	%xmm0, 0x40(%rdi)
	movdqa	%xmm1, 0x50(%rdi)
	movdqa	%xmm2, 0x60(%rdi)
	ret




	.align	16
	.globl	_crypto_aes128_encrypt_aesni
	.def	_crypto_aes128_encrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes128_encrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes128_encrypt_aesni:
	subq		$0x28, %rsp
	movdqa		%xmm6, 0x00(%rsp)
	movdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Lencrypt128_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x30, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and encrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Lencrypt128_loop
	subq		%rax, %rcx
.Lencrypt128_pre:
	/* 1 block outside of loop */
	movdqu		(%rdx), %xmm0
	pxor		-0x30(%rdi), %xmm0, %xmm0
	aesenc		-0x20(%rdi), %xmm0, %xmm0
	aesenc		-0x10(%rdi), %xmm0, %xmm0
	aesenc		0x00(%rdi), %xmm0, %xmm0
	aesenc		0x10(%rdi), %xmm0, %xmm0
	aesenc		0x20(%rdi), %xmm0, %xmm0
	aesenc		0x30(%rdi), %xmm0, %xmm0
	aesenc		0x40(%rdi), %xmm0, %xmm0
	aesenc		0x50(%rdi), %xmm0, %xmm0
	aesenc		0x60(%rdi), %xmm0, %xmm0
	aesenclast	0x70(%rdi), %xmm0, %xmm0
	movdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Lencrypt128_pre
	.align	16
.Lencrypt128_loop:
	movdqu		0x00(%rdx), %xmm0
	movdqu		0x10(%rdx), %xmm1
	movdqu		0x20(%rdx), %xmm2
	movdqu		0x30(%rdx), %xmm3
	movdqa		-0x30(%rdi), %xmm5
	movdqa		-0x20(%rdi), %xmm6
	movdqa		-0x10(%rdi), %xmm7
	pxor		%xmm5, %xmm0, %xmm0
	pxor		%xmm5, %xmm1, %xmm1
	pxor		%xmm5, %xmm2, %xmm2
	pxor		%xmm5, %xmm3, %xmm3
	aesenc		%xmm6, %xmm0, %xmm0
	aesenc		%xmm6, %xmm1, %xmm1
	aesenc		%xmm6, %xmm2, %xmm2
	aesenc		%xmm6, %xmm3, %xmm3
	aesenc		%xmm7, %xmm0, %xmm0
	aesenc		%xmm7, %xmm1, %xmm1
	aesenc		%xmm7, %xmm2, %xmm2
	aesenc		%xmm7, %xmm3, %xmm3
	movdqa		0x00(%rdi), %xmm4
	movdqa		0x10(%rdi), %xmm5
	movdqa		0x20(%rdi), %xmm6
	movdqa		0x30(%rdi), %xmm7
	aesenc		%xmm4, %xmm0, %xmm0
	aesenc		%xmm4, %xmm1, %xmm1
	aesenc		%xmm4, %xmm2, %xmm2
	aesenc		%xmm4, %xmm3, %xmm3
	aesenc		%xmm5, %xmm0, %xmm0
	aesenc		%xmm5, %xmm1, %xmm1
	aesenc		%xmm5, %xmm2, %xmm2
	aesenc		%xmm5, %xmm3, %xmm3
	aesenc		%xmm6, %xmm0, %xmm0
	aesenc		%xmm6, %xmm1, %xmm1
	aesenc		%xmm6, %xmm2, %xmm2
	aesenc		%xmm6, %xmm3, %xmm3
	aesenc		%xmm7, %xmm0, %xmm0
	aesenc		%xmm7, %xmm1, %xmm1
	aesenc		%xmm7, %xmm2, %xmm2
	aesenc		%xmm7, %xmm3, %xmm3
	movdqa		0x40(%rdi), %xmm4
	movdqa		0x50(%rdi), %xmm5
	movdqa		0x60(%rdi), %xmm6
	movdqa		0x70(%rdi), %xmm7
	aesenc		%xmm4, %xmm0, %xmm0
	aesenc		%xmm4, %xmm1, %xmm1
	aesenc		%xmm4, %xmm2, %xmm2
	aesenc		%xmm4, %xmm3, %xmm3
	aesenc		%xmm5, %xmm0, %xmm0
	aesenc		%xmm5, %xmm1, %xmm1
	aesenc		%xmm5, %xmm2, %xmm2
	aesenc		%xmm5, %xmm3, %xmm3
	aesenc		%xmm6, %xmm0, %xmm0
	aesenc		%xmm6, %xmm1, %xmm1
	aesenc		%xmm6, %xmm2, %xmm2
	aesenc		%xmm6, %xmm3, %xmm3
	aesenclast	%xmm7, %xmm0, %xmm0
	aesenclast	%xmm7, %xmm1, %xmm1
	aesenclast	%xmm7, %xmm2, %xmm2
	aesenclast	%xmm7, %xmm3, %xmm3
	movdqu		%xmm0, 0x00(%rsi)
	movdqu		%xmm1, 0x10(%rsi)
	movdqu		%xmm2, 0x20(%rsi)
	movdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	jnz			.Lencrypt128_loop
	.align	16
.Lencrypt128_end:
	movdqa		0x10(%rsp), %xmm7
	movdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret


	.align	16
	.globl	_crypto_aes192_encrypt_aesni
	.def	_crypto_aes192_encrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes192_encrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes192_encrypt_aesni:
	subq		$0x28, %rsp
	vmovdqa		%xmm6, 0x00(%rsp)
	vmovdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Lencrypt192_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x50, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and encrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Lencrypt192_loop
	subq		%rax, %rcx
.Lencrypt192_pre:
	/* 1 block outside of loop */
	vmovdqu		(%rdx), %xmm0
	vpxor		-0x50(%rdi), %xmm0, %xmm0
	vaesenc		-0x40(%rdi), %xmm0, %xmm0
	vaesenc		-0x30(%rdi), %xmm0, %xmm0
	vaesenc		-0x20(%rdi), %xmm0, %xmm0
	vaesenc		-0x10(%rdi), %xmm0, %xmm0
	vaesenc		0x00(%rdi), %xmm0, %xmm0
	vaesenc		0x10(%rdi), %xmm0, %xmm0
	vaesenc		0x20(%rdi), %xmm0, %xmm0
	vaesenc		0x30(%rdi), %xmm0, %xmm0
	vaesenc		0x40(%rdi), %xmm0, %xmm0
	vaesenc		0x50(%rdi), %xmm0, %xmm0
	vaesenc		0x60(%rdi), %xmm0, %xmm0
	vaesenclast	0x70(%rdi), %xmm0, %xmm0
	vmovdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Lencrypt192_pre
	.align	16
.Lencrypt192_loop:
	vmovdqu		0x00(%rdx), %xmm0
	vmovdqu		0x10(%rdx), %xmm1
	vmovdqu		0x20(%rdx), %xmm2
	vmovdqu		0x30(%rdx), %xmm3
	vmovdqa		-0x50(%rdi), %xmm7
	vpxor		%xmm7, %xmm0, %xmm0
	vpxor		%xmm7, %xmm1, %xmm1
	vpxor		%xmm7, %xmm2, %xmm2
	vpxor		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x40(%rdi), %xmm4
	vmovdqa		-0x30(%rdi), %xmm5
	vmovdqa		-0x20(%rdi), %xmm6
	vmovdqa		-0x10(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenc		%xmm7, %xmm0, %xmm0
	vaesenc		%xmm7, %xmm1, %xmm1
	vaesenc		%xmm7, %xmm2, %xmm2
	vaesenc		%xmm7, %xmm3, %xmm3
	vmovdqa		0x00(%rdi), %xmm4
	vmovdqa		0x10(%rdi), %xmm5
	vmovdqa		0x20(%rdi), %xmm6
	vmovdqa		0x30(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenc		%xmm7, %xmm0, %xmm0
	vaesenc		%xmm7, %xmm1, %xmm1
	vaesenc		%xmm7, %xmm2, %xmm2
	vaesenc		%xmm7, %xmm3, %xmm3
	vmovdqa		0x40(%rdi), %xmm4
	vmovdqa		0x50(%rdi), %xmm5
	vmovdqa		0x60(%rdi), %xmm6
	vmovdqa		0x70(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenclast	%xmm7, %xmm0, %xmm0
	vaesenclast	%xmm7, %xmm1, %xmm1
	vaesenclast	%xmm7, %xmm2, %xmm2
	vaesenclast	%xmm7, %xmm3, %xmm3
	vmovdqu		%xmm0, 0x00(%rsi)
	vmovdqu		%xmm1, 0x10(%rsi)
	vmovdqu		%xmm2, 0x20(%rsi)
	vmovdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	ja			.Lencrypt192_loop
	.align	16
.Lencrypt192_end:
	vmovdqa		0x10(%rsp), %xmm7
	vmovdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret


	.align	16
	.globl	_crypto_aes256_encrypt_aesni
	.def	_crypto_aes256_encrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes256_encrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes256_encrypt_aesni:
	subq		$0x28, %rsp
	vmovdqa		%xmm6, 0x00(%rsp)
	vmovdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Lencrypt256_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x70, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and encrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Lencrypt256_loop
	subq		%rax, %rcx
.Lencrypt256_pre:
	/* 1 block outside of loop */
	vmovdqu		(%rdx), %xmm0
	vpxor		-0x70(%rdi), %xmm0, %xmm0
	vaesenc		-0x60(%rdi), %xmm0, %xmm0
	vaesenc		-0x50(%rdi), %xmm0, %xmm0
	vaesenc		-0x40(%rdi), %xmm0, %xmm0
	vaesenc		-0x30(%rdi), %xmm0, %xmm0
	vaesenc		-0x20(%rdi), %xmm0, %xmm0
	vaesenc		-0x10(%rdi), %xmm0, %xmm0
	vaesenc		0x00(%rdi), %xmm0, %xmm0
	vaesenc		0x10(%rdi), %xmm0, %xmm0
	vaesenc		0x20(%rdi), %xmm0, %xmm0
	vaesenc		0x30(%rdi), %xmm0, %xmm0
	vaesenc		0x40(%rdi), %xmm0, %xmm0
	vaesenc		0x50(%rdi), %xmm0, %xmm0
	vaesenc		0x60(%rdi), %xmm0, %xmm0
	vaesenclast	0x70(%rdi), %xmm0, %xmm0
	vmovdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Lencrypt256_pre
	.align	16
.Lencrypt256_loop:
	vmovdqu		0x00(%rdx), %xmm0
	vmovdqu		0x10(%rdx), %xmm1
	vmovdqu		0x20(%rdx), %xmm2
	vmovdqu		0x30(%rdx), %xmm3
	vmovdqa		-0x70(%rdi), %xmm5
	vmovdqa		-0x60(%rdi), %xmm6
	vmovdqa		-0x50(%rdi), %xmm7
	vpxor		%xmm5, %xmm0, %xmm0
	vpxor		%xmm5, %xmm1, %xmm1
	vpxor		%xmm5, %xmm2, %xmm2
	vpxor		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenc		%xmm7, %xmm0, %xmm0
	vaesenc		%xmm7, %xmm1, %xmm1
	vaesenc		%xmm7, %xmm2, %xmm2
	vaesenc		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x40(%rdi), %xmm4
	vmovdqa		-0x30(%rdi), %xmm5
	vmovdqa		-0x20(%rdi), %xmm6
	vmovdqa		-0x10(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenc		%xmm7, %xmm0, %xmm0
	vaesenc		%xmm7, %xmm1, %xmm1
	vaesenc		%xmm7, %xmm2, %xmm2
	vaesenc		%xmm7, %xmm3, %xmm3
	vmovdqa		0x00(%rdi), %xmm4
	vmovdqa		0x10(%rdi), %xmm5
	vmovdqa		0x20(%rdi), %xmm6
	vmovdqa		0x30(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenc		%xmm7, %xmm0, %xmm0
	vaesenc		%xmm7, %xmm1, %xmm1
	vaesenc		%xmm7, %xmm2, %xmm2
	vaesenc		%xmm7, %xmm3, %xmm3
	vmovdqa		0x40(%rdi), %xmm4
	vmovdqa		0x50(%rdi), %xmm5
	vmovdqa		0x60(%rdi), %xmm6
	vmovdqa		0x70(%rdi), %xmm7
	vaesenc		%xmm4, %xmm0, %xmm0
	vaesenc		%xmm4, %xmm1, %xmm1
	vaesenc		%xmm4, %xmm2, %xmm2
	vaesenc		%xmm4, %xmm3, %xmm3
	vaesenc		%xmm5, %xmm0, %xmm0
	vaesenc		%xmm5, %xmm1, %xmm1
	vaesenc		%xmm5, %xmm2, %xmm2
	vaesenc		%xmm5, %xmm3, %xmm3
	vaesenc		%xmm6, %xmm0, %xmm0
	vaesenc		%xmm6, %xmm1, %xmm1
	vaesenc		%xmm6, %xmm2, %xmm2
	vaesenc		%xmm6, %xmm3, %xmm3
	vaesenclast	%xmm7, %xmm0, %xmm0
	vaesenclast	%xmm7, %xmm1, %xmm1
	vaesenclast	%xmm7, %xmm2, %xmm2
	vaesenclast	%xmm7, %xmm3, %xmm3
	vmovdqu		%xmm0, 0x00(%rsi)
	vmovdqu		%xmm1, 0x10(%rsi)
	vmovdqu		%xmm2, 0x20(%rsi)
	vmovdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	ja			.Lencrypt256_loop
	.align	16
.Lencrypt256_end:
	vmovdqa		0x10(%rsp), %xmm7
	vmovdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret




	.align	16
	.globl	_crypto_aes128_decrypt_aesni
	.def	_crypto_aes128_decrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes128_decrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes128_decrypt_aesni:
	subq		$0x28, %rsp
	vmovdqa		%xmm6, 0x00(%rsp)
	vmovdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Ldecrypt128_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x30, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and encrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Ldecrypt128_loop
	subq		%rax, %rcx
.Ldecrypt128_pre:
	/* 1 block outside of loop */
	vmovdqu		(%rdx), %xmm0
	vpxor		0x70(%rdi), %xmm0, %xmm0
	vaesdec		0x60(%rdi), %xmm0, %xmm0
	vaesdec		0x50(%rdi), %xmm0, %xmm0
	vaesdec		0x40(%rdi), %xmm0, %xmm0
	vaesdec		0x30(%rdi), %xmm0, %xmm0
	vaesdec		0x20(%rdi), %xmm0, %xmm0
	vaesdec		0x10(%rdi), %xmm0, %xmm0
	vaesdec		0x00(%rdi), %xmm0, %xmm0
	vaesdec		-0x10(%rdi), %xmm0, %xmm0
	vaesdec		-0x20(%rdi), %xmm0, %xmm0
	vaesdeclast	-0x30(%rdi), %xmm0, %xmm0
	vmovdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Ldecrypt128_pre
	.align	16
.Ldecrypt128_loop:
	vmovdqu		0x00(%rdx), %xmm0
	vmovdqu		0x10(%rdx), %xmm1
	vmovdqu		0x20(%rdx), %xmm2
	vmovdqu		0x30(%rdx), %xmm3
	vmovdqa		0x70(%rdi), %xmm5
	vmovdqa		0x60(%rdi), %xmm6
	vmovdqa		0x50(%rdi), %xmm7
	vpxor		%xmm5, %xmm0, %xmm0
	vpxor		%xmm5, %xmm1, %xmm1
	vpxor		%xmm5, %xmm2, %xmm2
	vpxor		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		0x40(%rdi), %xmm4
	vmovdqa		0x30(%rdi), %xmm5
	vmovdqa		0x20(%rdi), %xmm6
	vmovdqa		0x10(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x00(%rdi), %xmm4
	vmovdqa		-0x10(%rdi), %xmm5
	vmovdqa		-0x20(%rdi), %xmm6
	vmovdqa		-0x30(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdeclast	%xmm7, %xmm0, %xmm0
	vaesdeclast	%xmm7, %xmm1, %xmm1
	vaesdeclast	%xmm7, %xmm2, %xmm2
	vaesdeclast	%xmm7, %xmm3, %xmm3
	vmovdqu		%xmm0, 0x00(%rsi)
	vmovdqu		%xmm1, 0x10(%rsi)
	vmovdqu		%xmm2, 0x20(%rsi)
	vmovdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	jnz			.Ldecrypt128_loop
	.align	16
.Ldecrypt128_end:
	vmovdqa		0x10(%rsp), %xmm7
	vmovdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret


	.align	16
	.globl	_crypto_aes192_decrypt_aesni
	.def	_crypto_aes192_decrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes192_decrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes192_decrypt_aesni:
	subq		$0x28, %rsp
	vmovdqa		%xmm6, 0x00(%rsp)
	vmovdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Ldecrypt192_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x50, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and decrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Ldecrypt192_loop
	subq		%rax, %rcx
.Ldecrypt192_pre:
	/* 1 block outside of loop */
	vmovdqu		(%rdx), %xmm0
	vpxor		0x70(%rdi), %xmm0, %xmm0
	vaesdec		0x60(%rdi), %xmm0, %xmm0
	vaesdec		0x50(%rdi), %xmm0, %xmm0
	vaesdec		0x40(%rdi), %xmm0, %xmm0
	vaesdec		0x30(%rdi), %xmm0, %xmm0
	vaesdec		0x20(%rdi), %xmm0, %xmm0
	vaesdec		0x10(%rdi), %xmm0, %xmm0
	vaesdec		0x00(%rdi), %xmm0, %xmm0
	vaesdec		-0x10(%rdi), %xmm0, %xmm0
	vaesdec		-0x20(%rdi), %xmm0, %xmm0
	vaesdec		-0x30(%rdi), %xmm0, %xmm0
	vaesdec		-0x40(%rdi), %xmm0, %xmm0
	vaesdeclast	-0x50(%rdi), %xmm0, %xmm0
	vmovdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Ldecrypt192_pre
	.align	16
.Ldecrypt192_loop:
	vmovdqu		0x00(%rdx), %xmm0
	vmovdqu		0x10(%rdx), %xmm1
	vmovdqu		0x20(%rdx), %xmm2
	vmovdqu		0x30(%rdx), %xmm3
	vmovdqa		0x70(%rdi), %xmm7
	vpxor		%xmm7, %xmm0, %xmm0
	vpxor		%xmm7, %xmm1, %xmm1
	vpxor		%xmm7, %xmm2, %xmm2
	vpxor		%xmm7, %xmm3, %xmm3
	vmovdqa		0x60(%rdi), %xmm4
	vmovdqa		0x50(%rdi), %xmm5
	vmovdqa		0x40(%rdi), %xmm6
	vmovdqa		0x30(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		0x20(%rdi), %xmm4
	vmovdqa		0x10(%rdi), %xmm5
	vmovdqa		0x00(%rdi), %xmm6
	vmovdqa		-0x10(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x20(%rdi), %xmm4
	vmovdqa		-0x30(%rdi), %xmm5
	vmovdqa		-0x40(%rdi), %xmm6
	vmovdqa		-0x50(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdeclast	%xmm7, %xmm0, %xmm0
	vaesdeclast	%xmm7, %xmm1, %xmm1
	vaesdeclast	%xmm7, %xmm2, %xmm2
	vaesdeclast	%xmm7, %xmm3, %xmm3
	vmovdqu		%xmm0, 0x00(%rsi)
	vmovdqu		%xmm1, 0x10(%rsi)
	vmovdqu		%xmm2, 0x20(%rsi)
	vmovdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	ja			.Ldecrypt192_loop
	.align	16
.Ldecrypt192_end:
	vmovdqa		0x10(%rsp), %xmm7
	vmovdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret


	.align	16
	.globl	_crypto_aes256_decrypt_aesni
	.def	_crypto_aes256_decrypt_aesni;	.scl	2;	.type	32;	.endef
	/* void _aes256_decrypt_aesni(const uint32_t* key [%rdi], */
	/*                            uint8_t* dst [%rsi], */
	/*                            const uint8_t* src [%rdx], */
	/*                            size_t length [%rcx]) */
_crypto_aes256_decrypt_aesni:
	subq		$0x28, %rsp
	vmovdqa		%xmm6, 0x00(%rsp)
	vmovdqa		%xmm7, 0x10(%rsp)
	cmpq		$16, %rcx
	jb			.Ldecrypt256_end
	shrq		$4, %rcx			/* Divide length by 16 */
	addq		$0x70, %rdi			/* Offset key pointer */
	movq		%rcx, %rax
	/* Check (length / 16) mod 4, and decrypt that many blocks outside of the loop */
	andl		$3, %eax
	jz			.Ldecrypt256_loop
	subq		%rax, %rcx
.Ldecrypt256_pre:
	/* 1 block outside of loop */
	vmovdqu		(%rdx), %xmm0
	vpxor		0x70(%rdi), %xmm0, %xmm0
	vaesdec		0x60(%rdi), %xmm0, %xmm0
	vaesdec		0x50(%rdi), %xmm0, %xmm0
	vaesdec		0x40(%rdi), %xmm0, %xmm0
	vaesdec		0x30(%rdi), %xmm0, %xmm0
	vaesdec		0x20(%rdi), %xmm0, %xmm0
	vaesdec		0x10(%rdi), %xmm0, %xmm0
	vaesdec		0x00(%rdi), %xmm0, %xmm0
	vaesdec		-0x10(%rdi), %xmm0, %xmm0
	vaesdec		-0x20(%rdi), %xmm0, %xmm0
	vaesdec		-0x30(%rdi), %xmm0, %xmm0
	vaesdec		-0x40(%rdi), %xmm0, %xmm0
	vaesdec		-0x50(%rdi), %xmm0, %xmm0
	vaesdec		-0x60(%rdi), %xmm0, %xmm0
	vaesdeclast	-0x70(%rdi), %xmm0, %xmm0
	vmovdqu		%xmm0, (%rsi)
	addq		$16, %rdx
	addq		$16, %rsi
	decl		%eax
	jnz			.Ldecrypt256_pre
	.align	16
.Ldecrypt256_loop:
	vmovdqu		0x00(%rdx), %xmm0
	vmovdqu		0x10(%rdx), %xmm1
	vmovdqu		0x20(%rdx), %xmm2
	vmovdqu		0x30(%rdx), %xmm3
	vmovdqa		0x70(%rdi), %xmm5
	vmovdqa		0x60(%rdi), %xmm6
	vmovdqa		0x50(%rdi), %xmm7
	vpxor		%xmm5, %xmm0, %xmm0
	vpxor		%xmm5, %xmm1, %xmm1
	vpxor		%xmm5, %xmm2, %xmm2
	vpxor		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		0x40(%rdi), %xmm4
	vmovdqa		0x30(%rdi), %xmm5
	vmovdqa		0x20(%rdi), %xmm6
	vmovdqa		0x10(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x00(%rdi), %xmm4
	vmovdqa		-0x10(%rdi), %xmm5
	vmovdqa		-0x20(%rdi), %xmm6
	vmovdqa		-0x30(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdec		%xmm7, %xmm0, %xmm0
	vaesdec		%xmm7, %xmm1, %xmm1
	vaesdec		%xmm7, %xmm2, %xmm2
	vaesdec		%xmm7, %xmm3, %xmm3
	vmovdqa		-0x40(%rdi), %xmm4
	vmovdqa		-0x50(%rdi), %xmm5
	vmovdqa		-0x60(%rdi), %xmm6
	vmovdqa		-0x70(%rdi), %xmm7
	vaesdec		%xmm4, %xmm0, %xmm0
	vaesdec		%xmm4, %xmm1, %xmm1
	vaesdec		%xmm4, %xmm2, %xmm2
	vaesdec		%xmm4, %xmm3, %xmm3
	vaesdec		%xmm5, %xmm0, %xmm0
	vaesdec		%xmm5, %xmm1, %xmm1
	vaesdec		%xmm5, %xmm2, %xmm2
	vaesdec		%xmm5, %xmm3, %xmm3
	vaesdec		%xmm6, %xmm0, %xmm0
	vaesdec		%xmm6, %xmm1, %xmm1
	vaesdec		%xmm6, %xmm2, %xmm2
	vaesdec		%xmm6, %xmm3, %xmm3
	vaesdeclast	%xmm7, %xmm0, %xmm0
	vaesdeclast	%xmm7, %xmm1, %xmm1
	vaesdeclast	%xmm7, %xmm2, %xmm2
	vaesdeclast	%xmm7, %xmm3, %xmm3
	vmovdqu		%xmm0, 0x00(%rsi)
	vmovdqu		%xmm1, 0x10(%rsi)
	vmovdqu		%xmm2, 0x20(%rsi)
	vmovdqu		%xmm3, 0x30(%rsi)
	addq		$64, %rdx
	addq		$64, %rsi
	subq		$4, %rcx
	ja			.Ldecrypt256_loop
	.align	16
.Ldecrypt256_end:
	vmovdqa		0x10(%rsp), %xmm7
	vmovdqa		0x00(%rsp), %xmm6
	addq		$0x28, %rsp
	ret

