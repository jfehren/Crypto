	.file	"sha256-sha.asm"
	.text

	.macro	ROUND4	s0, s1, m, mt0, mt1, mt2, mt3, mt4, k
	movdqa		\mt0, \m
	paddd		\k(%rcx), \m
	sha256rnds2	\s0, \s1
	movdqa		\mt0, \mt4
	palignr		$4, \mt3, \mt4
	paddd		\mt4, \mt1
	sha256msg2	\mt0, \mt1
	pshufd 		$0x0E, \m, \m
	sha256rnds2	\s1, \s0
	sha256msg1	\mt0, \mt3
	.endm

	.section .rdata,"dr"
	.align	64
PSHUFFLE_BYTE_FLIP_MASK:
	.quad	0x0405060700010203
	.quad	0x0c0d0e0f08090a0b

	.text
	.globl	_crypto_sha256_compress_sha
	.def	_crypto_sha256_compress_sha;	.scl	2;	.type	32;	.endef
	// void _sha256_compress_sha(uint32_t *state [%rdi],
	//                           const uint8_t *data [%rsi],
	//                           size_t length [%rdx],
	//                           const uint32_t *K [%rcx])
	.align	16
_crypto_sha256_compress_sha:
	// Clobber xmm8-xmm10
	subq	$0x38, %rsp
	movdqa	%xmm10, 0x20(%rsp)
	movdqa	%xmm9,  0x10(%rsp)
	movdqa	%xmm8,  0x00(%rsp)
	shrq	$6, %rdx	// length /= 64
	jz		.Lend
	// Load initial hash values
	movdqu	0x00(%rdi), %xmm1
	movdqu	0x10(%rdi), %xmm2

	// Need to reorder these appropriately
	// DCBA, HGFE -> ABEF, CDGH
	pshufd	$0xB1, %xmm1, %xmm1	// CDAB
	pshufd	$0x1B, %xmm2, %xmm2	// EFGH
	movdqa	%xmm1, %xmm7
	palignr	$8, %xmm2, %xmm1	// ABEF
	pblendw	$0xF0, %xmm7, %xmm2	// CDGH

	movdqa	PSHUFFLE_BYTE_FLIP_MASK(%rip), %xmm8

	.align	16
.Ltop:
	// Save hash values for addition after processing
	movdqa		%xmm1, %xmm9
	movdqa		%xmm2, %xmm10

	// Rounds 0-3
	movdqu		0x00(%rsi), %xmm0
	pshufb		%xmm8, %xmm0
	movdqa		%xmm0, %xmm3
	paddd		0x00(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1

	// Rounds 4-7
	movdqu		0x10(%rsi), %xmm0
	pshufb		%xmm8, %xmm0
	movdqa		%xmm0, %xmm4
	paddd		0x10(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1
	sha256msg1	%xmm4, %xmm3

	// Rounds 8-11
	movdqu		0x20(%rsi), %xmm0
	pshufb		%xmm8, %xmm0
	movdqa		%xmm0, %xmm5
	paddd		0x20(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1
	sha256msg1	%xmm5, %xmm4

	// Rounds 12-15
	movdqu		0x30(%rsi), %xmm0
	pshufb		%xmm8, %xmm0
	movdqa		%xmm0, %xmm6
	paddd		0x30(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	movdqa		%xmm6, %xmm7
	palignr		$4, %xmm5, %xmm7
	paddd		%xmm7, %xmm3
	sha256msg2	%xmm6, %xmm3
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1
	sha256msg1	%xmm6, %xmm5

	// Rounds 16-19
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7, 0x40
	// Rounds 20-23
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm4, %xmm5, %xmm6, %xmm3, %xmm7, 0x50
	// Rounds 24-27
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm5, %xmm6, %xmm3, %xmm4, %xmm7, 0x60
	// Rounds 28-31
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm6, %xmm3, %xmm4, %xmm5, %xmm7, 0x70
	// Rounds 32-35
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7, 0x80
	// Rounds 36-39
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm4, %xmm5, %xmm6, %xmm3, %xmm7, 0x90
	// Rounds 40-43
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm5, %xmm6, %xmm3, %xmm4, %xmm7, 0xA0
	// Rounds 44-47
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm6, %xmm3, %xmm4, %xmm5, %xmm7, 0xB0
	// Rounds 48-51
	ROUND4		%xmm1, %xmm2, %xmm0, %xmm3, %xmm4, %xmm5, %xmm6, %xmm7, 0xC0

	// Rounds 52-55
	movdqa		%xmm4, %xmm0
	paddd		0xD0(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	movdqa		%xmm4, %xmm7
	palignr		$4, %xmm3, %xmm7
	paddd		%xmm7, %xmm5
	sha256msg2	%xmm4, %xmm5
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1

	// Rounds 56-59
	movdqa		%xmm5, %xmm0
	paddd		0xE0(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	movdqa		%xmm5, %xmm7
	palignr		$4, %xmm4, %xmm7
	paddd		%xmm7, %xmm6
	sha256msg2	%xmm5, %xmm6
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1

	// Rounds 60-63
	movdqa		%xmm6, %xmm0
	paddd		0xF0(%rcx), %xmm0
	sha256rnds2	%xmm1, %xmm2
	pshufd 		$0x0E, %xmm0, %xmm0
	sha256rnds2	%xmm2, %xmm1

	// Add current hash values with previously saved
	paddd	%xmm9, %xmm1
	paddd	%xmm10, %xmm2

	decq	%rdx	// --length
	jnz		.Ltop

	// Write hash values back
	pshufd	$0x1B, %xmm1, %xmm1	// FEBA
	pshufd	$0xB1, %xmm2, %xmm2	// DCHG
	movdqa	%xmm1, %xmm7
	pblendw	$0xF0, %xmm2, %xmm1	// DCBA
	palignr	$8, %xmm7, %xmm2	// HGFE

	movdqu	%xmm1, 0x00(%rdi)
	movdqu	%xmm2, 0x10(%rdi)

.Lend:
	// Restore clobbered registers and return
	movdqa	0x00(%rsp), %xmm8
	movdqa	0x10(%rsp), %xmm9
	movdqa	0x20(%rsp), %xmm10
	addq	$0x38, %rsp
	ret

