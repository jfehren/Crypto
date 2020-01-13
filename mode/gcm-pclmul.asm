	.file	"gcm-pclmul.asm"

	.section	.rdata, "dr"
	.align	16
bswap:
	.byte	0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08
	.byte	0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00

/*
struct ghash_pclmul {
	union crypto_block16 d;       (0x00)
	union crypto_block16 h1;      (0x10)
	union crypto_block16 h2;      (0x20)
	union crypto_block16 h3;      (0x30)
	union crypto_block16 h4;      (0x40)
	union crypto_block16 h1_h2_m; (0x50)
	union crypto_block16 h3_h4_m; (0x60)
*/

	.text
	.align	16
	.def	gcm_gf_reduce;	.scl	2;	.type	32;	.endef
	/* __m128i [xmm0] gcm_gf_reduce(__m128i h [xmm0], __m128i m [xmm1], __m128i l [xmm2]) */
gcm_gf_reduce:
/* Registers Used: */
/* xmm0: t0, h     */
/* xmm1: t1, m     */
/* xmm2: t2, l     */
/* xmm3: t3[, t0]  */
/* xmm4: t4[, t1]  */
	vxorpd	%xmm0, %xmm1, %xmm3		/* h [xmm0] ^ m [xmm1] -> t0 [xmm3] */
	vxorpd	%xmm2, %xmm3, %xmm3		/* l [xmm2] ^ t0 [xmm3] -> t0 [xmm3] */
	vpslldq	$8, %xmm3, %xmm4		/* t0 [xmm3] << 64 -> t1 [xmm4] */
	vpsrldq	$8, %xmm3, %xmm3		/* t0 [xmm3] >> 64 -> t0 [xmm3] */
	vxorpd	%xmm2, %xmm4, %xmm1		/* l [xmm2] ^ t1 [xmm4] -> t1 [xmm1] */
	vxorpd	%xmm0, %xmm3, %xmm0		/* h [xmm0] ^ t0 [xmm3] -> t0 [xmm0] */

	vpsrld	$31, %xmm1, %xmm2		/* t1 [xmm1] >> 31 -> t2 [xmm2] */
	vpsrld	$31, %xmm0, %xmm3		/* t0 [xmm0] >> 31 -> t3 [xmm3] */
	vpslld	$1, %xmm1, %xmm1		/* t1 [xmm1] << 1 -> t1 [xmm1] */
	vpslld	$1, %xmm0, %xmm0		/* t0 [xmm0] << 1 -> t0 [xmm0] */

	vpsrldq	$12, %xmm2, %xmm4		/* t2 [xmm2] >> 96 -> t4 [xmm4] */
	vpslldq	$4, %xmm3, %xmm3		/* t3 [xmm3] << 32 -> t3 [xmm3] */
	vpslldq	$4, %xmm2, %xmm2		/* t2 [xmm2] << 32 -> t2 [xmm2] */
	vorpd	%xmm4, %xmm0, %xmm0		/* t4 [xmm4] | t0 [xmm0] -> t0 [xmm0] */
	vorpd	%xmm2, %xmm1, %xmm1		/* t2 [xmm2] | t1 [xmm1] -> t1 [xmm1] */
	vorpd	%xmm3, %xmm0, %xmm0		/* t3 [xmm3] | t0 [xmm0] -> t0 [xmm0] */

	vpslld	$31, %xmm1, %xmm2		/* t1 [xmm1] << 31 -> t2 [xmm2] */
	vpslld	$30, %xmm1, %xmm3		/* t1 [xmm1] << 30 -> t3 [xmm3] */
	vpslld	$25, %xmm1, %xmm4		/* t1 [xmm1] << 25 -> t4 [xmm4] */

	vxorpd	%xmm3, %xmm2, %xmm2		/* t3 [xmm3] ^ t2 [xmm2] -> t2 [xmm2] */
	vxorpd	%xmm4, %xmm2, %xmm2		/* t4 [xmm4] ^ t2 [xmm2] -> t2 [xmm2] */
	vpsrldq	$4, %xmm2, %xmm3		/* t2 [xmm2] >> 32 -> t3 [xmm3] */
	vpslldq	$12, %xmm2, %xmm2		/* t2 [xmm2] << 96 -> t2 [xmm2] */
	vxorpd	%xmm2, %xmm1, %xmm1		/* t2 [xmm2] ^ t1 [xmm1] -> t1 [xmm1] */
	vxorpd	%xmm3, %xmm0, %xmm0		/* t3 [xmm3] ^ t0 [xmm0] -> t0 [xmm0] */

	vpsrld	$1, %xmm1, %xmm4		/* t1 [xmm1] >> 1 -> t4 [xmm4] */
	vpsrld	$2, %xmm1, %xmm3		/* t1 [xmm1] >> 2 -> t3 [xmm3] */
	vpsrld	$7, %xmm1, %xmm2		/* t1 [xmm1] >> 7 -> t2 [xmm2] */
	vxorpd	%xmm4, %xmm3, %xmm3		/* t4 [xmm2] ^ t3 [xmm3] -> t3 [xmm3] */
	vxorpd	%xmm3, %xmm2, %xmm2		/* t3 [xmm3] ^ t2 [xmm2] -> t2 [xmm2] */
	vxorpd	%xmm2, %xmm1, %xmm1		/* t2 [xmm2] ^ t1 [xmm1] -> t1 [xmm1] */
	vxorpd	%xmm1, %xmm0, %xmm0		/* t1 [xmm1] ^ t0 [xmm0] -> t0 [xmm0] */
	ret		/* return t0 [xmm0] */


	.align	16
	.globl	_crypto_gcm_init_hash_pclmul
	.def	_crypto_gcm_init_hash_pclmul;	.scl	2;	.type	32;	.endef
	/* void _crypto_gcm_init_hash_pclmul(struct ghash_pclmul *dst [%rdi], */
	/*                                   const union crypto_block16 *src [%rsi]) */
_crypto_gcm_init_hash_pclmul:
	subq	$8, %rsp		/* align stack */
	vxorpd	%xmm0, %xmm0, %xmm0			/* 0 -> xmm0 */
	vmovdqa	(%rsi), %xmm6				/* *src -> xmm6 */
	vmovdqa	%xmm0, 0x00(%rdi)			/* dst->d = 0 */
	/* Compute h1 */
	vpshufb	bswap(%rip), %xmm6, %xmm6	/* bswapdq(*src) -> h1 [xmm6] */
	vmovdqa	%xmm6, 0x10(%rdi)			/* dst->h1 = bswapdq(*src) [xmm6] */
	vpshufd	$0x4E, %xmm6, %xmm7			/* ROT(h1 [xmm4], 64) -> h1m [xmm7] */
	vxorpd	%xmm6, %xmm7, %xmm7			/* h1 [xmm6] ^ ROT(h1, 64) [xmm7] -> h1m [xmm7] */

	vpclmulhqhqdq %xmm6, %xmm6, %xmm0	/* pclmul_hh(h1 [xmm6], h1 [xmm6]) -> high [xmm0] */
	vpclmullqlqdq %xmm7, %xmm7, %xmm1	/* pclmul_ll(h1m [xmm7], h1m [xmm7]) -> mid [xmm1] */
	vpclmullqlqdq %xmm6, %xmm6, %xmm2	/* pclmul_ll(h1 [xmm6], h1 [xmm6]) -> low [xmm2] */
	call	gcm_gf_reduce	/* gf_reduce(high, mid, low) */
	vmovdqa	%xmm0, 0x20(%rdi)			/* dst->h2 = gf_reduce(high, mid, low) [xmm0] */
	vmovdqa	%xmm0, %xmm3				/* h2 -> xmm3 */
	vpshufd	$0x4E, %xmm0, %xmm1			/* ROT(h2 [xmm4/xmm0], 64) -> h2m [xmm1] */
	vxorpd	%xmm0, %xmm1, %xmm1			/* h2 [xmm4/xmm0] ^ ROT(h2, 64) [xmm1] -> h2m [xmm1] */
	vshufpd	$0x01, %xmm1, %xmm7, %xmm7	/* (h1m [xmm7], h2m [xmm1]) -> h1_h2_m [xmm7] */
	vmovdqa	%xmm7, 0x50(%rdi)			/* dst->h1_h2_m = (h1m, h2m) [xmm7] */

	vpclmulhqhqdq %xmm6, %xmm4, %xmm0	/* pclmul_hh(h1 [xmm6], h2 [xmm3]) -> high [xmm0] */
	vpclmullqlqdq %xmm7, %xmm1, %xmm1	/* pclmul_ll(h1m [xmm7], h2m [xmm1]) -> mid [xmm1] */
	vpclmullqlqdq %xmm6, %xmm4, %xmm2	/* pclmul_ll(h1 [xmm6], h2 [xmm3]) -> low [xmm2] */
	call	gcm_gf_reduce	/* gf_reduce(high, mid, low) */
	vmovdqa	%xmm0, 0x30(%rdi)			/* dst->h3 = gf_reduce(high, mid, low) [xmm0] */
	vmovdqa	%xmm0, %xmm3				/* h3 -> xmm3 */
	vpshufd	$0x4E, %xmm0, %xmm5			/* ROT(h3 [xmm3/xmm0], 64) -> h3m [xmm5] */
	vxorpd	%xmm0, %xmm5, %xmm5			/* h3 [xmm3/xmm0] ^ ROT(h3, 64) [xmm5] -> h3m [xmm5] */

	vpclmulhqhqdq %xmm6, %xmm3, %xmm0	/* pclmul_hh(h1 [xmm6], h3 [xmm3]) -> high [xmm0] */
	vpclmullqlqdq %xmm7, %xmm5, %xmm1	/* pclmul_ll(h1m [xmm7], h3m [xmm5]) -> mid [xmm1] */
	vpclmullqlqdq %xmm6, %xmm3, %xmm2	/* pclmul_ll(h1 [xmm6], h3 [xmm3]) -> low [xmm2] */
	call	gcm_gf_reduce	/* gf_reduce(high, mid, low) */
	vmovdqa	%xmm0, 0x40(%rdi)			/* dst->h4 = gf_reduce(high, mid, low) [xmm0] */
	vpshufd	$0x4E, %xmm0, %xmm1			/* ROT(h4 [xmm0], 64) -> h4m [xmm1] */
	vxorpd	%xmm0, %xmm1, %xmm1			/* h4 [xmm0] ^ ROT(h4, 64) [xmm1] -> h4m [xmm1] */
	vshufpd	$0x01, %xmm5, %xmm1, %xmm1	/* (h3m [xmm5], h4m [xmm1]) -> h3_h4_m [xmm1] */
	vmovdqa	%xmm1, 0x60(%rdi)			/* dst->h3_h4_m = (h3m, h4m) [xmm1] */
	addq	$8, %rsp		/* restore stack */
	ret


	.align	16
	.globl	_crypto_gcm_hash_pclmul
	.def	_crypto_gcm_hash_pclmul;	.scl	2;	.type	32;	.endef
	/* void _crypto_gcm_hash_pclmul(struct ghash_pclmul *hash [%rdi], */
	/*                              const uint8_t *src [%rsi], */
	/*                              size_t length [%rdx]) */
_crypto_gcm_hash_pclmul:
	subq	$0x88, %rsp		/* allocate 16-byte aligned space for 8x 16-byte registers */
	/* clobber xmm8-xmm15 */
	vmovdqa	%xmm15, 0x00(%rsp)
	vmovdqa	%xmm14, 0x10(%rsp)
	vmovdqa	%xmm13, 0x20(%rsp)
	vmovdqa	%xmm12, 0x30(%rsp)
	vmovdqa	%xmm11, 0x40(%rsp)
	vmovdqa	%xmm10, 0x50(%rsp)
	vmovdqa	%xmm9,  0x60(%rsp)
	vmovdqa	%xmm8,  0x70(%rsp)
	cmpq	$64, %rdx
	vmovdqa	0x00(%rdi), %xmm0	/* dst->d -> xmm0 */
	vmovdqa	0x10(%rdi), %xmm10	/* dst->h1 -> h1 [xmm10] */
	vmovdqa	0x50(%rdi), %xmm14	/* dst->h1_h2_m -> h1_h2_m [xmm14] */
	jb		.Lhash_single
	movq	%rdx, %rax		/* length -> rax */
	andq	$-64, %rax		/* (length - (length % 64)) -> rax */
	subq	%rax, %rdx		/* (length % 64) -> length */
	vmovdqa	0x20(%rdi), %xmm11	/* dst->h2 -> h2 [xmm11] */
	vmovdqa	0x30(%rdi), %xmm12	/* dst->h3 -> h3 [xmm12] */
	vmovdqa	0x40(%rdi), %xmm13	/* dst->h4 -> h4 [xmm13] */
	vmovdqa	0x60(%rdi), %xmm15	/* dst->h3_h4_m -> h3_h4_m [xmm15] */
	.align	16
.Lhash4:
	/* Register Usage:                 */
	/* xmm0:  th, d     xmm1:  tm      */
	/* xmm2:  tl        xmm3:  t2      */
	/* xmm4:  t3        xmm5:  t4      */
	/* xmm6:  x1        xmm7:  x2      */
	/* xmm8:  x3        xmm9:  x4      */
	/* xmm10: h1        xmm11: h2      */
	/* xmm12: h3        xmm13: h4      */
	/* xmm14: h1m/h2m   xmm15: h3m/h4m */
	vmovdqa	bswap(%rip), %xmm5		/* BSWAP -> xmm5 */
	vmovdqu	0x00(%rsi), %xmm6		/* src[15:0 ] -> x1 [xmm6] */
	vmovdqu	0x10(%rsi), %xmm7		/* src[31:16] -> x2 [xmm7] */
	vmovdqu	0x20(%rsi), %xmm8		/* src[47:32] -> x3 [xmm8] */
	vmovdqu	0x30(%rsi), %xmm9		/* src[63:48] -> x4 [xmm9] */
	vpshufb	%xmm5, %xmm6, %xmm6		/* bswapdq(x1) -> x1 [xmm6] */
	vpshufb	%xmm5, %xmm7, %xmm7		/* bswapdq(x2) -> x2 [xmm7] */
	vpshufb	%xmm5, %xmm8, %xmm8		/* bswapdq(x3) -> x3 [xmm8] */
	vpshufb	%xmm5, %xmm9, %xmm9		/* bswapdq(x4) -> x4 [xmm9] */
	vxorpd	%xmm0, %xmm6, %xmm6		/* d [xmm0] ^ x1 [xmm6] -> x1 [xmm6] */
	/* Low Product */
	vpclmullqlqdq %xmm10, %xmm9, %xmm5	/* pclmul_ll(h1 [xmm10], x4 [xmm9]) -> t4 [xmm5] */
	vpclmullqlqdq %xmm11, %xmm8, %xmm4	/* pclmul_ll(h2 [xmm11], x3 [xmm8]) -> t3 [xmm4] */
	vpclmullqlqdq %xmm12, %xmm7, %xmm3	/* pclmul_ll(h3 [xmm12], x2 [xmm7]) -> t2 [xmm3] */
	vpclmullqlqdq %xmm13, %xmm6, %xmm2	/* pclmul_ll(h4 [xmm13], x1 [xmm6]) -> tl [xmm2] */
	vxorpd	%xmm5, %xmm4, %xmm4		/* t4 [xmm5] ^ t3 [xmm4] -> t3 [xmm4] */
	vxorpd	%xmm4, %xmm3, %xmm3		/* t3 [xmm4] ^ t2 [xmm3] -> t2 [xmm3] */
	vxorpd	%xmm3, %xmm2, %xmm2		/* t2 [xmm3] ^ tl [xmm2] -> tl [xmm2] */
	/* High Product */
	vpclmulhqhqdq %xmm10, %xmm9, %xmm5	/* pclmul_hh(h1 [xmm10], x4 [xmm9]) -> t4 [xmm5] */
	vpclmulhqhqdq %xmm11, %xmm8, %xmm4	/* pclmul_hh(h2 [xmm11], x3 [xmm8]) -> t3 [xmm4] */
	vpclmulhqhqdq %xmm12, %xmm7, %xmm3	/* pclmul_hh(h3 [xmm12], x2 [xmm7]) -> t2 [xmm3] */
	vpclmulhqhqdq %xmm13, %xmm6, %xmm0	/* pclmul_hh(h4 [xmm13], x1 [xmm6]) -> th [xmm0] */
	vxorpd	%xmm5, %xmm4, %xmm4		/* t4 [xmm5] ^ t3 [xmm4] -> t3 [xmm4] */
	vxorpd	%xmm4, %xmm3, %xmm3		/* t3 [xmm4] ^ t2 [xmm3] -> t2 [xmm3] */
	vxorpd	%xmm3, %xmm0, %xmm0		/* t2 [xmm3] ^ th [xmm0] -> th [xmm0] */
	/* Compute middle multiplicands */
	vpshufd	$0x4E, %xmm9, %xmm5		/* ROT(x4 [xmm9], 64) -> t4 [xmm5] */
	vpshufd	$0x4E, %xmm8, %xmm4		/* ROT(x3 [xmm8], 64) -> t3 [xmm4] */
	vpshufd	$0x4E, %xmm7, %xmm3		/* ROT(x2 [xmm7], 64) -> t2 [xmm3] */
	vpshufd	$0x4E, %xmm6, %xmm1		/* ROT(x1 [xmm6], 64) -> tm [xmm1] */
	vxorpd	%xmm9, %xmm5, %xmm5		/* x4 [xmm9] ^ ROT(x4, 64) [xmm5] -> t4 [xmm5] */
	vxorpd	%xmm8, %xmm4, %xmm4		/* x3 [xmm8] ^ ROT(x3, 64) [xmm4] -> t3 [xmm4] */
	vxorpd	%xmm7, %xmm3, %xmm3		/* x2 [xmm7] ^ ROT(x2, 64) [xmm3] -> t2 [xmm3] */
	vxorpd	%xmm6, %xmm1, %xmm1		/* x1 [xmm6] ^ ROT(x1, 64) [xmm1] -> tm [xmm1] */
	/* Middle Product */
	vpclmulhqhqdq %xmm15, %xmm5, %xmm5	/* pclmul_hh(h4m [xmm15], t4 [xmm5]) -> t4 [xmm5] */
	vpclmullqlqdq %xmm15, %xmm4, %xmm4	/* pclmul_ll(h3m [xmm15], t3 [xmm4]) -> t3 [xmm4] */
	vpclmulhqhqdq %xmm14, %xmm3, %xmm3	/* pclmul_hh(h2m [xmm14], t2 [xmm3]) -> t2 [xmm3] */
	vpclmullqlqdq %xmm14, %xmm1, %xmm1	/* pclmul_ll(h1m [xmm14], tm [xmm1]) -> tm [xmm1] */
	vxorpd	%xmm5, %xmm4, %xmm4		/* t4 [xmm5] ^ t3 [xmm4] -> t3 [xmm4] */
	vxorpd	%xmm4, %xmm3, %xmm3		/* t3 [xmm4] ^ t2 [xmm3] -> t2 [xmm3] */
	vxorpd	%xmm3, %xmm1, %xmm1		/* t2 [xmm3] ^ tm [xmm1] -> tm [xmm1] */
	call	gcm_gf_reduce	/* gf_reduce(th, tm, tl) -> d [xmm0] */
	addq	$64, %rsi	/* offset input pointer */
	subq	$64, %rax	/* decrement loop counter */
	jnz		.Lhash4		/* jump to start of loop if counter is non-zero */

	.align	16
.Lhash_single:
	cmpq	$16, %rdx
	jb		.Lhash_part		/* less than 16 bytes remaining, so jump to partial block hash */
	movq	%rdx, %rax		/* length -> rax */
	andq	$-16, %rax		/* (length - (length % 16)) -> rax */
	subq	%rax, %rdx		/* (length % 16) -> length */
1:	vmovdqu	(%rsi), %xmm6				/* src[15:0 ] -> x [xmm6] */
	vpshufb	bswap(%rip), %xmm6, %xmm6	/* bswapdq(x) -> x [xmm6] */
	vxorpd	%xmm0, %xmm6, %xmm6			/* d [xmm0] ^ x [xmm6] -> x [xmm6] */
	vpshufd	$0x4E, %xmm6, %xmm1			/* ROT(x [xmm6], 64) -> tm [xmm1] */
	vxorpd	%xmm6, %xmm1, %xmm1			/* x [xmm6] ^ ROT(x, 64) [xmm1] -> tm [xmm1] */
	vpclmulhqhqdq %xmm10, %xmm6, %xmm0	/* pclmul_hh(h1 [xmm10], x [xmm6]) -> th [xmm0] */
	vpclmullqlqdq %xmm14, %xmm1, %xmm1	/* pclmul_ll(h1m [xmm14], tm [xmm1]) -> tm [xmm1] */
	vpclmullqlqdq %xmm10, %xmm6, %xmm2	/* pclmul_ll(h1 [xmm10], x [xmm6]) -> tl [xmm2] */
	call	gcm_gf_reduce	/* gf_reduce(th, tm, tl) -> d [xmm0] */
	addq	$16, %rsi	/* offset input pointer */
	subq	$16, %rax	/* decrement loop counter */
	jnz		1b		/* jump to start of loop if counter is non-zero */

.Lhash_part:
	testb	%dl, %dl
	jz		.Lhash_end	/* no bytes remaining, jump to return */
	xorl	%ecx, %ecx			/* 0 -> i [rcx] */
	subq	$16, %rsp			/* allocate space for a temporary 16-byte vector, tmp */
	movq	%rcx, (%rsp)		/* 0 -> tmp[ 7:0] */
	movq	%rcx, 8(%rsp)		/* 0 -> tmp[15:8] */
	testb	$8, %dl				/* check for 8 bytes to copy */
	jz		1f			/* do not have 8 bytes, skip to 4 byte copy */
	movq	(%rsi), %rax		/* src[7:0] -> rax */
	movq	%rax, (%rsp)		/* src[7:0] -> tmp[7:0] */
	leal	8(%rcx), %ecx		/* 8 -> i [rcx] */
1:	testb	$4, %dl				/* check for 4 bytes to copy */
	jz		1f			/* do not have 4 bytes, skip to 2 byte copy */
	movl	(%rsi,%rcx), %eax	/* src[i+3:i] -> rax */
	movl	%eax, (%rsp,%rcx)	/* src[i+3:i] -> tmp[i+3:i] */
	leal	4(%rcx), %ecx		/* i + 4 -> i [rcx] */
1:	testb	$2, %dl				/* check for 2 bytes to copy */
	jz		1f			/* do not have 2 bytes, skip to 1 byte copy */
	movw	(%rsi,%rcx), %ax	/* src[i+1:i] -> rax */
	movw	%ax, (%rsp,%rcx)	/* src[i+1:i] -> tmp[rcx+1:rcx] */
	leal	2(%rcx), %ecx		/* i + 2 -> i [rcx] */
1:	testb	$1, %dl				/* check for 1 bytes to copy */
	jz		1f		/* do not have 1 byte, skip to partial block hash processing */
	movb	(%rsi,%rcx), %al	/* src[i] -> rax */
	movb	%al, (%rsp,%rcx)	/* src[i] -> tmp[i] */
1:	vmovdqa	(%rsp), %xmm6		/* tmp[15:0] -> x [xmm6] */
	vpshufb	bswap(%rip), %xmm6, %xmm6	/* bswapdq(x) -> x [xmm6] */
	vxorpd	%xmm0, %xmm6, %xmm6			/* d [xmm0] ^ x [xmm6] -> x [xmm6] */
	vpshufd	$0x4E, %xmm6, %xmm1			/* ROT(x [xmm6], 64) -> tm [xmm1] */
	vxorpd	%xmm6, %xmm1, %xmm1			/* x [xmm6] ^ ROT(x, 64) [xmm1] -> tm [xmm1] */
	vpclmulhqhqdq %xmm10, %xmm6, %xmm0	/* pclmul_hh(h1 [xmm10], x [xmm6]) -> th [xmm0] */
	vpclmullqlqdq %xmm14, %xmm1, %xmm1	/* pclmul_ll(h1m [xmm14], tm [xmm1]) -> tm [xmm1] */
	vpclmullqlqdq %xmm10, %xmm6, %xmm2	/* pclmul_ll(h1 [xmm10], x [xmm6]) -> tl [xmm2] */
	call	gcm_gf_reduce	/* gf_reduce(th, tm, tl) */

	addq	$16, %rsp	/* restore space allocated on stack */
.Lhash_end:
	vmovdqa	%xmm0, (%rdi)	/* dst->d = d [xmm0] */
	/* restore clobbered xmm registers */
	vmovdqa	0x70(%rsp), %xmm8
	vmovdqa	0x60(%rsp), %xmm9
	vmovdqa	0x50(%rsp), %xmm10
	vmovdqa	0x40(%rsp), %xmm11
	vmovdqa	0x30(%rsp), %xmm12
	vmovdqa	0x20(%rsp), %xmm13
	vmovdqa	0x10(%rsp), %xmm14
	vmovdqa	0x00(%rsp), %xmm15
	addq	$0x88, %rsp		/* free temporary memory on stack */
	ret
