	.file	"mem.lace"
	.text
	.globl	mem_copy
	.p2align	4
	.type	mem_copy,@function
mem_copy:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdx, -8(%rbp)
	jmp	.LBB0_1
.LBB0_1:
	movq	-8(%rbp), %rax
	cmpq	$0, %rax
	je	.LBB0_3
	movq	-8(%rbp), %rcx
	movb	(%rax,%rcx), %dl
	movb	%dl, (%rax,%rcx)
	jmp	.LBB0_1
.LBB0_3:
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end0:
	.size	mem_copy, .Lfunc_end0-mem_copy
	.cfi_endproc

	.globl	mem_alloc
	.p2align	4
	.type	mem_alloc,@function
mem_alloc:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$32, %rsp
	andq	$15, %rdi
	cmpq	$0, %rdi
	je	.LBB1_2
.LBB1_2:
	cmpq	$0, head(%rip)
	je	.LBB1_6
	xorl	%eax, %eax
	movl	%eax, %edi
	callq	request_space
	movq	%rax, %rcx
	movq	%rcx, -24(%rbp)
	xorl	%eax, %eax
	cmpq	$0, %rcx
	movq	%rax, -16(%rbp)
	je	.LBB1_5
.LBB1_4:
	movq	-16(%rbp), %rax
	addq	$32, %rsp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.LBB1_5:
	.cfi_def_cfa %rbp, 16
	movq	-24(%rbp), %rax
	movq	%rax, head(%rip)
	movq	%rax, -32(%rbp)
	jmp	.LBB1_8
.LBB1_6:
	leaq	-8(%rbp), %rdi
	callq	find_free_space
	cmpq	$0, %rax
	movq	%rax, -32(%rbp)
	jne	.LBB1_8
	movq	-8(%rbp), %rdi
	callq	request_space
	xorl	%ecx, %ecx
	cmpq	$0, %rax
	movq	%rcx, -16(%rbp)
	movq	%rax, -32(%rbp)
	jne	.LBB1_4
.LBB1_8:
	movq	-32(%rbp), %rax
	addq	$8, %rax
	movq	%rax, -16(%rbp)
	jmp	.LBB1_4
.Lfunc_end1:
	.size	mem_alloc, .Lfunc_end1-mem_alloc
	.cfi_endproc

	.globl	mem_free
	.p2align	4
	.type	mem_free,@function
mem_free:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	xorl	%eax, %eax
	testb	$1, %al
	jne	.LBB2_2
.LBB2_1:
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.LBB2_2:
	.cfi_def_cfa %rbp, 16
	jmp	.LBB2_1
.Lfunc_end2:
	.size	mem_free, .Lfunc_end2-mem_free
	.cfi_endproc

	.globl	mem_realloc
	.p2align	4
	.type	mem_realloc,@function
mem_realloc:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$32, %rsp
	movq	%rdi, -16(%rbp)
	movq	%rsi, -8(%rbp)
	cmpq	$0, %rdi
	jne	.LBB3_2
	jmp	.LBB3_3
.LBB3_1:
	movq	-24(%rbp), %rax
	addq	$32, %rsp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.LBB3_2:
	.cfi_def_cfa %rbp, 16
	movq	-8(%rbp), %rdi
	callq	mem_alloc@PLT
	movq	%rax, -24(%rbp)
	jmp	.LBB3_1
.LBB3_3:
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rdx
	cmpq	%rdx, %rcx
	movq	%rax, -24(%rbp)
	jae	.LBB3_1
	xorl	%eax, %eax
	cmpq	$0, %rcx
	movq	%rax, -24(%rbp)
	jne	.LBB3_1
	movq	-16(%rbp), %rsi
	callq	mem_copy@PLT
	movq	-16(%rbp), %rdi
	callq	mem_free@PLT
	jmp	.LBB3_1
.Lfunc_end3:
	.size	mem_realloc, .Lfunc_end3-mem_realloc
	.cfi_endproc

	.p2align	4
	.type	fetch_border,@function
fetch_border:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	xorl	%eax, %eax
	movl	%eax, %edi
	callq	brk@PLT
	movq	%rax, border(%rip)
	movq	border(%rip), %rax
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end4:
	.size	fetch_border, .Lfunc_end4-fetch_border
	.cfi_endproc

	.p2align	4
	.type	get_block_pointer,@function
get_block_pointer:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, %rax
	addq	$-8, %rax
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end5:
	.size	get_block_pointer, .Lfunc_end5-get_block_pointer
	.cfi_endproc

	.p2align	4
	.type	extend_heap,@function
extend_heap:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movq	%rdi, -8(%rbp)
	cmpq	$0, border(%rip)
	je	.LBB6_3
	callq	fetch_border
	movq	$-1, %rax
	cmpq	$0, border(%rip)
	movq	%rax, -16(%rbp)
	je	.LBB6_3
.LBB6_2:
	movq	-16(%rbp), %rax
	addq	$16, %rsp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.LBB6_3:
	.cfi_def_cfa %rbp, 16
	movq	-8(%rbp), %rax
	cmpq	$0, %rax
	jne	.LBB6_5
	movq	border(%rip), %rax
	movq	%rax, -16(%rbp)
	jmp	.LBB6_2
.LBB6_5:
	callq	brk@PLT
	movq	%rax, %rcx
	movq	$-1, %rax
	cmpq	%rdx, %rcx
	movq	%rax, -16(%rbp)
	je	.LBB6_2
	movq	%rax, border(%rip)
	movq	border(%rip), %rax
	movq	%rax, -16(%rbp)
	jmp	.LBB6_2
.Lfunc_end6:
	.size	extend_heap, .Lfunc_end6-extend_heap
	.cfi_endproc

	.p2align	4
	.type	find_free_space,@function
find_free_space:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, -8(%rbp)
	jmp	.LBB7_1
.LBB7_1:
	cmpq	$0, %rax
	jne	.LBB7_4
	movb	$1, %al
	testb	$1, %al
	jne	.LBB7_4
	movq	-8(%rbp), %rax
	movq	%rcx, (%rax)
	jmp	.LBB7_1
.LBB7_4:
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end7:
	.size	find_free_space, .Lfunc_end7-find_free_space
	.cfi_endproc

	.p2align	4
	.type	request_space,@function
request_space:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$16, %rsp
	movq	%rsi, -8(%rbp)
	callq	extend_heap
	movq	-8(%rbp), %rax
	addq	$16, %rsp
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end8:
	.size	request_space, .Lfunc_end8-request_space
	.cfi_endproc

	.type	border,@object
	.local	border
	.comm	border,8,8
	.type	head,@object
	.local	head
	.comm	head,8,8
	.section	".note.GNU-stack","",@progbits
