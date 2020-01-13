	.file	"serpent-avx.asm"

	.include "serpent-internal.asm"

	.section	.rdata, "dr"
	.align	8
LT64_mask_lsh13:
	.quad	0xFFFFE000FFFFFFFF
LT64_mask_rsh19:
	.quad	0xFFFFFFFF00001FFF

LT64_mask_lsh3:
	.quad	0xFFFFFFF8FFFFFFFF
LT64_mask_rsh29:
	.quad	0xFFFFFFFF00000007

LT64_mask_lsh1:
	.quad	0xFFFFFFFEFFFFFFFF
LT64_mask_rsh31:
	.quad	0xFFFFFFFF00000001

LT64_mask_lsh7:
	.quad	0xFFFFFF80FFFFFFFF
LT64_mask_rsh25:
	.quad	0xFFFFFFFF0000007F

LT64_mask_lsh5:
	.quad	0xFFFFFFE0FFFFFFFF
LT64_mask_rsh27:
	.quad	0xFFFFFFFF0000001F

LT64_mask_lsh22:
	.quad	0xFFC00000FFFFFFFF
LT64_mask_rsh10:
	.quad	0xFFFFFFFF003FFFFF

	.text
	.align	16
	.globl	_crypto_serpent_encrypt_avx
	.def	_crypto_serpent_encrypt_avx;	.scl	2;	.type	32;	.endef
	/* void _serpent_encrypt_avx(const uint32_t(*subkeys)[4] [%rdi], */
	/*                           uint8_t* dst [%rsi], */
	/*                           const uint8_t* src [%rdx], */
	/*                           size_t length [%rcx]) */
_crypto_serpent_encrypt_avx:
	/* If there is exactly 64 bytes of input (4 sets of 16 bytes), delegate to special case */
	cmpq	$64, %rcx
	je		.Lavx_encrypt4
	/* If there is exactly 128 bytes of input (8 sets of 16 bytes), delegate to special case */
	/* If there is less than 128 bytes of input, delegate to x64 code */
	cmpq	$128, %rcx
	ja		1f
	je		.Lavx_encrypt8
	subq	$8, %rsp
	call	_crypto_serpent_encrypt_x64
	addq	$8, %rsp
	ret
	/* Clobber registers */
1:	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$0x50, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
	/* Save stack pointer */
	movq	%rsp, %r11
	/* Divide length argument by 16 */
	shrq	$4, %rcx
	/* Move dst pointer */
	movq	%rsi, %r14
	/* Move src pointer */
	movq	%rdx, %r13
	/* Move length argument */
	movq	%rcx, %r12
	/* Move key pointer */
	movq	%rdi, %r10
	/* Divide length by 2 */
	shrq	$1, %r12
	/* Check whether length was odd before division */
	jnc		.Lencrypt2	/* Skip to 2-block handling if length was not odd before division */
	/* Encrypt a single block outside of the loop */
	/* Load state */
	movl	0x0(%r13), %eax
	movl	0x4(%r13), %ebx
	movl	0x8(%r13), %ecx
	movl	0xC(%r13), %edx
	/* Perform Encryption */
	ENC0	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x000
	ENC1	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x010
	ENC2	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x020
	ENC3	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x030
	ENC4	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x040
	ENC5	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x050
	ENC6	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x060
	ENC7	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x070
	ENC0	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x080
	ENC1	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x090
	ENC2	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x0A0
	ENC3	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x0B0
	ENC4	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x0C0
	ENC5	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x0D0
	ENC6	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x0E0
	ENC7	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x0F0
	ENC0	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x100
	ENC1	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x110
	ENC2	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x120
	ENC3	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x130
	ENC4	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x140
	ENC5	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x150
	ENC6	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x160
	ENC7	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x170
	ENC0	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x180
	ENC1	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x190
	ENC2	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x1A0
	ENC3	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x1B0
	ENC4	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x1C0
	ENC5	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r10, 0x1D0
	ENC6	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r10, 0x1E0
	ADDKEY	%edi,%esi,%ebp,%esp, %r10, 0x1F0
	SBOX7	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx
	ADDKEY	%eax,%ebx,%ecx,%edx, %r10, 0x200
	/* Write Output */
	movl	%eax, 0x0(%r14)
	movl	%ebx, 0x4(%r14)
	movl	%ecx, 0x8(%r14)
	movl	%edx, 0xC(%r14)
	addq	$16, %r13		/* Offset src pointer */
	addq	$16, %r14		/* Offset dst pointer */
	.align	16
.Lencrypt2:
	/* Divide length by 2 */
	shrq	$1, %r12
	jnc		.Lencrypt_loop	/* skip to loop entry if length was not odd before division */
	/* Load most commonly used rotate and shift masks */
	movq	LT64_mask_lsh3(%rip), %r8
	movq	LT64_mask_lsh7(%rip), %r9
	/* Encrypt a pair of blocks outside of the loop */
	/* Load state 1 and 2 */
	movl	0x00(%r13), %eax
	movl	0x04(%r13), %ebx
	movl	0x08(%r13), %ecx
	movl	0x0C(%r13), %edx
	movl	0x10(%r13), %edi
	movl	0x14(%r13), %esi
	movl	0x18(%r13), %ebp
	movl	0x1C(%r13), %esp
	shlq	$32, %rdi
	shlq	$32, %rsi
	shlq	$32, %rbp
	shlq	$32, %rsp
	orq		%rdi, %rax
	orq		%rsi, %rbx
	orq		%rbp, %rcx
	orq		%rsp, %rdx
	/* Perform Encryption */
	ENC0	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x000, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC1	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x010, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC2	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x020, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC3	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x030, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC4	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x040, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC5	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x050, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC6	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x060, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC7	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x070, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC0	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x080, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC1	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x090, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC2	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x0A0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC3	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x0B0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC4	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x0C0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC5	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x0D0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC6	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x0E0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC7	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x0F0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC0	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x100, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC1	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x110, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC2	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x120, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC3	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x130, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC4	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x140, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC5	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x150, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC6	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x160, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC7	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x170, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC0	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x180, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC1	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x190, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC2	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x1A0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC3	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x1B0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC4	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x1C0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ENC5	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r10, 0x1D0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	ENC6	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r10, 0x1E0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	ADDKEY	%rdi,%rsi,%rbp,%rsp, %r10, 0x1F0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	SBOX7	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, 64
	ADDKEY	%rax,%rbx,%rcx,%rdx, %r10, 0x200, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	/* Write output */
	movl	%eax, 0x00(%r14)
	movl	%ebx, 0x04(%r14)
	movl	%ecx, 0x08(%r14)
	movl	%edx, 0x0C(%r14)
	shrq	$32, %rax
	shrq	$32, %rbx
	shrq	$32, %rcx
	shrq	$32, %rdx
	movl	%eax, 0x10(%r14)
	movl	%ebx, 0x14(%r14)
	movl	%ecx, 0x18(%r14)
	movl	%edx, 0x1C(%r14)
	addq	$32, %r13		/* Offset src pointer */
	addq	$32, %r14		/* Offset dst pointer */
	.align	16
.Lencrypt_loop:
	/* Load states */
	vmovups	0x00(%r13), %xmm0
	vmovups	0x10(%r13), %xmm1
	vmovups	0x20(%r13), %xmm2
	vmovups	0x30(%r13), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Encryption */
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x000, %r10, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x010, %r10, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x020, %r10, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x030, %r10, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x040, %r10, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x050, %r10, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x060, %r10, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x070, %r10, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x080, %r10, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x090, %r10, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0A0, %r10, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0B0, %r10, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0C0, %r10, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0D0, %r10, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0E0, %r10, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0F0, %r10, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x100, %r10, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x110, %r10, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x120, %r10, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x130, %r10, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x140, %r10, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x150, %r10, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x160, %r10, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x170, %r10, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x180, %r10, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x190, %r10, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1A0, %r10, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1B0, %r10, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1C0, %r10, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1D0, %r10, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1E0, %r10, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %r10, 128
	SBOX7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 128
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %r10, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, 0x00(%r14)
	vmovups	%xmm1, 0x10(%r14)
	vmovups	%xmm2, 0x20(%r14)
	vmovups	%xmm3, 0x30(%r14)
	addq	$64, %r13		/* Offset src pointer */
	addq	$64, %r14		/* Offset dst pointer */
	decq	%r12			/* Decrement loop counter */
	jnz		.Lencrypt_loop
	movq	%r11, %rsp	/* Restore stack pointer */
	/* Restore clobbered registers and return */
	vmovapd	0x00(%rsp), %xmm8
	vmovapd	0x10(%rsp), %xmm9
	vmovapd	0x20(%rsp), %xmm10
	vmovapd	0x30(%rsp), %xmm11
	vmovapd	0x40(%rsp), %xmm12
	addq	$0x50, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	popq	%r13
	popq	%r14
	ret



	.align	16
.Lavx_encrypt8:
	/* Clobber registers */
	subq	$0x58, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
	/* Offset src and dst pointers */
	addq	$64, %rdx
	addq	$64, %rsi
	/* Load states */
	vmovups	-0x40(%rdx), %xmm0
	vmovups	-0x30(%rdx), %xmm1
	vmovups	-0x20(%rdx), %xmm2
	vmovups	-0x10(%rdx), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Encryption */
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x000, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x010, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x020, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x030, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x040, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x050, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x060, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x070, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x080, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x090, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0A0, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0B0, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0C0, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0D0, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0E0, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0F0, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x100, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x110, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x120, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x130, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x140, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x150, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x160, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x170, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x180, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x190, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1A0, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1B0, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1C0, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1D0, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1E0, %rdi, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %rdi, 128
	SBOX7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 128
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %rdi, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, -0x40(%rsi)
	vmovups	%xmm1, -0x30(%rsi)
	vmovups	%xmm2, -0x20(%rsi)
	vmovups	%xmm3, -0x10(%rsi)
	/* Jump to start of 4-block case */
	jmp		.Lavx_encrypt4_start


	.align	16
.Lavx_encrypt4:
	/* Clobber registers */
	subq	$0x58, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
.Lavx_encrypt4_start:
	/* Load states */
	vmovups	0x00(%rdx), %xmm0
	vmovups	0x10(%rdx), %xmm1
	vmovups	0x20(%rdx), %xmm2
	vmovups	0x30(%rdx), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Encryption */
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x000, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x010, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x020, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x030, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x040, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x050, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x060, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x070, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x080, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x090, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0A0, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0B0, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0C0, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0D0, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0E0, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0F0, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x100, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x110, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x120, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x130, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x140, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x150, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x160, %rdi, 128
	ENC7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x170, %rdi, 128
	ENC0	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x180, %rdi, 128
	ENC1	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x190, %rdi, 128
	ENC2	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1A0, %rdi, 128
	ENC3	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1B0, %rdi, 128
	ENC4	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1C0, %rdi, 128
	ENC5	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1D0, %rdi, 128
	ENC6	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1E0, %rdi, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %rdi, 128
	SBOX7	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 128
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %rdi, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, 0x00(%rsi)
	vmovups	%xmm1, 0x10(%rsi)
	vmovups	%xmm2, 0x20(%rsi)
	vmovups	%xmm3, 0x30(%rsi)
	/* Restore clobbered registers and return */
	vmovapd	0x00(%rsp), %xmm8
	vmovapd	0x10(%rsp), %xmm9
	vmovapd	0x20(%rsp), %xmm10
	vmovapd	0x30(%rsp), %xmm11
	vmovapd	0x40(%rsp), %xmm12
	addq	$0x58, %rsp
	ret




	.align	16
	.globl	_crypto_serpent_decrypt_avx
	.def	_crypto_serpent_decrypt_avx;	.scl	2;	.type	32;	.endef
	/* void _serpent_decrypt_avx(const uint32_t(*subkeys)[4] [%rdi], */
	/*                           uint8_t* dst [%rsi], */
	/*                           const uint8_t* src [%rdx], */
	/*                           size_t length [%rcx]) */
_crypto_serpent_decrypt_avx:
	/* If there is exactly 64 bytes of input (4 sets of 16 bytes), delegate to special case */
	cmpq	$64, %rcx
	je		.Lavx_decrypt4
	/* If there is exactly 128 bytes of input (8 sets of 16 bytes), delegate to special case */
	/* If there is less than 128 bytes of input, delegate to x64 code */
	cmpq	$128, %rcx
	ja		1f
	je		.Lavx_decrypt8
	subq	$8, %rsp
	call	_crypto_serpent_decrypt_x64
	addq	$8, %rsp
	ret
	/* Clobber registers */
1:	pushq	%r12
	pushq	%rbp
	pushq	%rbx
	subq	$0x50, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
	/* Save stack pointer */
	movq	%rsp, %r12
	/* Divide length argument by 16 */
	shrq	$4, %rcx
	/* Move dst pointer */
	movq	%rsi, %r11
	/* Move src pointer */
	movq	%rdx, %r10
	/* Move length argument */
	movq	%rcx, %r9
	/* Move key pointer */
	movq	%rdi, %r8
	/* Divide length by 2 */
	shrq	$1, %r9
	/* Check whether length was odd before division */
	jnc		.Ldecrypt2	/* Skip to 2-block handling if length was not odd before division */
	/* Decrypt a single block outside of the loop */
	/* Load state */
	movl	0x0(%r10), %eax
	movl	0x4(%r10), %ebx
	movl	0x8(%r10), %ecx
	movl	0xC(%r10), %edx
	/* Perform Decryption */
	ADDKEY	%eax,%ebx,%ecx,%edx, %r8, 0x200
	IBOX7	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp
	ADDKEY	%edi,%esi,%ebp,%esp, %r8, 0x1F0
	DEC6	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x1E0
	DEC5	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x1D0
	DEC4	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x1C0
	DEC3	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x1B0
	DEC2	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x1A0
	DEC1	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x190
	DEC0	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x180
	DEC7	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x170
	DEC6	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x160
	DEC5	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x150
	DEC4	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x140
	DEC3	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x130
	DEC2	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x120
	DEC1	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x110
	DEC0	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x100
	DEC7	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x0F0
	DEC6	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x0E0
	DEC5	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x0D0
	DEC4	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x0C0
	DEC3	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x0B0
	DEC2	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x0A0
	DEC1	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x090
	DEC0	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x080
	DEC7	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x070
	DEC6	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x060
	DEC5	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x050
	DEC4	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x040
	DEC3	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x030
	DEC2	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x020
	DEC1	%eax,%ebx,%ecx,%edx, %edi,%esi,%ebp,%esp, %r8, 0x010
	DEC0	%edi,%esi,%ebp,%esp, %eax,%ebx,%ecx,%edx, %r8, 0x000
	/* Write Output */
	movl	%eax, 0x0(%r11)
	movl	%ebx, 0x4(%r11)
	movl	%ecx, 0x8(%r11)
	movl	%edx, 0xC(%r11)
	addq	$16, %r10		/* Offset src pointer */
	addq	$16, %r11		/* Offset dst pointer */
	.align	16
.Ldecrypt2:
	/* Divide length by 2 */
	shrq	$1, %r12
	jnc		.Ldecrypt_loop	/* skip to loop entry if length was not odd before division */
	/* Decrypt a pair of blocks outside of the loop */
	/* Load state 1 and 2 */
	movl	0x00(%r10), %eax
	movl	0x04(%r10), %ebx
	movl	0x08(%r10), %ecx
	movl	0x0C(%r10), %edx
	movl	0x10(%r10), %edi
	movl	0x14(%r10), %esi
	movl	0x18(%r10), %ebp
	movl	0x1C(%r10), %esp
	shlq	$32, %rdi
	shlq	$32, %rsi
	shlq	$32, %rbp
	shlq	$32, %rsp
	orq		%rdi, %rax
	orq		%rsi, %rbx
	orq		%rbp, %rcx
	orq		%rsp, %rdx
	/* Perform Decryption */
	ADDKEY	%rax,%rbx,%rcx,%rdx, %r8, 0x200, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	IBOX7	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, 64
	ADDKEY	%rdi,%rsi,%rbp,%rsp, %r8, 0x1F0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC6	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x1E0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC5	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x1D0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC4	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x1C0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC3	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x1B0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC2	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x1A0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC1	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x190, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC0	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x180, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC7	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x170, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC6	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x160, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC5	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x150, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC4	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x140, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC3	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x130, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC2	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x120, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC1	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x110, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC0	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x100, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC7	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x0F0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC6	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x0E0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC5	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x0D0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC4	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x0C0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC3	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x0B0, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC2	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x0A0, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC1	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x090, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC0	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x080, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC7	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x070, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC6	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x060, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC5	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x050, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC4	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x040, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC3	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x030, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC2	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x020, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	DEC1	%rax,%rbx,%rcx,%rdx, %rdi,%rsi,%rbp,%rsp, %r8, 0x010, 64, %eax,%ebx,%rax,%rbx,%rcx,%rdx
	DEC0	%rdi,%rsi,%rbp,%rsp, %rax,%rbx,%rcx,%rdx, %r8, 0x000, 64, %edi,%esi,%rdi,%rsi,%rbp,%rsp
	/* Write output */
	movl	%eax, 0x00(%r11)
	movl	%ebx, 0x04(%r11)
	movl	%ecx, 0x08(%r11)
	movl	%edx, 0x0C(%r11)
	shrq	$32, %rax
	shrq	$32, %rbx
	shrq	$32, %rcx
	shrq	$32, %rdx
	movl	%eax, 0x10(%r11)
	movl	%ebx, 0x14(%r11)
	movl	%ecx, 0x18(%r11)
	movl	%edx, 0x1C(%r11)
	addq	$32, %r10		/* Offset src pointer */
	addq	$32, %r11		/* Offset dst pointer */
	.align	16
.Ldecrypt_loop:
	/* Load states */
	vmovups	0x00(%r10), %xmm0
	vmovups	0x10(%r10), %xmm1
	vmovups	0x20(%r10), %xmm2
	vmovups	0x30(%r10), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Decryption */
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %r8, 128
	IBOX7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %r8, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1E0, %r8, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1D0, %r8, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1C0, %r8, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1B0, %r8, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1A0, %r8, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x190, %r8, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x180, %r8, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x170, %r8, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x160, %r8, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x150, %r8, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x140, %r8, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x130, %r8, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x120, %r8, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x110, %r8, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x100, %r8, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0F0, %r8, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0E0, %r8, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0D0, %r8, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0C0, %r8, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0B0, %r8, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0A0, %r8, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x090, %r8, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x080, %r8, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x070, %r8, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x060, %r8, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x050, %r8, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x040, %r8, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x030, %r8, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x020, %r8, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x010, %r8, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x000, %r8, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, 0x00(%r11)
	vmovups	%xmm1, 0x10(%r11)
	vmovups	%xmm2, 0x20(%r11)
	vmovups	%xmm3, 0x30(%r11)
	addq	$64, %r10		/* Offset src pointer */
	addq	$64, %r11		/* Offset dst pointer */
	decq	%r9				/* Decrement loop counter */
	jnz		.Ldecrypt_loop
	movq	%r12, %rsp	/* Restore stack pointer */
	/* Restore clobbered registers and return */
	vmovapd	0x00(%rsp), %xmm8
	vmovapd	0x10(%rsp), %xmm9
	vmovapd	0x20(%rsp), %xmm10
	vmovapd	0x30(%rsp), %xmm11
	vmovapd	0x40(%rsp), %xmm12
	addq	$0x50, %rsp
	popq	%rbx
	popq	%rbp
	popq	%r12
	ret


	.align	16
.Lavx_decrypt8:
	/* Clobber registers */
	subq	$0x50, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
	/* Offset src and dst pointers */
	addq	$64, %rdx
	addq	$64, %rsi
	/* Load states */
	vmovups	-0x40(%rdx), %xmm0
	vmovups	-0x30(%rdx), %xmm1
	vmovups	-0x20(%rdx), %xmm2
	vmovups	-0x10(%rdx), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Decryption */
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %rdi, 128
	IBOX7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1E0, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1D0, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1C0, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1B0, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1A0, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x190, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x180, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x170, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x160, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x150, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x140, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x130, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x120, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x110, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x100, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0F0, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0E0, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0D0, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0C0, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0B0, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0A0, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x090, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x080, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x070, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x060, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x050, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x040, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x030, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x020, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x010, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x000, %rdi, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, -0x40(%rsi)
	vmovups	%xmm1, -0x30(%rsi)
	vmovups	%xmm2, -0x20(%rsi)
	vmovups	%xmm3, -0x10(%rsi)
	/* Jump to start of 4-block case */
	jmp		.Lavx_decrypt4_start


	.align	16
.Lavx_decrypt4:
	/* Clobber registers */
	subq	$0x58, %rsp
	vmovapd	%xmm12, 0x40(%rsp)
	vmovapd	%xmm11, 0x30(%rsp)
	vmovapd	%xmm10, 0x20(%rsp)
	vmovapd	%xmm9, 0x10(%rsp)
	vmovapd	%xmm8, 0x00(%rsp)
	/* Set xmm12 for vnotpd macro */
	vpcmpeqd	%xmm12, %xmm12, %xmm12
.Lavx_decrypt4_start:
	/* Load states */
	vmovups	0x00(%rdx), %xmm0
	vmovups	0x10(%rdx), %xmm1
	vmovups	0x20(%rdx), %xmm2
	vmovups	0x30(%rdx), %xmm3
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Perform Decryption */
	ADDKEY	%xmm0,%xmm1,%xmm2,%xmm3, 0x200, %rdi, 128
	IBOX7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 128
	ADDKEY	%xmm4,%xmm5,%xmm6,%xmm7, 0x1F0, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1E0, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1D0, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1C0, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x1B0, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x1A0, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x190, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x180, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x170, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x160, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x150, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x140, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x130, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x120, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x110, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x100, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0F0, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0E0, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0D0, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0C0, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x0B0, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x0A0, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x090, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x080, %rdi, 128
	DEC7	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x070, %rdi, 128
	DEC6	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x060, %rdi, 128
	DEC5	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x050, %rdi, 128
	DEC4	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x040, %rdi, 128
	DEC3	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x030, %rdi, 128
	DEC2	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x020, %rdi, 128
	DEC1	%xmm0,%xmm1,%xmm2,%xmm3, %xmm4,%xmm5,%xmm6,%xmm7, 0x010, %rdi, 128
	DEC0	%xmm4,%xmm5,%xmm6,%xmm7, %xmm0,%xmm1,%xmm2,%xmm3, 0x000, %rdi, 128
	/* Transpose states */
	TRANSPOSE	%xmm0,%xmm1,%xmm2,%xmm3
	/* Write output */
	vmovups	%xmm0, 0x00(%rsi)
	vmovups	%xmm1, 0x10(%rsi)
	vmovups	%xmm2, 0x20(%rsi)
	vmovups	%xmm3, 0x30(%rsi)
	/* Restore clobbered registers and return */
	vmovapd	0x00(%rsp), %xmm8
	vmovapd	0x10(%rsp), %xmm9
	vmovapd	0x20(%rsp), %xmm10
	vmovapd	0x30(%rsp), %xmm11
	vmovapd	0x40(%rsp), %xmm12
	addq	$0x58, %rsp
	ret

