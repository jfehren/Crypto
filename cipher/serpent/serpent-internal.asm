
	.macro	vnotpd	x, y
	vxorpd	%xmm12, \x, \y
	.endm

	/* Transpose a 4x4 matrix, using 4 xmm registers as rows of 4 32-bit integers */
	.macro	TRANSPOSE	a,b,c,d
	/* Code adapted from the Intel x86 Assembly Intrinsic _MM_TRANSPOSE4_PS.
	   The function of this intrinsic, as well as others, can be found at
	   https:/*software.intel.com/sites/landingpage/IntrinsicsGuide/ */ */
	/* a = |a0 a1 a2 a3|
	   b = |b0 b1 b2 b3|
	   c = |c0 c1 c2 c3|
	   d = |d0 d1 d2 d3| */
	vunpcklps	\b, \a, %xmm8
	vunpcklps	\d, \c, %xmm9
	vunpckhps	\b, \a, %xmm10
	vunpckhps	\d, \c, %xmm11
	vmovlhps	%xmm9,  %xmm8,  \a
	vmovhlps	%xmm8,  %xmm9,  \b
	vmovlhps	%xmm11, %xmm10, \c
	vmovhlps	%xmm10, %xmm11, \d
	/* a = |a0 b0 c0 d0|
	   b = |a1 b1 c1 d1|
	   c = |a2 b2 c2 d2|
	   d = |a3 b3 c3 d3| */
	.endm

	.macro	SBOX0	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x1, \y3
	movl	\x0, \y0
	movl	\x0, \y1
	movl	\x2, \y2
	xorl	\x2, \y3	/* y3 = x1 ^ x2 */
	orl		\x3, \y0	/* y0 = x0 | x3 */
	xorl	\x1, \y1	/* y1 = x0 ^ x1 */
	xorl	\y0, \y3	/* y3 = y3 ^ y0 */
	orl		\y3, \y2	/* y2 = x2 | y3 */
	xorl	\x3, \x0	/* x0 = x0 ^ x3 */
	andl	\x3, \y2	/* y2 = y2 & x3 */
	xorl	\x2, \x3	/* x3 = x3 ^ x2 */
	orl		\x1, \x2	/* x2 = x2 | x1 */
	movl	\y1, \y0
	andl	\x2, \y0	/* y0 = y1 & x2 */
	xorl	\y0, \y2	/* y2 = y2 ^ y0 */
	andl	\y2, \y0	/* y0 = y0 & y2 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	andl	\x0, \x1	/* x1 = x1 & x0 */
	xorl	\x0, \y0	/* y0 = y0 ^ x0 */
	notl	\y0			/* y0 = ~y0 */
	movl	\y0, \y1
	xorl	\x1, \y1	/* y1 = y0 ^ x1 */
	xorl	\x3, \y1	/* y1 = y1 ^ x3 */
	.else
	.if \m==64
	movq	\x1, \y3
	movq	\x0, \y0
	movq	\x0, \y1
	movq	\x2, \y2
	xorq	\x2, \y3	/* y3 = x1 ^ x2 */
	orq		\x3, \y0	/* y0 = x0 | x3 */
	xorq	\x1, \y1	/* y1 = x0 ^ x1 */
	xorq	\y0, \y3	/* y3 = y3 ^ y0 */
	orq		\y3, \y2	/* y2 = x2 | y3 */
	xorq	\x3, \x0	/* x0 = x0 ^ x3 */
	andq	\x3, \y2	/* y2 = y2 & x3 */
	xorq	\x2, \x3	/* x3 = x3 ^ x2 */
	orq		\x1, \x2	/* x2 = x2 | x1 */
	movq	\y1, \y0
	andq	\x2, \y0	/* y0 = y1 & x2 */
	xorq	\y0, \y2	/* y2 = y2 ^ y0 */
	andq	\y2, \y0	/* y0 = y0 & y2 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	andq	\x0, \x1	/* x1 = x1 & x0 */
	xorq	\x0, \y0	/* y0 = y0 ^ x0 */
	notq	\y0			/* y0 = ~y0 */
	movq	\y0, \y1
	xorq	\x1, \y1	/* y1 = y0 ^ x1 */
	xorq	\x3, \y1	/* y1 = y1 ^ x3 */
	.else
	.if \m==128
	vxorpd	\x2, \x1, \y3	/* y3 = x1 ^ x2 */
	vorpd	\x3, \x0, \y0	/* y0 = x0 | x3 */
	vxorpd	\x1, \x0, \y1	/* y1 = x0 ^ x1 */
	vxorpd	\y0, \y3, \y3	/* y3 = y3 ^ y0 */
	vorpd	\y3, \x2, \y2	/* y2 = x2 | y3 */
	vxorpd	\x3, \x0, \x0	/* x0 = x0 ^ x3 */
	vandpd	\x3, \y2, \y2	/* y2 = y2 & x3 */
	vxorpd	\x2, \x3, \x3	/* x3 = x3 ^ x2 */
	vorpd	\x1, \x2, \x2	/* x2 = x2 | x1 */
	vandpd	\x2, \y1, \y0	/* y0 = y1 & x2 */
	vxorpd	\y0, \y2, \y2	/* y2 = y2 ^ y0 */
	vandpd	\y2, \y0, \y0	/* y0 = y0 & y2 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vandpd	\x0, \x1, \x1	/* x1 = x1 & x0 */
	vxorpd	\x0, \y0, \y0	/* y0 = y0 ^ x0 */
	vnotpd	\y0, \y0		/* y0 = ~ y0 */
	vxorpd	\x3, \y1, \y1	/* y1 = y1 ^ x3 */
	vxorpd	\x1, \y0, \y1	/* y1 = y0 ^ x1 */
	.else
	.error	"Invalid mode for SBOX0"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX0	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x2, \y1
	movl	\x0, \y2
	movl	\x0, \y0
	xorl	\x3, \y1	/* y1 = x2 ^ x3 */
	orl		\x1, \y2	/* y2 = x0 | x1 */
	xorl	\x2, \y0	/* y0 = x0 ^ x2 */
	xorl	\y1, \y2	/* y2 = y2 ^ y1 */
	andl	\x2, \y1	/* y1 = y1 & x2 */
	orl		\x1, \x2	/* x2 = x2 | x1 */
	xorl	\x3, \x1	/* x1 = x1 ^ x3 */
	orl		\x0, \y1	/* y1 = y1 | x0 */
	andl	\x2, \x1	/* x1 = x1 & x2 */
	orl		\y2, \x0	/* x0 = x0 | y2 */
	xorl	\x1, \y1	/* y1 = y1 ^ x1 */
	movl	\y2, \x1
	xorl	\y1, \x0	/* x0 = x0 ^ y1 */
	notl	\y2			/* y2 = ~ y2 */
	andl	\x0, \x1	/* x1 = x0 &~y2 */
	orl		\y2, \x3	/* x3 = x3 | y2 */
	xorl	\x2, \x3	/* x3 = x3 ^ x2 */
	movl	\x0, \y3
	orl		\x3, \x1	/* x1 = x1 | x3 */
	xorl	\x3, \y3	/* y3 = x0 ^ x3 */
	xorl	\x1, \y0	/* y0 = y0 ^ x1 */
	.else
	.if \m==64
	movq	\x2, \y1
	movq	\x0, \y2
	movq	\x0, \y0
	xorq	\x3, \y1	/* y1 = x2 ^ x3 */
	orq		\x1, \y2	/* y2 = x0 | x1 */
	xorq	\x2, \y0	/* y0 = x0 ^ x2 */
	xorq	\y1, \y2	/* y2 = y2 ^ y1 */
	andq	\x2, \y1	/* y1 = y1 & x2 */
	orq		\x1, \x2	/* x2 = x2 | x1 */
	xorq	\x3, \x1	/* x1 = x1 ^ x3 */
	orq		\x0, \y1	/* y1 = y1 | x0 */
	andq	\x2, \x1	/* x1 = x1 & x2 */
	orq		\y2, \x0	/* x0 = x0 | y2 */
	xorq	\x1, \y1	/* y1 = y1 ^ x1 */
	movq	\y2, \x1
	xorq	\y1, \x0	/* x0 = x0 ^ y1 */
	notq	\y2			/* y2 = ~ y2 */
	andq	\x0, \x1	/* x1 = x0 &~y2 */
	orq		\y2, \x3	/* x3 = x3 | y2 */
	xorq	\x2, \x3	/* x3 = x3 ^ x2 */
	movq	\x0, \y3
	orq		\x3, \x1	/* x1 = x1 | x3 */
	xorq	\x3, \y3	/* y3 = x0 ^ x3 */
	xorq	\x1, \y0	/* y0 = y0 ^ x1 */
	.else
	.if \m==128
	vxorpd	\x3, \x2, \y1	/* y1 = x2 ^ x3 */
	vorpd	\x1, \x0, \y2	/* y2 = x0 | x1 */
	vxorpd	\x2, \x0, \y0	/* y0 = x0 ^ x2 */
	vxorpd	\y1, \y2, \y2	/* y2 = y2 ^ y1 */
	vandpd	\x2, \y1, \y1	/* y1 = y1 & x2 */
	vorpd	\x1, \x2, \x2	/* x2 = x2 | x1 */
	vxorpd	\x3, \x1, \x1	/* x1 = x1 ^ x3 */
	vorpd	\x0, \y1, \y1	/* y1 = y1 | x0 */
	vandpd	\x2, \x1, \x1	/* x1 = x1 & x2 */
	vorpd	\y2, \x0, \x0	/* x0 = x0 | y2 */
	vxorpd	\x1, \y1, \y1	/* y1 = y1 ^ x1 */
	vxorpd	\y1, \x0, \y3	/* y3 = x0 ^ y1 */
	vandpd	\y3, \y2, \x1	/* x1 = y2 & y3 */
	vnotpd	\y2, \y2		/* y2 = ~ y2 */
	vorpd	\y2, \x3, \x3	/* x3 = x3 | y2 */
	vxorpd	\x2, \x3, \x3	/* x3 = x3 ^ x2 */
	vorpd	\x3, \x1, \x1	/* x1 = x1 | x3 */
	vxorpd	\x3, \y3, \y3	/* y3 = y3 ^ x3 */
	vxorpd	\x1, \y0, \y0	/* y0 = y0 ^ x1 */
	.else
	.error	"Invalid mode for IBOX0"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX1	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y1
	movl	\x2, \y2
	movl	\x1, \y0
	movl	\x0, \y3
	orl		\x3, \y1	/* y1 = x0 | x3 */
	xorl	\x3, \y2	/* y2 = x2 ^ x3 */
	notl	\y0			/* y0 = ~ x1 */
	xorl	\x2, \y3	/* y3 = x0 ^ x2 */
	orl		\x0, \y0	/* y0 = y0 | x0 */
	movl	\y1, \x0
	andl	\x3, \y3	/* y3 = y3 & x3 */
	andl	\y2, \x0	/* x0 = y1 & y2 */
	orl		\x1, \y3	/* y3 = y3 | x1 */
	xorl	\y0, \y2	/* y2 = y2 ^ y0 */
	xorl	\x0, \y3	/* y3 = y3 ^ x0 */
	movl	\y1, \x0
	xorl	\y3, \x0	/* x0 = y1 ^ y3 */
	movl	\x1, \y1
	xorl	\y2, \x0	/* x0 = x0 ^ y2 */
	andl	\x3, \y1	/* y1 = x1 & x3 */
	movl	\y3, \x3
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	orl		\y1, \x3	/* x3 = y1 | y3 */
	notl	\y3			/* y3 = ~ y3 */
	andl	\x3, \y0	/* y0 = y0 & x3 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	.else
	.if \m==64
	movq	\x0, \y1
	movq	\x2, \y2
	movq	\x1, \y0
	movq	\x0, \y3
	orq		\x3, \y1	/* y1 = x0 | x3 */
	xorq	\x3, \y2	/* y2 = x2 ^ x3 */
	notq	\y0			/* y0 = ~ x1 */
	xorq	\x2, \y3	/* y3 = x0 ^ x2 */
	orq		\x0, \y0	/* y0 = y0 | x0 */
	movq	\y1, \x0
	andq	\x3, \y3	/* y3 = y3 & x3 */
	andq	\y2, \x0	/* x0 = y1 & y2 */
	orq		\x1, \y3	/* y3 = y3 | x1 */
	xorq	\y0, \y2	/* y2 = y2 ^ y0 */
	xorq	\x0, \y3	/* y3 = y3 ^ x0 */
	movq	\y1, \x0
	xorq	\y3, \x0	/* x0 = y1 ^ y3 */
	movq	\x1, \y1
	xorq	\y2, \x0	/* x0 = x0 ^ y2 */
	andq	\x3, \y1	/* y1 = x1 & x3 */
	movq	\y3, \x3
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	orq		\y1, \x3	/* x3 = y1 | y3 */
	notq	\y3			/* y3 = ~ y3 */
	andq	\x3, \y0	/* y0 = y0 & x3 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	.else
	.if \m==128
	vorpd	\x3, \x0, \y1	/* y1 = x0 | x3 */
	vxorpd	\x3, \x2, \y2	/* y2 = x2 ^ x3 */
	vnotpd	\x1, \y0		/* y0 = ~ x1 */
	vxorpd	\x2, \x0, \y3	/* y3 = x0 ^ x2 */
	vorpd	\x0, \y0, \y0	/* y0 = y0 | x0 */
	vandpd	\x3, \y3, \y3	/* y3 = y3 & x3 */
	vandpd	\y2, \y1, \x0	/* x0 = y1 & y2 */
	vorpd	\x1, \y3, \y3	/* y3 = y3 | x1 */
	vxorpd	\y0, \y2, \y2	/* y2 = y2 ^ y0 */
	vxorpd	\x0, \y3, \y3	/* y3 = y3 ^ x0 */
	vxorpd	\y3, \y1, \x0	/* x0 = y1 ^ y3 */
	vxorpd	\y2, \x0, \x0	/* x0 = x0 ^ y2 */
	vandpd	\x3, \x1, \y1	/* y1 = x1 & x3 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vorpd	\y3, \y1, \x3	/* x3 = y1 | y3 */
	vnotpd	\y3, \y3		/* y3 = ~ y3 */
	vandpd	\x3, \y0, \y0	/* y0 = y0 & x3 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	.else
	.error	"Invalid mode for SBOX1"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX1	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x1, \y1
	movl	\x0, \y3
	movl	\x0, \y0
	orl		\x3, \y1	/* y1 = x1 | x3 */
	xorl	\x1, \y3	/* y3 = x0 ^ x1 */
	xorl	\x2, \y1	/* y1 = y1 ^ x2 */
	orl		\y1, \y0	/* y0 = x0 | y1 */
	movl	\x0, \y2
	andl	\y3, \y0	/* y0 = y0 & y3 */
	xorl	\y1, \y3	/* y3 = y3 ^ y1 */
	xorl	\y0, \x1	/* x1 = x1 ^ y0 */
	andl	\x2, \y2	/* y2 = x0 & x2 */
	andl	\x3, \x1	/* x1 = x1 & x3 */
	orl		\y2, \y1	/* y1 = y1 | y2 */
	orl		\x3, \y2	/* y2 = y2 | x3 */
	xorl	\y0, \y2	/* y2 = y2 ^ y0 */
	xorl	\x1, \y1	/* y1 = y1 ^ x1 */
	notl	\y2			/* y2 = ~ y2 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	orl		\y2, \x0	/* x0 = x0 | y2 */
	xorl	\y1, \y0	/* y0 = y0 ^ y1 */
	xorl	\x0, \y0	/* y0 = y0 ^ x0 */
	.else
	.if \m==64
	movq	\x1, \y1
	movq	\x0, \y3
	movq	\x0, \y0
	orq		\x3, \y1	/* y1 = x1 | x3 */
	xorq	\x1, \y3	/* y3 = x0 ^ x1 */
	xorq	\x2, \y1	/* y1 = y1 ^ x2 */
	orq		\y1, \y0	/* y0 = x0 | y1 */
	movq	\x0, \y2
	andq	\y3, \y0	/* y0 = y0 & y3 */
	xorq	\y1, \y3	/* y3 = y3 ^ y1 */
	xorq	\y0, \x1	/* x1 = x1 ^ y0 */
	andq	\x2, \y2	/* y2 = x0 & x2 */
	andq	\x3, \x1	/* x1 = x1 & x3 */
	orq		\y2, \y1	/* y1 = y1 | y2 */
	orq		\x3, \y2	/* y2 = y2 | x3 */
	xorq	\y0, \y2	/* y2 = y2 ^ y0 */
	xorq	\x1, \y1	/* y1 = y1 ^ x1 */
	notq	\y2			/* y2 = ~ y2 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	orq		\y2, \x0	/* x0 = x0 | y2 */
	xorq	\y1, \y0	/* y0 = y0 ^ y1 */
	xorq	\x0, \y0	/* y0 = y0 ^ x0 */
	.else
	.if \m==128
	vorpd	\x3, \x1, \y1	/* y1 = x1 | x3 */
	vxorpd	\x1, \x0, \y3	/* y3 = x0 ^ x1 */
	vxorpd	\x2, \y1, \y1	/* y1 = y1 ^ x2 */
	vorpd	\y1, \x0, \y0	/* y0 = x0 | y1 */
	vandpd	\y3, \y0, \y0	/* y0 = y0 & y3 */
	vxorpd	\y1, \y3, \y3	/* y3 = y3 ^ y1 */
	vxorpd	\y0, \x1, \x1	/* x1 = x1 ^ y0 */
	vandpd	\x2, \x0, \y2	/* y2 = x0 & x2 */
	vandpd	\x3, \x1, \x1	/* x1 = x1 & x3 */
	vorpd	\y2, \y1, \y1	/* y1 = y1 | y2 */
	vorpd	\x3, \y2, \y2	/* y2 = y2 | x3 */
	vxorpd	\y0, \y2, \y2	/* y2 = y2 ^ y0 */
	vxorpd	\x1, \y1, \y1	/* y1 = y1 ^ x1 */
	vnotpd	\y2, \y2		/* y2 = ~ y2 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vorpd	\y2, \x0, \x0	/* x0 = x0 | y2 */
	vxorpd	\y1, \y0, \y0	/* y0 = y0 ^ y1 */
	vxorpd	\x0, \y0, \y0	/* y0 = y0 ^ x0 */
	.else
	.error	"Invalid mode for IBOX1"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX2	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y1
	movl	\x0, \y2
	movl	\x3, \y3
	xorl	\x1, \y1	/* y1 = x0 ^ x1 */
	orl		\x2, \y2	/* y2 = x0 | x2 */
	movl	\y1, \y0
	xorl	\y2, \y3	/* y3 = x3 ^ y2 */
	xorl	\y3, \y0	/* y0 = y1 ^ y3 */
	orl		\x0, \x3	/* x3 = x3 | x0 */
	xorl	\y0, \x2	/* x2 = x2 ^ y0 */
	movl	\x1, \x0
	xorl	\x2, \x0	/* x0 = x1 ^ x2 */
	orl		\x1, \x2	/* x2 = x2 | x1 */
	andl	\y2, \x0	/* x0 = x0 & y2 */
	xorl	\x2, \y3	/* y3 = y3 ^ x2 */
	orl		\y3, \y1	/* y1 = y1 | y3 */
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	movl	\y3, \y2
	xorl	\y1, \y2	/* y2 = y3 ^ y1 */
	notl	\y3			/* y3 = ~ y3 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	xorl	\x3, \y2	/* y2 = y2 ^ x3 */
	.else
	.if \m==64
	movq	\x0, \y1
	movq	\x0, \y2
	movq	\x3, \y3
	xorq	\x1, \y1	/* y1 = x0 ^ x1 */
	orq		\x2, \y2	/* y2 = x0 | x2 */
	movq	\y1, \y0
	xorq	\y2, \y3	/* y3 = x3 ^ y2 */
	xorq	\y3, \y0	/* y0 = y1 ^ y3 */
	orq		\x0, \x3	/* x3 = x3 | x0 */
	xorq	\y0, \x2	/* x2 = x2 ^ y0 */
	movq	\x1, \x0
	xorq	\x2, \x0	/* x0 = x1 ^ x2 */
	orq		\x1, \x2	/* x2 = x2 | x1 */
	andq	\y2, \x0	/* x0 = x0 & y2 */
	xorq	\x2, \y3	/* y3 = y3 ^ x2 */
	orq		\y3, \y1	/* y1 = y1 | y3 */
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	movq	\y3, \y2
	xorq	\y1, \y2	/* y2 = y3 ^ y1 */
	notq	\y3			/* y3 = ~ y3 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	xorq	\x3, \y2	/* y2 = y2 ^ x3 */
	.else
	.if \m==128
	vxorpd	\x1, \x0, \y1	/* y1 = x0 ^ x1 */
	vorpd	\x2, \x0, \y2	/* y2 = x0 | x2 */
	vxorpd	\y2, \x3, \y3	/* y3 = x3 ^ y2 */
	vxorpd	\y3, \y1, \y0	/* y0 = y1 ^ y3 */
	vorpd	\x0, \x3, \x3	/* x3 = x3 | x0 */
	vxorpd	\y0, \x2, \x2	/* x2 = x2 ^ y0 */
	vxorpd	\x2, \x1, \x0	/* x0 = x1 ^ x2 */
	vorpd	\x1, \x2, \x2	/* x2 = x2 | x1 */
	vandpd	\y2, \x0, \x0	/* x0 = x0 & y2 */
	vxorpd	\x2, \y3, \y3	/* y3 = y3 ^ x2 */
	vorpd	\y3, \y1, \y1	/* y1 = y1 | y3 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vxorpd	\y1, \y3, \y2	/* y2 = y3 ^ y1 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	vnotpd	\y3, \y3		/* y3 = ~ y3 */
	vxorpd	\x3, \y2, \y2	/* y2 = y2 ^ x3 */
	.else
	.error	"Invalid mode for SBOX2"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX2	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y0
	movl	\x2, \y2
	movl	\x1, \y1
	xorl	\x3, \y2	/* y2 = x2 ^ x3 */
	xorl	\x3, \y0	/* y0 = x0 ^ x3 */
	orl		\y2, \y1	/* y1 = x1 | y2 */
	xorl	\y1, \y0	/* y0 = y0 ^ y1 */
	movl	\x3, \y1
	orl		\y0, \y1	/* y1 = x3 | y0 */
	andl	\x1, \y1	/* y1 = y1 & x1 */
	movl	\x0, \y3
	notl	\x3			/* x3 = ~ x3 */
	orl		\x2, \y3	/* y3 = x0 | x2 */
	andl	\y3, \y2	/* y2 = y2 & y3 */
	andl	\x1, \y3	/* y3 = y3 & x1 */
	andl	\x2, \x0	/* x0 = x0 & x2 */
	xorl	\y2, \y1	/* y1 = y1 ^ y2 */
	orl		\x3, \x0	/* x0 = x0 | x3 */
	xorl	\x0, \y3	/* y3 = y3 ^ x0 */
	andl	\y3, \x2	/* x2 = x2 & y3 */
	movl	\y0, \y2
	xorl	\x0, \x2	/* x2 = x2 ^ x0 */
	xorl	\y1, \y2	/* y2 = y0 ^ y1 */
	xorl	\x2, \y2	/* y2 = y2 ^ x2 */
	.else
	.if \m==64
	movq	\x0, \y0
	movq	\x2, \y2
	movq	\x1, \y1
	xorq	\x3, \y2	/* y2 = x2 ^ x3 */
	xorq	\x3, \y0	/* y0 = x0 ^ x3 */
	orq		\y2, \y1	/* y1 = x1 | y2 */
	xorq	\y1, \y0	/* y0 = y0 ^ y1 */
	movq	\x3, \y1
	orq		\y0, \y1	/* y1 = x3 | y0 */
	andq	\x1, \y1	/* y1 = y1 & x1 */
	movq	\x0, \y3
	notq	\x3			/* x3 = ~ x3 */
	orq		\x2, \y3	/* y3 = x0 | x2 */
	andq	\y3, \y2	/* y2 = y2 & y3 */
	andq	\x1, \y3	/* y3 = y3 & x1 */
	andq	\x2, \x0	/* x0 = x0 & x2 */
	xorq	\y2, \y1	/* y1 = y1 ^ y2 */
	orq		\x3, \x0	/* x0 = x0 | x3 */
	xorq	\x0, \y3	/* y3 = y3 ^ x0 */
	andq	\y3, \x2	/* x2 = x2 & y3 */
	movq	\y0, \y2
	xorq	\x0, \x2	/* x2 = x2 ^ x0 */
	xorq	\y1, \y2	/* y2 = y0 ^ y1 */
	xorq	\x2, \y2	/* y2 = y2 ^ x2 */
	.else
	.if \m==128
	vxorpd	\x3, \x2, \y2	/* y2 = x2 ^ x3 */
	vxorpd	\x3, \x0, \y0	/* y0 = x0 ^ x3 */
	vorpd	\y2, \x1, \y1	/* y1 = x1 | y2 */
	vxorpd	\y1, \y0, \y0	/* y0 = y0 ^ y1 */
	vorpd	\y0, \x3, \y1	/* y1 = x3 | y0 */
	vandpd	\x1, \y1, \y1	/* y1 = y1 & x1 */
	vnotpd	\x3, \x3		/* x3 = ~ x3 */
	vorpd	\x2, \x0, \y3	/* y3 = x0 | x2 */
	vandpd	\y3, \y2, \y2	/* y2 = y2 & y3 */
	vandpd	\x1, \y3, \y3	/* y3 = y3 & x1 */
	vandpd	\x2, \x0, \x0	/* x0 = x0 & x2 */
	vxorpd	\y2, \y1, \y1	/* y1 = y1 ^ y2 */
	vorpd	\x3, \x0, \x0	/* x0 = x0 | x3 */
	vxorpd	\x0, \y3, \y3	/* y3 = y3 ^ x0 */
	vandpd	\y3, \x2, \x2	/* x2 = x2 & y3 */
	vxorpd	\x0, \x2, \x2	/* x2 = x2 ^ x0 */
	vxorpd	\y1, \y0, \y2	/* y2 = y0 ^ y1 */
	vxorpd	\x2, \y2, \y2	/* y2 = y2 ^ x2 */
	.else
	.error	"Invalid mode for IBOX2"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX3	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y1
	movl	\x0, \y0
	movl	\x0, \y3
	movl	\x0, \y2
	xorl	\x2, \y1	/* y1 = x0 ^ x2 */
	orl		\x3, \y0	/* y0 = x0 | x3 */
	andl	\x3, \y3	/* y3 = x0 & x3 */
	andl	\x1, \y2	/* y2 = x0 & x1 */
	andl	\y0, \y1	/* y1 = y1 & y0 */
	orl		\x2, \y2	/* y2 = y2 | x2 */
	movl	\x3, \x2
	orl		\x1, \y3	/* y3 = y3 | x1 */
	xorl	\y1, \x2	/* x2 = x3 ^ y1 */
	xorl	\y3, \y1	/* y1 = y1 ^ y3 */
	orl		\x2, \x0	/* x0 = x0 | x2 */
	xorl	\x1, \x2	/* x2 = x2 ^ x1 */
	andl	\x3, \y3	/* y3 = y3 & x3 */
	xorl	\y3, \y0	/* y0 = y0 ^ y3 */
	movl	\y2, \y3
	xorl	\x2, \y3	/* y3 = y2 ^ x2 */
	xorl	\y0, \y2	/* y2 = y2 ^ y0 */
	orl		\y3, \x3	/* x3 = x3 | y3 */
	andl	\x3, \x1	/* x1 = x1 & x3 */
	movl	\x0, \y0
	xorl	\x1, \y0	/* y0 = y0 ^ x1 */
	.else
	.if \m==64
	movq	\x0, \y1
	movq	\x0, \y0
	movq	\x0, \y3
	movq	\x0, \y2
	xorq	\x2, \y1	/* y1 = x0 ^ x2 */
	orq		\x3, \y0	/* y0 = x0 | x3 */
	andq	\x3, \y3	/* y3 = x0 & x3 */
	andq	\x1, \y2	/* y2 = x0 & x1 */
	andq	\y0, \y1	/* y1 = y1 & y0 */
	orq		\x2, \y2	/* y2 = y2 | x2 */
	movq	\x3, \x2
	orq		\x1, \y3	/* y3 = y3 | x1 */
	xorq	\y1, \x2	/* x2 = x3 ^ y1 */
	xorq	\y3, \y1	/* y1 = y1 ^ y3 */
	orq		\x2, \x0	/* x0 = x0 | x2 */
	xorq	\x1, \x2	/* x2 = x2 ^ x1 */
	andq	\x3, \y3	/* y3 = y3 & x3 */
	xorq	\y3, \y0	/* y0 = y0 ^ y3 */
	movq	\y2, \y3
	xorq	\x2, \y3	/* y3 = y2 ^ x2 */
	xorq	\y0, \y2	/* y2 = y2 ^ y0 */
	orq		\y3, \x3	/* x3 = x3 | y3 */
	andq	\x3, \x1	/* x1 = x1 & x3 */
	movq	\x0, \y0
	xorq	\x1, \y0	/* y0 = y0 ^ x1 */
	.else
	.if \m==128
	vxorpd	\x2, \x0, \y1	/* y1 = x0 ^ x2 */
	vorpd	\x3, \x0, \y0	/* y0 = x0 | x3 */
	vandpd	\x3, \x0, \y3	/* y3 = x0 & x3 */
	vandpd	\x1, \x0, \y2	/* y2 = x0 & x1 */
	vandpd	\y0, \y1, \y1	/* y1 = y1 & y0 */
	vorpd	\x2, \y2, \y2	/* y2 = y2 | x2 */
	vorpd	\x1, \y3, \y3	/* y3 = y3 | x1 */
	vxorpd	\y1, \x3, \x2	/* x2 = x3 ^ y1 */
	vxorpd	\y3, \y1, \y1	/* y1 = y1 ^ y3 */
	vorpd	\x2, \x0, \x0	/* x0 = x0 | x2 */
	vxorpd	\x1, \x2, \x2	/* x2 = x2 ^ x1 */
	vandpd	\x3, \y3, \y3	/* y3 = y3 & x3 */
	vxorpd	\y3, \y0, \y0	/* y0 = y0 ^ y3 */
	vxorpd	\x2, \y2, \y3	/* y3 = y2 ^ x2 */
	vxorpd	\y0, \y2, \y2	/* y2 = y2 ^ y0 */
	vorpd	\y3, \x3, \x3	/* x3 = x3 | y3 */
	vandpd	\x3, \x1, \x1	/* x1 = x1 & x3 */
	vxorpd	\x1, \x0, \y0	/* y0 = x0 ^ x1 */
	.else
	.error	"Invalid mode for SBOX3"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX3	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x2, \y3
	movl	\x0, \y2
	movl	\x1, \y0
	movl	\x2, \y1
	orl		\x3, \y3	/* y3 = x2 | x3 */
	orl		\x3, \y2	/* y2 = x0 | x3 */
	andl	\y3, \y0	/* y0 = x1 & y3 */
	xorl	\y2, \y1	/* y1 = x2 ^ y2 */
	xorl	\x0, \x3	/* x3 = x3 ^ x0 */
	xorl	\y1, \y0	/* y0 = y0 ^ y1 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	xorl	\x3, \y3	/* y3 = y3 ^ x3 */
	andl	\y1, \y2	/* y2 = y2 & y1 */
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	xorl	\x3, \y2	/* y2 = y2 ^ x3 */
	orl		\y0, \x3	/* x3 = x3 | y0 */
	andl	\y2, \x0	/* x0 = x0 & y2 */
	andl	\x3, \y1	/* y1 = y1 & x3 */
	orl		\x1, \x0	/* x0 = x0 | x1 */
	xorl	\x1, \y1	/* y1 = y1 ^ x1 */
	xorl	\x0, \y3	/* y3 = y3 ^ x0 */
	.else
	.if \m==64
	movq	\x2, \y3
	movq	\x0, \y2
	movq	\x1, \y0
	movq	\x2, \y1
	orq		\x3, \y3	/* y3 = x2 | x3 */
	orq		\x3, \y2	/* y2 = x0 | x3 */
	andq	\y3, \y0	/* y0 = x1 & y3 */
	xorq	\y2, \y1	/* y1 = x2 ^ y2 */
	xorq	\x0, \x3	/* x3 = x3 ^ x0 */
	xorq	\y1, \y0	/* y0 = y0 ^ y1 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	xorq	\x3, \y3	/* y3 = y3 ^ x3 */
	andq	\y1, \y2	/* y2 = y2 & y1 */
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	xorq	\x3, \y2	/* y2 = y2 ^ x3 */
	orq		\y0, \x3	/* x3 = x3 | y0 */
	andq	\y2, \x0	/* x0 = x0 & y2 */
	andq	\x3, \y1	/* y1 = y1 & x3 */
	orq		\x1, \x0	/* x0 = x0 | x1 */
	xorq	\x1, \y1	/* y1 = y1 ^ x1 */
	xorq	\x0, \y3	/* y3 = y3 ^ x0 */
	.else
	.if \m==128
	vorpd	\x3, \x2, \y3	/* y3 = x2 | x3 */
	vorpd	\x3, \x0, \y2	/* y2 = x0 | x3 */
	vandpd	\y3, \x1, \y0	/* y0 = x1 & y3 */
	vxorpd	\y2, \x2, \y1	/* y1 = x2 ^ y2 */
	vxorpd	\x0, \x3, \x3	/* x3 = x3 ^ x0 */
	vxorpd	\y1, \y0, \y0	/* y0 = y0 ^ y1 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	vxorpd	\x3, \y3, \y3	/* y3 = y3 ^ x3 */
	vandpd	\y1, \y2, \y2	/* y2 = y2 & y1 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vxorpd	\x3, \y2, \y2	/* y2 = y2 ^ x3 */
	vorpd	\y0, \x3, \x3	/* x3 = x3 | y0 */
	vandpd	\y2, \x0, \x0	/* x0 = x0 & y2 */
	vandpd	\x3, \y1, \y1	/* y1 = y1 & x3 */
	vorpd	\x1, \x0, \x0	/* x0 = x0 | x1 */
	vxorpd	\x1, \y1, \y1	/* y1 = y1 ^ x1 */
	vxorpd	\x0, \y3, \y3	/* y3 = y3 ^ x0 */
	.else
	.error	"Invalid mode for IBOX3"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX4	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y3
	movl	\x1, \y2
	movl	\x1, \y0
	orl		\x1, \y3	/* y3 = x0 | x1 */
	orl		\x2, \y2	/* y2 = x1 | x2 */
	andl	\x3, \y3	/* y3 = y3 & x3 */
	xorl	\x0, \y2	/* y2 = y2 ^ x0 */
	xorl	\x3, \y0	/* y0 = x1 ^ x3 */
	orl		\y2, \x3	/* x3 = x3 | y2 */
	andl	\x2, \x1	/* x1 = x1 & x2 */
	xorl	\y3, \x2	/* x2 = x2 ^ y3 */
	xorl	\y2, \y3	/* y3 = y3 ^ y2 */
	andl	\x3, \x0	/* x0 = x0 & x3 */
	movl	\y3, \y1
	orl		\x1, \y2	/* y2 = y2 | x1 */
	andl	\y0, \y1	/* y1 = y3 & y0 */
	xorl	\y1, \y2	/* y2 = y2 ^ y1 */
	xorl	\y0, \y1	/* y1 = y1 ^ y0 */
	andl	\x3, \y0	/* y0 = y0 & x3 */
	orl		\x1, \y1	/* y1 = y1 | x1 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	notl	\y0			/* y0 = ~ y0 */
	.else
	.if \m==64
	movq	\x0, \y3
	movq	\x1, \y2
	movq	\x1, \y0
	orq		\x1, \y3	/* y3 = x0 | x1 */
	orq		\x2, \y2	/* y2 = x1 | x2 */
	andq	\x3, \y3	/* y3 = y3 & x3 */
	xorq	\x0, \y2	/* y2 = y2 ^ x0 */
	xorq	\x3, \y0	/* y0 = x1 ^ x3 */
	orq		\y2, \x3	/* x3 = x3 | y2 */
	andq	\x2, \x1	/* x1 = x1 & x2 */
	xorq	\y3, \x2	/* x2 = x2 ^ y3 */
	xorq	\y2, \y3	/* y3 = y3 ^ y2 */
	andq	\x3, \x0	/* x0 = x0 & x3 */
	movq	\y3, \y1
	orq		\x1, \y2	/* y2 = y2 | x1 */
	andq	\y0, \y1	/* y1 = y3 & y0 */
	xorq	\y1, \y2	/* y2 = y2 ^ y1 */
	xorq	\y0, \y1	/* y1 = y1 ^ y0 */
	andq	\x3, \y0	/* y0 = y0 & x3 */
	orq		\x1, \y1	/* y1 = y1 | x1 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	notq	\y0			/* y0 = ~ y0 */
	.else
	.if \m==128
	vorpd	\x1, \x0, \y3	/* y3 = x0 | x1 */
	vorpd	\x2, \x1, \y2	/* y2 = x1 | x2 */
	vandpd	\x3, \y3, \y3	/* y3 = y3 & x3 */
	vxorpd	\x0, \y2, \y2	/* y2 = y2 ^ x0 */
	vxorpd	\x3, \x1, \y0	/* y0 = x1 ^ x3 */
	vorpd	\y2, \x3, \x3	/* x3 = x3 | y2 */
	vandpd	\x2, \x1, \x1	/* x1 = x1 & x2 */
	vxorpd	\y3, \x2, \x2	/* x2 = x2 ^ y3 */
	vxorpd	\y2, \y3, \y3	/* y3 = y3 ^ y2 */
	vandpd	\x3, \x0, \x0	/* x0 = x0 & x3 */
	vorpd	\x1, \y2, \y2	/* y2 = y2 | x1 */
	vandpd	\y0, \y3, \y1	/* y1 = y3 & y0 */
	vxorpd	\y1, \y2, \y2	/* y2 = y2 ^ y1 */
	vxorpd	\y0, \y1, \y1	/* y1 = y1 ^ y0 */
	vandpd	\x3, \y0, \y0	/* y0 = y0 & x3 */
	vorpd	\x1, \y1, \y1	/* y1 = y1 | x1 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vnotpd	\y0, \y0		/* y0 = ~ y0 */
	.else
	.error	"Invalid mode for SBOX4"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX4	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x2, \y1
	movl	\x2, \y2
	movl	\x0, \y0
	orl		\x3, \y2	/* y2 = x2 | x3 */
	xorl	\x3, \y1	/* y1 = x2 ^ x3 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	orl		\x3, \x1	/* x1 = x1 | x3 */
	xorl	\y2, \y0	/* y0 = x0 ^ y2 */
	xorl	\y2, \x3	/* x3 = x3 ^ y2 */
	andl	\x0, \y2	/* y2 = y2 & x0 */
	xorl	\y2, \y1	/* y1 = y1 ^ y2 */
	xorl	\x0, \y2	/* y2 = y2 ^ x0 */
	andl	\x1, \x0	/* x0 = x0 & x1 */
	orl		\x2, \y2	/* y2 = y2 | x2 */
	movl	\x0, \y3
	notl	\x0			/* x0 = ~ x0 */
	xorl	\x3, \y3	/* y3 = x3 ^~x0 */
	orl		\y1, \x0	/* x0 = x0 | y1 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	xorl	\x0, \y0	/* y0 = y0 ^ x0 */
	xorl	\x0, \y2	/* y2 = y2 ^ x0 */
	.else
	.if \m==64
	movq	\x2, \y1
	movq	\x2, \y2
	movq	\x0, \y0
	orq		\x3, \y2	/* y2 = x2 | x3 */
	xorq	\x3, \y1	/* y1 = x2 ^ x3 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	orq		\x3, \x1	/* x1 = x1 | x3 */
	xorq	\y2, \y0	/* y0 = x0 ^ y2 */
	xorq	\y2, \x3	/* x3 = x3 ^ y2 */
	andq	\x0, \y2	/* y2 = y2 & x0 */
	xorq	\y2, \y1	/* y1 = y1 ^ y2 */
	xorq	\x0, \y2	/* y2 = y2 ^ x0 */
	andq	\x1, \x0	/* x0 = x0 & x1 */
	orq		\x2, \y2	/* y2 = y2 | x2 */
	movq	\x0, \y3
	notq	\x0			/* x0 = ~ x0 */
	xorq	\x3, \y3	/* y3 = x3 ^~x0 */
	orq		\y1, \x0	/* x0 = x0 | y1 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	xorq	\x0, \y0	/* y0 = y0 ^ x0 */
	xorq	\x0, \y2	/* y2 = y2 ^ x0 */
	.else
	.if \m==128
	vorpd	\x3, \x2, \y2	/* y2 = x2 | x3 */
	vxorpd	\x3, \x2, \y1	/* y1 = x2 ^ x3 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	vorpd	\x3, \x1, \x1	/* x1 = x1 | x3 */
	vxorpd	\y2, \x0, \y0	/* y0 = x0 ^ y2 */
	vxorpd	\y2, \x3, \x3	/* x3 = x3 ^ y2 */
	vandpd	\x0, \y2, \y2	/* y2 = y2 & x0 */
	vxorpd	\y2, \y1, \y1	/* y1 = y1 ^ y2 */
	vxorpd	\x0, \y2, \y2	/* y2 = y2 ^ x0 */
	vandpd	\x1, \x0, \x0	/* x0 = x0 & x1 */
	vorpd	\x2, \y2, \y2	/* y2 = y2 | x2 */
	vxorpd	\x0, \x3, \y3	/* y3 = x3 ^ x0 */
	vnotpd	\x0, \x0		/* x0 = ~ x0 */
	vorpd	\y1, \x0, \x0	/* x0 = x0 | y1 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	vxorpd	\x0, \y0, \y0	/* y0 = y0 ^ x0 */
	vxorpd	\x0, \y2, \y2	/* y2 = y2 ^ x0 */
	.else
	.error	"Invalid mode for IBOX4"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX5	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x1, \y0
	movl	\x0, \y2
	orl		\x3, \y0	/* y0 = x1 | x3 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	movl	\x1, \x2
	xorl	\x3, \x2	/* x2 = x1 ^ x3 */
	xorl	\x2, \y2	/* y2 = x0 ^ x2 */
	andl	\x2, \x0	/* x0 = x0 & x2 */
	movl	\x1, \y3
	xorl	\x0, \y0	/* y0 = y0 ^ x0 */
	orl		\y2, \y3	/* y3 = x1 | y2 */
	orl		\y0, \x1	/* x1 = x1 | y0 */
	notl	\y0			/* y0 = ~ y0 */
	xorl	\x2, \y3	/* y3 = y3 ^ x2 */
	orl		\y0, \x0	/* x0 = x0 | y0 */
	movl	\x3, \y1
	xorl	\x0, \y3	/* y3 = y3 ^ x0 */
	orl		\y0, \y1	/* y1 = x3 | y0 */
	xorl	\y1, \x3	/* x3 = x3 ^ y1 */
	xorl	\y2, \y1	/* y1 = y1 ^ y2 */
	orl		\x3, \y2	/* y2 = y2 | x3 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	.else
	.if \m==64
	movq	\x1, \y0
	movq	\x0, \y2
	orq		\x3, \y0	/* y0 = x1 | x3 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	movq	\x1, \x2
	xorq	\x3, \x2	/* x2 = x1 ^ x3 */
	xorq	\x2, \y2	/* y2 = x0 ^ x2 */
	andq	\x2, \x0	/* x0 = x0 & x2 */
	movq	\x1, \y3
	xorq	\x0, \y0	/* y0 = y0 ^ x0 */
	orq		\y2, \y3	/* y3 = x1 | y2 */
	orq		\y0, \x1	/* x1 = x1 | y0 */
	notq	\y0			/* y0 = ~ y0 */
	xorq	\x2, \y3	/* y3 = y3 ^ x2 */
	orq		\y0, \x0	/* x0 = x0 | y0 */
	movq	\x3, \y1
	xorq	\x0, \y3	/* y3 = y3 ^ x0 */
	orq		\y0, \y1	/* y1 = x3 | y0 */
	xorq	\y1, \x3	/* x3 = x3 ^ y1 */
	xorq	\y2, \y1	/* y1 = y1 ^ y2 */
	orq		\x3, \y2	/* y2 = y2 | x3 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	.else
	.if \m==128
	vorpd	\x3, \x1, \y0	/* y0 = x1 | x3 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vxorpd	\x3, \x1, \x2	/* x2 = x1 ^ x3 */
	vxorpd	\x2, \x0, \y2	/* y2 = x0 ^ x2 */
	vandpd	\x2, \x0, \x0	/* x0 = x0 & x2 */
	vxorpd	\x0, \y0, \y0	/* y0 = y0 ^ x0 */
	vorpd	\y2, \x1, \y3	/* y3 = x1 | y2 */
	vorpd	\y0, \x1, \x1	/* x1 = x1 | y0 */
	vnotpd	\y0, \y0		/* y0 = ~ y0 */
	vxorpd	\x2, \y3, \y3	/* y3 = y3 ^ x2 */
	vorpd	\y0, \x0, \x0	/* x0 = x0 | y0 */
	vxorpd	\x0, \y3, \y3	/* y3 = y3 ^ x0 */
	vorpd	\y0, \x3, \y1	/* y1 = x3 | y0 */
	vxorpd	\y1, \x3, \x3	/* x3 = x3 ^ y1 */
	vxorpd	\y2, \y1, \y1	/* y1 = y1 ^ y2 */
	vorpd	\x3, \y2, \y2	/* y2 = y2 | x3 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	.else
	.error	"Invalid mode for SBOX5"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX5	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y1
	movl	\x2, \y3
	andl	\x3, \y1	/* y1 = x0 & x3 */
	movl	\x1, \y0
	xorl	\y1, \y3	/* y3 = x2 ^ y1 */
	movl	\x0, \y2
	andl	\y3, \y0	/* y0 = x1 & y3 */
	xorl	\x3, \y2	/* y2 = x0 ^ x3 */
	xorl	\x1, \x3	/* x3 = x3 ^ x1 */
	xorl	\y2, \y0	/* y0 = y0 ^ y2 */
	andl	\x0, \x2	/* x2 = x2 & x0 */
	andl	\y0, \x0	/* x0 = x0 & y0 */
	xorl	\y0, \y1	/* y1 = y1 ^ y0 */
	orl		\x1, \x2	/* x2 = x2 | x1 */
	xorl	\x2, \y1	/* y1 = y1 ^ x2 */
	movl	\y0, \y2
	notl	\x1			/* x1 = ~ x1 */
	orl		\y1, \y2	/* y2 = y0 | y1 */
	orl		\x0, \x1	/* x1 = x1 | x0 */
	xorl	\y3, \y2	/* y2 = y2 ^ y3 */
	xorl	\x1, \y3	/* y3 = y3 ^ x1 */
	xorl	\x3, \y2	/* y2 = y2 ^ x3 */
	.else
	.if \m==64
	movq	\x0, \y1
	movq	\x2, \y3
	andq	\x3, \y1	/* y1 = x0 & x3 */
	movq	\x1, \y0
	xorq	\y1, \y3	/* y3 = x2 ^ y1 */
	movq	\x0, \y2
	andq	\y3, \y0	/* y0 = x1 & y3 */
	xorq	\x3, \y2	/* y2 = x0 ^ x3 */
	xorq	\x1, \x3	/* x3 = x3 ^ x1 */
	xorq	\y2, \y0	/* y0 = y0 ^ y2 */
	andq	\x0, \x2	/* x2 = x2 & x0 */
	andq	\y0, \x0	/* x0 = x0 & y0 */
	xorq	\y0, \y1	/* y1 = y1 ^ y0 */
	orq		\x1, \x2	/* x2 = x2 | x1 */
	xorq	\x2, \y1	/* y1 = y1 ^ x2 */
	movq	\y0, \y2
	notq	\x1			/* x1 = ~ x1 */
	orq		\y1, \y2	/* y2 = y0 | y1 */
	orq		\x0, \x1	/* x1 = x1 | x0 */
	xorq	\y3, \y2	/* y2 = y2 ^ y3 */
	xorq	\x1, \y3	/* y3 = y3 ^ x1 */
	xorq	\x3, \y2	/* y2 = y2 ^ x3 */
	.else
	.if \m==128
	vandpd	\x3, \x0, \y1	/* y1 = x0 & x3 */
	vxorpd	\y1, \x2, \y3	/* y3 = x2 ^ y1 */
	vandpd	\y3, \x1, \y0	/* y0 = x1 & y3 */
	vxorpd	\x3, \x0, \y2	/* y2 = x0 ^ x3 */
	vxorpd	\x1, \x3, \x3	/* x3 = x3 ^ x1 */
	vxorpd	\y2, \y0, \y0	/* y0 = y0 ^ y2 */
	vandpd	\x0, \x2, \x2	/* x2 = x2 & x0 */
	vandpd	\y0, \x0, \x0	/* x0 = x0 & y0 */
	vxorpd	\y0, \y1, \y1	/* y1 = y1 ^ y0 */
	vorpd	\x1, \x2, \x2	/* x2 = x2 | x1 */
	vxorpd	\x2, \y1, \y1	/* y1 = y1 ^ x2 */
	vnotpd	\x1, \x1		/* x1 = ~ x1 */
	vorpd	\y1, \y0, \y2	/* y2 = y0 | y1 */
	vorpd	\x0, \x1, \x1	/* x1 = x1 | x0 */
	vxorpd	\y3, \y2, \y2	/* y2 = y2 ^ y3 */
	vxorpd	\x1, \y3, \y3	/* y3 = y3 ^ x1 */
	vxorpd	\x3, \y2, \y2	/* y2 = y2 ^ x3 */
	.else
	.error	"Invalid mode for IBOX5"
	.endif
	.endif
	.endif
	.endm


	.macro	SBOX6	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y0
	movl	\x0, \y1
	movl	\x0, \y2
	xorl	\x3, \y0	/* y0 = x0 ^ x3 */
	andl	\x3, \y1	/* y1 = x0 & x3 */
	orl		\x2, \y2	/* y2 = x0 | x2 */
	movl	\x1, \y3
	orl		\x1, \x3	/* x3 = x3 | x1 */
	xorl	\x1, \x0	/* x0 = x0 ^ x1 */
	xorl	\x2, \x3	/* x3 = x3 ^ x2 */
	orl		\x2, \y3	/* y3 = x1 | x2 */
	xorl	\x1, \x2	/* x2 = x2 ^ x1 */
	andl	\y0, \y3	/* y3 = y3 & y0 */
	xorl	\x2, \y1	/* y1 = y1 ^ x2 */
	notl	\y1			/* y1 = ~ y1 */
	andl	\y1, \x1	/* x1 = x1 & y1 */
	andl	\y1, \y0	/* y0 = y0 & y1 */
	xorl	\y3, \x1	/* x1 = x1 ^ y3 */
	xorl	\x3, \y3	/* y3 = y3 ^ x3 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	notl	\y2			/* y2 = ~ y2 */
	xorl	\x0, \y0	/* y0 = y0 ^ x0 */
	xorl	\y2, \y0	/* y0 = y0 ^ y2 */
	.else
	.if \m==64
	movq	\x0, \y0
	movq	\x0, \y1
	movq	\x0, \y2
	xorq	\x3, \y0	/* y0 = x0 ^ x3 */
	andq	\x3, \y1	/* y1 = x0 & x3 */
	orq		\x2, \y2	/* y2 = x0 | x2 */
	movq	\x1, \y3
	orq		\x1, \x3	/* x3 = x3 | x1 */
	xorq	\x1, \x0	/* x0 = x0 ^ x1 */
	xorq	\x2, \x3	/* x3 = x3 ^ x2 */
	orq		\x2, \y3	/* y3 = x1 | x2 */
	xorq	\x1, \x2	/* x2 = x2 ^ x1 */
	andq	\y0, \y3	/* y3 = y3 & y0 */
	xorq	\x2, \y1	/* y1 = y1 ^ x2 */
	notq	\y1			/* y1 = ~ y1 */
	andq	\y1, \x1	/* x1 = x1 & y1 */
	andq	\y1, \y0	/* y0 = y0 & y1 */
	xorq	\y3, \x1	/* x1 = x1 ^ y3 */
	xorq	\x3, \y3	/* y3 = y3 ^ x3 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	notq	\y2			/* y2 = ~ y2 */
	xorq	\x0, \y0	/* y0 = y0 ^ x0 */
	xorq	\y2, \y0	/* y0 = y0 ^ y2 */
	.else
	.if \m==128
	vxorpd	\x3, \x0, \y0	/* y0 = x0 ^ x3 */
	vandpd	\x3, \x0, \y1	/* y1 = x0 & x3 */
	vorpd	\x2, \x0, \y2	/* y2 = x0 | x2 */
	vorpd	\x1, \x3, \x3	/* x3 = x3 | x1 */
	vxorpd	\x1, \x0, \x0	/* x0 = x0 ^ x1 */
	vxorpd	\x2, \x3, \x3	/* x3 = x3 ^ x2 */
	vorpd	\x2, \x1, \y3	/* y3 = x1 | x2 */
	vxorpd	\x1, \x2, \x2	/* x2 = x2 ^ x1 */
	vandpd	\y0, \y3, \y3	/* y3 = y3 & y0 */
	vxorpd	\x2, \y1, \y1	/* y1 = y1 ^ x2 */
	vnotpd	\y1, \y1		/* y1 = ~ y1 */
	vandpd	\y1, \x1, \x1	/* x1 = x1 & y1 */
	vandpd	\y1, \y0, \y0	/* y0 = y0 & y1 */
	vxorpd	\y3, \x1, \x1	/* x1 = x1 ^ y3 */
	vxorpd	\x3, \y3, \y3	/* y3 = y3 ^ x3 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	vnotpd	\y2, \y2		/* y2 = ~ y2 */
	vxorpd	\x0, \y0, \y0	/* y0 = y0 ^ x0 */
	vxorpd	\y2, \y0, \y0	/* y0 = y0 ^ y2 */
	.else
	.error	"Invalid mode for SBOX6"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX6	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y2
	movl	\x1, \y0
	movl	\x0, \y1
	xorl	\x2, \y2	/* y2 = x0 ^ x2 */
	xorl	\x3, \y0	/* y0 = x1 ^ x3 */
	notl	\x2			/* x2 = ~ x2 */
	movl	\x1, \y3
	orl		\x2, \y1	/* y1 = x0 | x2 */
	andl	\y2, \y3	/* y3 = x1 & y2 */
	xorl	\y0, \y1	/* y1 = y1 ^ y0 */
	orl		\x3, \y3	/* y3 = y3 | x3 */
	orl		\x2, \x3	/* x3 = x3 | x2 */
	orl		\x1, \x2	/* x2 = x2 | x1 */
	andl	\x0, \x2	/* x2 = x2 & x0 */
	movl	\x2, \y0
	xorl	\y3, \y0	/* y0 = x2 ^ y3 */
	andl	\y2, \y3	/* y3 = y3 & y2 */
	notl	\y0			/* y0 = ~ y0 */
	xorl	\x2, \y3	/* y3 = y3 ^ x2 */
	xorl	\y1, \x0	/* x0 = x0 ^ y1 */
	andl	\y0, \x1	/* x1 = x1 & y0 */
	xorl	\x3, \y2	/* y2 = y2 ^ x3 */
	xorl	\x0, \y3	/* y3 = y3 ^ x0 */
	xorl	\x1, \y2	/* y2 = y2 ^ x1 */
	.else
	.if \m==64
	movq	\x0, \y2
	movq	\x1, \y0
	movq	\x0, \y1
	xorq	\x2, \y2	/* y2 = x0 ^ x2 */
	xorq	\x3, \y0	/* y0 = x1 ^ x3 */
	notq	\x2			/* x2 = ~ x2 */
	movq	\x1, \y3
	orq		\x2, \y1	/* y1 = x0 | x2 */
	andq	\y2, \y3	/* y3 = x1 & y2 */
	xorq	\y0, \y1	/* y1 = y1 ^ y0 */
	orq		\x3, \y3	/* y3 = y3 | x3 */
	orq		\x2, \x3	/* x3 = x3 | x2 */
	orq		\x1, \x2	/* x2 = x2 | x1 */
	andq	\x0, \x2	/* x2 = x2 & x0 */
	movq	\x2, \y0
	xorq	\y3, \y0	/* y0 = x2 ^ y3 */
	andq	\y2, \y3	/* y3 = y3 & y2 */
	notq	\y0			/* y0 = ~ y0 */
	xorq	\x2, \y3	/* y3 = y3 ^ x2 */
	xorq	\y1, \x0	/* x0 = x0 ^ y1 */
	andq	\y0, \x1	/* x1 = x1 & y0 */
	xorq	\x3, \y2	/* y2 = y2 ^ x3 */
	xorq	\x0, \y3	/* y3 = y3 ^ x0 */
	xorq	\x1, \y2	/* y2 = y2 ^ x1 */
	.else
	.if \m==128
	vxorpd	\x2, \x0, \y2	/* y2 = x0 ^ x2 */
	vxorpd	\x3, \x1, \y0	/* y0 = x1 ^ x3 */
	vnotpd	\x2, \x2		/* x2 = ~ x2 */
	vorpd	\x2, \x0, \y1	/* y1 = x0 | x2 */
	vandpd	\y2, \x1, \y3	/* y3 = x1 & y2 */
	vxorpd	\y0, \y1, \y1	/* y1 = y1 ^ y0 */
	vorpd	\x3, \y3, \y3	/* y3 = y3 | x3 */
	vorpd	\x2, \x3, \x3	/* x3 = x3 | x2 */
	vorpd	\x1, \x2, \x2	/* x2 = x2 | x1 */
	vandpd	\x0, \x2, \x2	/* x2 = x2 & x0 */
	vxorpd	\y3, \x2, \y0	/* y0 = x2 ^ y3 */
	vandpd	\y2, \y3, \y3	/* y3 = y3 & y2 */
	vnotpd	\y0, \y0		/* y0 = ~ y0 */
	vxorpd	\x2, \y3, \y3	/* y3 = y3 ^ x2 */
	vxorpd	\y1, \x0, \x0	/* x0 = x0 ^ y1 */
	vandpd	\y0, \x1, \x1	/* x1 = x1 & y0 */
	vxorpd	\x3, \y2, \y2	/* y2 = y2 ^ x3 */
	vxorpd	\x0, \y3, \y3	/* y3 = y3 ^ x0 */
	vxorpd	\x1, \y2, \y2	/* y2 = y2 ^ x1 */
	.else
	.error	"Invalid mode for IBOX6"
	.endif
	.endif
	.endif
	.endm

	.macro	SBOX7	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y0
	movl	\x1, \y3
	andl	\x2, \y0	/* y0 = x0 & x2 */
	movl	\x3, \y1
	orl		\y0, \y3	/* y3 = x1 | y0 */
	notl	\y1			/* y1 = ~ y1 */
	xorl	\x2, \y3	/* y3 = y3 ^ x2 */
	andl	\x0, \y1	/* y1 = y1 & x0 */
	xorl	\y1, \y3	/* y3 = y3 ^ y1 */
	movl	\x2, \y1
	movl	\x0, \y2
	orl		\y3, \y1	/* y1 = x2 | y3 */
	andl	\x1, \y2	/* y2 = x0 & x1 */
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	xorl	\y2, \x2	/* x2 = x2 ^ y2 */
	orl		\x3, \y2	/* y2 = y2 | x3 */
	xorl	\y2, \y1	/* y1 = y1 ^ y2 */
	movl	\x1, \y2
	notl	\x3			/* x3 = ~ x3 */
	orl		\y0, \y2	/* y2 = x1 | y0 */
	xorl	\y1, \y0	/* y0 = y0 ^ y1 */
	xorl	\y1, \x1	/* x1 = x1 ^ y1 */
	andl	\y3, \y2	/* y2 = y2 & y3 */
	orl		\x3, \y0	/* y0 = y0 | x3 */
	orl		\x1, \y2	/* y2 = y2 | x1 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	xorl	\x0, \y2	/* y2 = y2 ^ x0 */
	.else
	.if \m==64
	movq	\x0, \y0
	movq	\x1, \y3
	andq	\x2, \y0	/* y0 = x0 & x2 */
	movq	\x3, \y1
	orq		\y0, \y3	/* y3 = x1 | y0 */
	notq	\y1			/* y1 = ~ y1 */
	xorq	\x2, \y3	/* y3 = y3 ^ x2 */
	andq	\x0, \y1	/* y1 = y1 & x0 */
	xorq	\y1, \y3	/* y3 = y3 ^ y1 */
	movq	\x2, \y1
	movq	\x0, \y2
	orq		\y3, \y1	/* y1 = x2 | y3 */
	andq	\x1, \y2	/* y2 = x0 & x1 */
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	xorq	\y2, \x2	/* x2 = x2 ^ y2 */
	orq		\x3, \y2	/* y2 = y2 | x3 */
	xorq	\y2, \y1	/* y1 = y1 ^ y2 */
	movq	\x1, \y2
	notq	\x3			/* x3 = ~ x3 */
	orq		\y0, \y2	/* y2 = x1 | y0 */
	xorq	\y1, \y0	/* y0 = y0 ^ y1 */
	xorq	\y1, \x1	/* x1 = x1 ^ y1 */
	andq	\y3, \y2	/* y2 = y2 & y3 */
	orq		\x3, \y0	/* y0 = y0 | x3 */
	orq		\x1, \y2	/* y2 = y2 | x1 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	xorq	\x0, \y2	/* y2 = y2 ^ x0 */
	.else
	.if \m==128
	vandpd	\x2, \x0, \y0	/* y0 = x0 & x2 */
	vorpd	\y0, \x1, \y3	/* y3 = x1 | y0 */
	vnotpd	\x3, \y1		/* y1 = ~ x3 */
	vxorpd	\x2, \y3, \y3	/* y3 = y3 ^ x2 */
	vandpd	\x0, \y1, \y1	/* y1 = y1 & x0 */
	vxorpd	\y1, \y3, \y3	/* y3 = y3 ^ y1 */
	vorpd	\y3, \x2, \y1	/* y1 = x2 | y3 */
	vandpd	\x1, \x0, \y2	/* y2 = x0 & x1 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vxorpd	\y2, \x2, \x2	/* x2 = x2 ^ y2 */
	vorpd	\x3, \y2, \y2	/* y2 = y2 | x3 */
	vxorpd	\y2, \y1, \y1	/* y1 = y1 ^ y2 */
	vnotpd	\x3, \x3		/* x3 = ~ x3 */
	vorpd	\y0, \x1, \y2	/* y2 = x1 | y0 */
	vxorpd	\y1, \y0, \y0	/* y0 = y0 ^ y1 */
	vxorpd	\y1, \x1, \x1	/* x1 = x1 ^ y1 */
	vandpd	\y3, \y2, \y2	/* y2 = y2 & y3 */
	vorpd	\x3, \y0, \y0	/* y0 = y0 | x3 */
	vorpd	\x1, \y2, \y2	/* y2 = y2 | x1 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vxorpd	\x0, \y2, \y2	/* y2 = y2 ^ x0 */
	.else
	.error	"Invalid mode for SBOX7"
	.endif
	.endif
	.endif
	.endm

	.macro	IBOX7	x0,x1,x2,x3, y0,y1,y2,y3, m=32
	.if \m==32
	movl	\x0, \y3
	movl	\x1, \y2
	movl	\x0, \y1
	movl	\x0, \y0
	andl	\x1, \y3	/* y3 = x0 & x1 */
	xorl	\x3, \y2	/* y2 = x1 ^ x3 */
	orl		\x3, \y1	/* y1 = x0 | x3 */
	orl		\y3, \y2	/* y2 = y2 | y3 */
	andl	\x2, \y1	/* y1 = y1 & x2 */
	orl		\x1, \y0	/* y0 = x0 | x1 */
	orl		\x2, \y3	/* y3 = y3 | x2 */
	xorl	\y1, \y2	/* y2 = y2 ^ y1 */
	movl	\x3, \y1
	xorl	\y0, \y3	/* y3 = y3 ^ y0 */
	andl	\x3, \y0	/* y0 = y0 & x3 */
	xorl	\y3, \y1	/* y1 = x3 ^ y3 */
	xorl	\x1, \y0	/* y0 = y0 ^ x1 */
	notl	\y1			/* y1 = ~ y1 */
	orl		\y0, \y1	/* y1 = y1 | y0 */
	xorl	\x2, \y0	/* y0 = y0 ^ x2 */
	orl		\y1, \x3	/* x3 = x3 | y1 */
	xorl	\x0, \y1	/* y1 = y1 ^ x0 */
	xorl	\x3, \y0	/* y0 = y0 ^ x3 */
	.else
	.if \m==64
	movq	\x0, \y3
	movq	\x1, \y2
	movq	\x0, \y1
	movq	\x0, \y0
	andq	\x1, \y3	/* y3 = x0 & x1 */
	xorq	\x3, \y2	/* y2 = x1 ^ x3 */
	orq		\x3, \y1	/* y1 = x0 | x3 */
	orq		\y3, \y2	/* y2 = y2 | y3 */
	andq	\x2, \y1	/* y1 = y1 & x2 */
	orq		\x1, \y0	/* y0 = x0 | x1 */
	orq		\x2, \y3	/* y3 = y3 | x2 */
	xorq	\y1, \y2	/* y2 = y2 ^ y1 */
	movq	\x3, \y1
	xorq	\y0, \y3	/* y3 = y3 ^ y0 */
	andq	\x3, \y0	/* y0 = y0 & x3 */
	xorq	\y3, \y1	/* y1 = x3 ^ y3 */
	xorq	\x1, \y0	/* y0 = y0 ^ x1 */
	notq	\y1			/* y1 = ~ y1 */
	orq		\y0, \y1	/* y1 = y1 | y0 */
	xorq	\x2, \y0	/* y0 = y0 ^ x2 */
	orq		\y1, \x3	/* x3 = x3 | y1 */
	xorq	\x0, \y1	/* y1 = y1 ^ x0 */
	xorq	\x3, \y0	/* y0 = y0 ^ x3 */
	.else
	.if \m==128
	vandpd	\x1, \x0, \y3	/* y3 = x0 & x1 */
	vxorpd	\x3, \x1, \y2	/* y2 = x1 ^ x3 */
	vorpd	\x3, \x0, \y1	/* y1 = x0 | x3 */
	vorpd	\y3, \y2, \y2	/* y2 = y2 | y3 */
	vandpd	\x2, \y1, \y1	/* y1 = y1 & x2 */
	vorpd	\x1, \x0, \y0	/* y0 = x0 | x1 */
	vorpd	\x2, \y3, \y3	/* y3 = y3 | x2 */
	vxorpd	\y1, \y2, \y2	/* y2 = y2 ^ y1 */
	vxorpd	\y0, \y3, \y3	/* y3 = y3 ^ y0 */
	vandpd	\x3, \y0, \y0	/* y0 = y0 & x3 */
	vxorpd	\y3, \x3, \y1	/* y1 = x3 ^ y3 */
	vxorpd	\x1, \y0, \y0	/* y0 = y0 ^ x1 */
	vnotpd	\y1, \y1		/* y1 = ~ y1 */
	vorpd	\y0, \y1, \y1	/* y1 = y1 | y0 */
	vxorpd	\x2, \y0, \y0	/* y0 = y0 ^ x2 */
	vorpd	\y1, \x3, \x3	/* x3 = x3 | y1 */
	vxorpd	\x0, \y1, \y1	/* y1 = y1 ^ x0 */
	vxorpd	\x3, \y0, \y0	/* y0 = y0 ^ x3 */
	.else
	.error	"Invalid mode for IBOX7"
	.endif
	.endif
	.endif
	.endm


	.macro	LT	t0,t1,t2,t3, x0,x1,x2,x3, m=32
	.if \m==32
	roll	$13, \x0		/* x0 = ROL(x0, 13) */
	roll	$3, \x2			/* x2 = ROL(x2, 3) */
	xorl	\x0, \x1		/* x1 = x1 ^ x0 */
	leal	(,\x0,8), \t0	/* t0 = x0 << 3 */
	xorl	\x2, \x3		/* x3 = x3 ^ x2 */
	xorl	\x2, \x1		/* x1 = x1 ^ x0 ^ x2 */
	xorl	\t0, \x3		/* x3 = x3 ^ x2 ^ (x0 << 3) */
	roll	$1, \x1			/* x1 = ROL(x1, 1) */
	roll	$7, \x3			/* x3 = ROL(x3, 7) */
	movl	\x1, \t1		/* t1 = x1 */
	xorl	\x1, \x0		/* x0 = x0 ^ x1 */
	shll	$7, \x1			/* t1 = x1 << 7 */
	xorl	\x3, \x2		/* x3 = x3 ^ x2 */
	xorl	\x3, \x0		/* x0 = x0 ^ x1 ^ x3 */
	xorl	\t1, \x2		/* x2 = x2 ^ x3 ^ (x1 << 7) */
	roll	$5, \x0			/* x0 = ROL(x0, 5) */
	roll	$22, \x2		/* x2 = ROL(x2, 22) */
	.else
	.if \m==64
	movq	\x0, \t0
	leaq	(,\x2,8), \t2
	shrq	$19, \x0
	shrq	$29, \x2
	shlq	$13, \t0
	andq	%r8, \t2
	andq	LT64_mask_rsh19(%rip), \x0
	andq	LT64_mask_rsh29(%rip), \x2
	andq	LT64_mask_lsh13(%rip), \t0
	orq		\t2, \x2
	orq		\t0, \x0
	xorq	\x2, \x1
	leaq	(,\x0,8), \t2
	xorq	\x2, \x3
	andq	%r8, \t2
	xorq	\x0, \x1
	xorq	\t2, \x3
	leaq	(\x1,\x1), \t1
	movq	\x3, \t3
	shrq	$31, \x1
	shrq	$25, \x3
	shlq	$7, \t3
	andq	LT64_mask_lsh1(%rip), \t1
	andq	LT64_mask_rsh31(%rip), \x1
	andq	LT64_mask_rsh25(%rip), \x3
	andq	%r9, \t3
	orq		\t1, \x1
	orq		\t3, \x3
	movq	\x1, \t2
	xorq	\x3, \x2
	andq	%r9, \t2
	xorq	\x1, \x0
	xorq	\t2, \x2
	movq	\x0, \t0
	movq	\x2, \t2
	shrq	$27, \x0
	shrq	$10, \x2
	shlq	$5, \t0
	shlq	$22, \t2
	andq	LT64_mask_rsh27(%rip), \x0
	andq	LT64_mask_rsh10(%rip), \x2
	andq	LT64_mask_lsh5(%rip), \t0
	andq	LT64_mask_lsh22(%rip), \t2
	orq		\t0, \x0
	orq		\t2, \x2
	.else
	.if \m==128
	vpsrld	$19, \x0, %xmm8
	vpsrld	$29, \x2, %xmm9
	vpslld	$13, \x0, \x0
	vpslld	$3, \x2, \x2
	vorpd	%xmm8, \x0, \x0
	vorpd	%xmm9, \x2, \x2
	vpslld	$3, \x0, %xmm10
	vxorpd	\x0, \x1, \x1
	vxorpd	%xmm10, \x3, \x3
	vxorpd	\x2, \x1, \x1
	vxorpd	\x2, \x3, \x3
	vpsrld	$31, \x1, %xmm8
	vpsrld	$25, \x3, %xmm9
	vpslld	$1, \x1, \x1
	vpslld	$7, \x3, \x3
	vorpd	%xmm8, \x1, \x1
	vorpd	%xmm9, \x3, \x3
	vpslld	$7, \x1, %xmm10
	vxorpd	\x1, \x0, \x0
	vxorpd	%xmm10, \x2, \x2
	vxorpd	\x3, \x0, \x0
	vxorpd	\x3, \x2, \x2
	vpsrld	$27, \x0, %xmm8
	vpsrld	$10, \x2, %xmm9
	vpslld	$5, \x0, \x0
	vpslld	$22, \x2, \x2
	vorpd	%xmm8, \x0, \x0
	vorpd	%xmm9, \x2, \x2
	.else
	.error	"Invalid mode for LT"
	.endif
	.endif
	.endif
	.endm

	.macro	ILT	x0,x1,x2,x3, t0,t1,t2,t3, m=32
	.if \m==32
	roll	$10, \x2		/* x2 = ROR(x2, 22) */
	roll	$27, \x0		/* x0 = ROR(x0, 5) */
	movl	\x1, \t1		/* t1 = x1 */
	xorl	\x3, \x2		/* x2 = x2 ^ x3 */
	shll	$7, \t1			/* t1 = x1 << 7 */
	xorl	\x1, \x0		/* x0 = x0 ^ x1 */
	xorl	\t1, \x2		/* x2 = x2 ^ x3 ^ (x1 << 7) */
	xorl	\x3, \x0		/* x0 = x0 ^ x1 ^ x3 */
	roll	$25, \x3		/* x3 = ROR(x3, 7) */
	roll	$31, \x1		/* x1 = ROR(x1, 1) */
	leal	(,\x0,8), \t0	/* t0 = x0 << 3 */
	xorl	\x2, \x3		/* x3 = x3 ^ x2 */
	xorl	\x0, \x1		/* x1 = x1 ^ x0 */
	xorl	\t0, \x3		/* x3 = x3 ^ x2 ^ (x0 << 3) */
	xorl	\x2, \x1		/* x1 = x1 ^ x0 ^ x2 */
	roll	$29, \x2		/* x2 = ROR(x2, 3) */
	roll	$19, \x0		/* x0 = ROR(x0, 13) */
	.else
	.if \m==64
	movq	\x2, \t2
	movq	\x0, \t0
	movq	\x1, \t1
	shrq	$22, \x2
	shrq	$5, \x0
	shlq	$10, \t2
	shlq	$27, \t0
	shlq	$7, \t1
	andq	LT64_mask_rsh22(%rip), \x2
	andq	LT64_mask_rsh5(%rip), \x0
	andq	LT64_mask_lsh10(%rip), \t2
	andq	LT64_mask_lsh27(%rip), \t0
	andq	LT64_mask_lsh7(%rip), \t1
	orq		\t2, \x2
	orq		\t0, \x0
	xorq	\x3, \x2
	xorq	\x1, \x0
	xorq	\t1, \x2
	xorq	\x3, \x0
	movq	\x3, \t3
	movq	\x1, \t1
	shrq	$7, \x3
	shrq	$1, \x1
	shlq	$25, \t3
	shlq	$31, \t1
	leaq	(,\x0,8), \t0
	andq	LT64_mask_rsh7(%rip), \x3
	andq	LT64_mask_rsh1(%rip), \x1
	andq	LT64_mask_lsh25(%rip), \t3
	andq	LT64_mask_lsh31(%rip), \t1
	andq	LT64_mask_lsh3(%rip), \t0
	orq		\t3, \x3
	orq		\t1, \x1
	xorq	\x2, \x3
	xorq	\x0, \x1
	xorq	\t0, \x3
	xorq	\x2, \x1
	movq	\x2, \t2
	movq	\x0, \t0
	shrq	$3, \x2
	shrq	$13, \x0
	shlq	$29, \t2
	shlq	$19, \t0
	andq	LT64_mask_rsh3(%rip), \x2
	andq	LT64_mask_rsh13(%rip), \x0
	andq	LT64_mask_lsh29(%rip), \t2
	andq	LT64_mask_lsh19(%rip), \t0
	orq		\t2, \x2
	orq		\t0, \x0
	.else
	.if \m==128
	vpslld	$27, \x0, %xmm8
	vpslld	$10, \x2, %xmm9
	vpslld	$7, \x1, %xmm10
	vpsrld	$5, \x0, \x0
	vpsrld	$22, \x2, \x2
	vorpd	%xmm8, \x0, \x0
	vorpd	%xmm9, \x2, \x2
	vxorpd	\x1, \x0, \x0
	vxorpd	%xmm10, \x2, \x2
	vxorpd	\x3, \x0, \x0
	vxorpd	\x3, \x2, \x2
	vpslld	$31, \x1, %xmm8
	vpslld	$25, \x3, %xmm9
	vpslld	$3, \x0, %xmm10
	vpsrld	$1, \x1, \x1
	vpsrld	$7, \x3, \x3
	vorpd	%xmm8, \x1, \x1
	vorpd	%xmm9, \x3, \x3
	vxorpd	\x0, \x1, \x1
	vxorpd	%xmm10, \x3, \x3
	vxorpd	\x2, \x1, \x1
	vxorpd	\x2, \x3, \x3
	vpslld	$29, \x0, %xmm8
	vpslld	$19, \x2, %xmm9
	vpsrld	$13, \x0, \x0
	vpsrld	$3, \x2, \x2
	vorpd	%xmm8, \x0, \x0
	vorpd	%xmm9, \x2, \x2
	vprrld	13, %xmm8,  \x0
	vprrld	3,  %xmm9, \x2
	.else
	.error	"Invalid mode for ILT"
	.endif
	.endif
	.endif
	.endm


	.macro	ADDKEY	x0,x1,x2,x3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	.if \m==32
	xorl	0x0+\ko(\kr), \x0
	xorl	0x4+\ko(\kr), \x1
	xorl	0x8+\ko(\kr), \x2
	xorl	0xC+\ko(\kr), \x3
	.else
	.if \m==64
	movl	0x0+\ko(\kr), \t0d
	movl	0x4+\ko(\kr), \t1d
	movq	\t0, \t2
	movq	\t1, \t3
	shlq	$32, \t0
	shlq	$32, \t1
	orq		\t0, \t2
	orq		\t1, \t3
	xorq	\t2, \x0
	xorq	\t3, \x1
	movl	0x8+\ko(\kr), \t0d
	movl	0xC+\ko(\kr), \t1d
	movq	\t0, \t2
	movq	\t1, \t3
	shlq	$32, \t0
	shlq	$32, \t1
	orq		\t0, \t2
	orq		\t1, \t3
	xorq	\t2, \x2
	xorq	\t3, \x3
	.else
	.if \m==128
	vmovaps	\ko(\kr), %xmm11
	vpshufd	$0x00, %xmm11, %xmm8
	vpshufd	$0x55, %xmm11, %xmm9
	vpshufd	$0xAA, %xmm11, %xmm10
	vpshufd	$0xFF, %xmm11, %xmm11
	vxorpd	%xmm8,  \x0, \x0
	vxorpd	%xmm9,  \x1, \x1
	vxorpd	%xmm10, \x2, \x2
	vxorpd	%xmm11, \x3, \x3
	.else
	.error	"Invalid mode for ADDKEY"
	.endif
	.endif
	.endif
	.endm


	.macro	ENC0 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX0	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC1 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX1	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC2 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX2	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC3 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX3	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC4 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX4	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC5 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX5	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC6 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX6	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm

	.macro	ENC7 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ADDKEY	\x0,\x1,\x2,\x3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	SBOX7	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	LT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	.endm


	.macro	DEC0 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX0	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC1 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX1	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC2 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX2	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC3 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX3	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC4 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX4	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC5 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX5	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC6 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX6	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

	.macro	DEC7 x0,x1,x2,x3, y0,y1,y2,y3, kr,ko, m=32, t0d=0,t1d=0,t0=0,t1=0,t2=0,t3=0
	ILT		\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	IBOX7	\x0,\x1,\x2,\x3, \y0,\y1,\y2,\y3, \m
	ADDKEY	\y0,\y1,\y2,\y3, \kr,\ko, \m, \t0d,\t1d,\t0,\t1,\t2,\t3
	.endm

