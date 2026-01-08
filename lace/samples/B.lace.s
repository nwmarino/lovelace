	.file	"/home/statim/lace/samples/B.lace"
	.text
	.global	bar
	.type	bar, @function
bar:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L0_0:
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	bar, .-bar

