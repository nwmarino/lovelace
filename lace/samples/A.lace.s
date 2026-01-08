	.file	"/home/nwm/statim/lace/samples/A.lace"
	.text
	.global	foo
	.type	foo, @function
foo:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L0_0:
.LFE0:
	.size	foo, .-foo

	.text
	.global	gt5
	.type	gt5, @function
gt5:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L1_0:
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	cmpq	$5, %rax
	setgb	%al
	cmpb	$0, %al
	jne	.L1_1
	jmp	.L1_2
.L1_1:
	movb	$1, %al
	addq	$16, %rsp
	popq	%rbp
	ret
.L1_2:
	movb	$0, %al
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE1:
	.size	gt5, .-gt5

