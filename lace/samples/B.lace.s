	.text
	.global	and
	.type	and, @function
and:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L0_0:
	movb	%dil, -1(%rbp)
	movb	%sil, -2(%rbp)
	movb	-1(%rbp), %al
	cmpb	$0, %al
	setneb	%al
	cmpb	$0, %al
	jne	.L0_1
	movb	$0, %al
	jmp	.L0_2
.L0_1:
	movb	-2(%rbp), %cl
	cmpb	$0, %cl
	setneb	%cl
	movb	%cl, %al
	jmp	.L0_2
.L0_2:
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	and, .-and


	.global	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L2_0:
	movb	$1, %sil
	movb	$1, %dil
	call	and
	movsx	%al, %rax
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE2:
	.size	main, .-main

