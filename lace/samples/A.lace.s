	.file	"/home/statim/lace/samples/A.lace"
	.text
	.global	main
	.type	main, @function
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L0_0:
	movq	$42, %rdi
	call	exit
	movq	$5, %rax
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	main, .-main

