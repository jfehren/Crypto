	.file	"sha1-sha.asm"


	.macro	ROUND4	abcd, e0, e1, m0, m1, m2, m3, k
	sha1nexte	\m0, \e0
	movdqa		\abcd, \e1
	sha1msg2	\m0, \m1
	sha1rnds4	$\k, \e0, \abcd
	sha1msg1	\m0, \m3
	pxor		\m0, \m2
	.endm


	.section .rdata,"dr"
	.align	64
PSHUFFLE_BYTE_FLIP_MASK:
	.quad	0x08090a0b0c0d0e0f
	.quad	0x0001020304050607
UPPER_WORD_MASK:
	.quad	0x0000000000000000
	.quad	0xFFFFFFFF00000000

	.text
	.globl	_crypto_sha1_compress_sha
	.def	_crypto_sha1_compress_sha;	.scl	2;	.type	32;	.endef
	/* void _sha1_compress_sha(uint32_t *state [%rdi], */
	/*                         const uint8_t *data [%rsi], */
	/*                         size_t length [%rdx]) */
	.align	16
_crypto_sha1_compress_sha:
	/* Allocate 32 bytes of 16-bytes aligned data on the stack */
	subq	$0x28, %rsp
	shrq	$6, %rdx	/* length /= 64 */
	jz		.Lend
	/* Load initial hash values */
	pinsrd	3, 0x10(%rdi), %xmm1	/* Load E */
	movdqu	0x00(%rdi), %xmm0		/* Load A, B, C, and D */
	pand	UPPER_WORD_MASK(%rip), %xmm1	/* Clear unused words from xmm1 */
	pshufd	$0x1B, %xmm0, %xmm0		/* Reverse the order of A, B, C, and D in xmm0 */
	movdqa	PSHUFFLE_BYTE_FLIP_MASK(%rip), %xmm7	/* Load the byte-swap mask */
	.align	16
.Ltop:
	/* Save hash values for addition after processing */
	movdqa	%xmm0, 0x00(%rsp)
	movdqa	%xmm1, 0x10(%rsp)

	/* Load the message */
	movdqu	0x00(%rsi), %xmm3
	movdqu	0x10(%rsi), %xmm4
	movdqu	0x20(%rsi), %xmm5
	movdqu	0x30(%rsi), %xmm6
	/* reverse the bytes of the message, since SHA-1 uses big-endian byte ordering */
	pshufb	%xmm7, %xmm3
	pshufb	%xmm7, %xmm4
	pshufb	%xmm7, %xmm5
	pshufb	%xmm7, %xmm6
	addq	$64, %rsi		/* offset data pointer */

	/* Rounds 0-3 */
	paddd		%xmm3, %xmm1
	movdqa		%xmm0, %xmm2
	sha1rnds4	$0, %xmm1, %xmm0
	/* Rounds 4-7 */
	sha1nexte	%xmm4, %xmm2
	movdqa		%xmm0, %xmm1
	sha1rnds4	$0, %xmm2, %xmm0
	sha1msg1	%xmm4, %xmm3
	/* Rounds 8-11 */
	sha1nexte	%xmm5, %xmm1
	movdqa		%xmm0, %xmm2
	sha1rnds4	$0, %xmm1, %xmm0
	sha1msg1	%xmm5, %xmm4
	pxor		%xmm5, %xmm3
	/* Rounds 12-15 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm6, %xmm3, %xmm4, %xmm5, 0
	/* Rounds 16-19 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, 0
	/* Rounds 20-23 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm4, %xmm5, %xmm6, %xmm3, 1
	/* Rounds 24-27 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm5, %xmm6, %xmm3, %xmm4, 1
	/* Rounds 28-31 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm6, %xmm3, %xmm4, %xmm5, 1
	/* Rounds 32-35 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, 1
	/* Rounds 36-39 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm4, %xmm5, %xmm6, %xmm3, 1
	/* Rounds 40-43 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm5, %xmm6, %xmm3, %xmm4, 2
	/* Rounds 44-47 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm6, %xmm3, %xmm4, %xmm5, 2
	/* Rounds 48-51 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm3, %xmm4, %xmm5, %xmm6, 2
	/* Rounds 52-55 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm4, %xmm5, %xmm6, %xmm3, 2
	/* Rounds 56-59 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm5, %xmm6, %xmm3, %xmm4, 2
	/* Rounds 60-63 */
	ROUND4	%xmm0, %xmm1, %xmm2, %xmm6, %xmm3, %xmm4, %xmm5, 3
	/* Rounds 64-67 */
	ROUND4	%xmm0, %xmm2, %xmm1, %xmm3, %xmm4, %xmm5, %xmm6, 3
	/* Rounds 68-71 */
	sha1nexte	%xmm4, %xmm2
	movdqa		%xmm0, %xmm1
	sha1msg2	%xmm4, %xmm5
	sha1rnds4	$3, %xmm2, %xmm0
	pxor		%xmm4, %xmm6
	/* Rounds 72-75 */
	sha1nexte	%xmm5, %xmm1
	movdqa		%xmm0, %xmm2
	sha1msg2	%xmm5, %xmm6
	sha1rnds4	$3, %xmm1, %xmm0
	/* Rounds 76-79 */
	sha1nexte	%xmm6, %xmm2
	movdqa		%xmm0, %xmm1
	sha1rnds4	$3, %xmm2, %xmm0

	/* Add saved hash to current hash */
	sha1nexte	0x10(%rsp), %xmm1
	paddd		0x00(%rsp), %xmm0

	decq	%rdx	/* --length */
	jnz		.Ltop

	/* Write hash values back */
	pshufd	0x1B, %xmm0, %xmm0
	movdqu	%xmm0, 0x00(%rdi)
	pextrd	$3, %xmm1, 0x10(%rdi)

.Lend:
	/* Restore stack pointer and return */
	addq	$0x28, %rsp
	ret

