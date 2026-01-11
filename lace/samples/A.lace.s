	.file	"A.lace"
	.text
	.p2align	4
	.type	strlen,@function
strlen:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, -8(%rbp)
	jmp	.LBB0_1
.LBB0_1:
	movq	-8(%rbp), %rax
	cmpb	$0, (%rax,%rcx)
	je	.LBB0_3
	jmp	.LBB0_1
.LBB0_3:
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end0:
	.size	strlen, .Lfunc_end0-strlen
	.cfi_endproc

	.p2align	4
	.type	print,@function
print:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	callq	strlen
	movq	-8(%rbp), %rsi
	movq	%rax, %rdx
	movl	$1, %edi
	callq	write@PLT
	addq	$16, %rsp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end1:
	.size	print, .Lfunc_end1-print
	.cfi_endproc

	.globl	main
	.p2align	4
	.type	main,@function
main:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	leaq	.L__unnamed_1(%rip), %rsi
	movl	$4, %edx
	callq	write@PLT
	callq	close@PLT
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc

	.type	.L__unnamed_1,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L__unnamed_1:
	.asciz	"hey!"
	.size	.L__unnamed_1, 5

	.type	.L__unnamed_2,@object
.L__unnamed_2:
	.asciz	"hey!\n"
	.size	.L__unnamed_2, 6

	.section	".note.GNU-stack","",@progbits
