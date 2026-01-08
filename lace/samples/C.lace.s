	.file	"/home/nwm/statim/lace/samples/C.lace"
	.text
	.global	iadd
	.type	iadd, @function
iadd:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L0_0:
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rcx
	addq	%rax, %rcx
	movq	%rcx, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	iadd, .-iadd

	.text
	.global	isub
	.type	isub, @function
isub:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L1_0:
	movl	%edi, -4(%rbp)
	movl	%esi, -8(%rbp)
	movl	-4(%rbp), %eax
	movl	-8(%rbp), %ecx
	subl	%ecx, %eax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE1:
	.size	isub, .-isub

	.section	.rodata.cst8,"aM", @progbits, 8
	.p2align	3,0x0
.LCPI2_0:
	.string "hey!"
	.text
	.global	mango
	.type	mango, @function
mango:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L2_0:
	call	foo
	movq	%rax, -8(%rbp)
	lea	.LCPI2_0(%rip), %rax
	movq	%rax, -16(%rbp)
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE2:
	.size	mango, .-mango

	.text
	.global	sdiv
	.type	sdiv, @function
sdiv:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L3_0:
	movb	%dil, -1(%rbp)
	movb	%sil, -2(%rbp)
	movb	-1(%rbp), %al
	movb	-2(%rbp), %cl
	cqo	
	idivb	%cl
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE3:
	.size	sdiv, .-sdiv

	.text
	.global	smul
	.type	smul, @function
smul:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L4_0:
	movw	%di, -2(%rbp)
	movw	%si, -4(%rbp)
	movw	-2(%rbp), %ax
	movw	-4(%rbp), %cx
	imulw	%cx, %ax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE4:
	.size	smul, .-smul

	.text
	.global	srem
	.type	srem, @function
srem:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L5_0:
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rcx
	cqo	
	idivq	%rcx
	mov	%rdx, %rcx
	movq	%rcx, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE5:
	.size	srem, .-srem

	.text
	.global	udiv
	.type	udiv, @function
udiv:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L6_0:
	movb	%dil, -1(%rbp)
	movb	%sil, -2(%rbp)
	movb	-1(%rbp), %al
	movb	-2(%rbp), %cl
	movl	$0, %edx
	divb	%cl
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE6:
	.size	udiv, .-udiv

	.text
	.global	umul
	.type	umul, @function
umul:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L7_0:
	movw	%di, -2(%rbp)
	movw	%si, -4(%rbp)
	movw	-2(%rbp), %ax
	movw	-4(%rbp), %cx
	imulw	%cx, %ax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE7:
	.size	umul, .-umul

	.text
	.global	urem
	.type	urem, @function
urem:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L8_0:
	movq	%rdi, -8(%rbp)
	movq	%rsi, -16(%rbp)
	movq	-8(%rbp), %rax
	movq	-16(%rbp), %rcx
	movl	$0, %edx
	divq	%rcx
	mov	%rdx, %rcx
	movq	%rcx, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE8:
	.size	urem, .-urem

