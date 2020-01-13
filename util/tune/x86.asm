	.file	"x86.asm"
	.text
	.globl	_speed_cyclecounter
	.def	_speed_cyclecounter;	.scl	2;	.type	32;	.endef
	.align	8
_speed_cyclecounter:
	pushl	%ebx
	xorl	%eax, %eax
	cpuid
	rdtsc
	movl	8(%esp), %ebx
	movl	%eax, (%ebx)
	movl	%edx, 4(%ebx)
	popl	%ebx
	ret
