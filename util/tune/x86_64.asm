	.file	"x86_64.asm"
	.text
	.globl	speed_cyclecounter
	.def	speed_cyclecounter;	.scl	2;	.type	32;	.endef
	.align	16
speed_cyclecounter:
	movq	%rbx, %r10
	xorl	%eax, %eax
	cpuid
	rdtsc
	movl	%eax, (%rdi)
	movl	%edx, 4(%rdi)
	movq	%r10, %rbx
	ret
