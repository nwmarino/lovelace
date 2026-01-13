	.file	"/home/statim/lace/samples/A.lace"
	.data
	.align	8
	.type	glob, @object
	.size	glob, 8
glob:
	.quad 5
	.text
	.global	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L0_0:
	movq	$5, glob
	leaq	glob, %rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	$42, (%rax)
	movq	glob, %rax
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	main, .-main

	.text
	.type	print, @function
print:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L1_0:
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rcx
	movq	-8(%rbp), %rax
	movq	%rax, %rdi
	pushq	%rcx
	call	strlen
	popq	%rcx
	movq	%rax, %rdx
	movq	%rcx, %rsi
	movq	$1, %rdi
	call	write
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE1:
	.size	print, .-print

	.text
	.type	strlen, @function
strlen:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L2_0:
	movq	%rdi, -16(%rbp)
	movq	$0, -8(%rbp)
	jmp	.L2_1
.L2_1:
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rcx
	addq	%rcx, %rax
	movb	(%rax), %al
	cmpb	$0, %al
	seteb	%al
	cmpb	$0, %al
	jne	.L2_3
	jmp	.L2_2
.L2_2:
	movq	-8(%rbp), %rax
	movq	%rax, -8(%rbp)
	addq	$1, %rax
	jmp	.L2_1
.L2_3:
	movq	-8(%rbp), %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE2:
	.size	strlen, .-strlen

