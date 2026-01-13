	.file	"/home/statim/lace/samples/mem.lace"
	.data
	.align	8
	.type	border, @object
	.size	border, 8
border:
	.quad 0x0
	.data
	.align	8
	.type	head, @object
	.size	head, 8
head:
	.quad 0x0
	.text
	.type	extend_heap, @function
extend_heap:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L0_0:
	movq	%rdi, -8(%rbp)
	movq	border, %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L0_1
	jmp	.L0_4
.L0_1:
	call	fetch_border
	movq	border, %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L0_2
	jmp	.L0_3
.L0_2:
	movq	$-1, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.L0_3:
	jmp	.L0_4
.L0_4:
	movq	-8(%rbp), %rcx
	cmpq	$0, %rcx
	seteb	%cl
	cmpb	$0, %cl
	jne	.L0_5
	jmp	.L0_6
.L0_5:
	movq	border, %rcx
	movq	%rcx, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.L0_6:
	movq	border, %rcx
	movq	%rcx, -24(%rbp)
	movq	border, %rcx
	movq	-8(%rbp), %rdx
	addq	%rdx, %rcx
	movq	%rcx, -16(%rbp)
	movq	-16(%rbp), %rcx
	movq	%rcx, %rdi
	pushq	%rax
	call	brk
	popq	%rax
	movq	-24(%rbp), %rcx
	cmpq	%rax, %rcx
	seteb	%al
	cmpb	$0, %al
	jne	.L0_7
	jmp	.L0_8
.L0_7:
	movq	$-1, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.L0_8:
	movq	-16(%rbp), %rcx
	movq	%rcx, border
	movq	border, %rcx
	movq	%rcx, %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE0:
	.size	extend_heap, .-extend_heap

	.text
	.type	fetch_border, @function
fetch_border:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L1_0:
	movq	$0, %rdi
	call	brk
	movq	%rax, border
	movq	border, %rax
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE1:
	.size	fetch_border, .-fetch_border

	.text
	.type	find_free_space, @function
find_free_space:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L2_0:
	movq	%rdi, -16(%rbp)
	movq	%rsi, -24(%rbp)
	movq	border, %rax
	movq	%rax, -8(%rbp)
	jmp	.L2_1
.L2_1:
	movq	-8(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L2_7
	jmp	.L2_2
.L2_2:
	movq	-8(%rbp), %rax
	addq	$16, %rax
	movb	(%rax), %al
	cmpb	$0, %al
	setneb	%al
	cmpb	$0, %al
	jne	.L2_3
	movb	$0, %al
	jmp	.L2_4
.L2_3:
	movq	-8(%rbp), %rcx
	movq	(%rcx), %rcx
	movq	-24(%rbp), %rdx
	cmpq	%rcx, %rdx
	setbeb	%cl
	movb	%cl, %al
	jmp	.L2_4
.L2_4:
	cmpb	$0, %al
	jne	.L2_5
	jmp	.L2_6
.L2_5:
	jmp	.L2_7
.L2_6:
	movq	-16(%rbp), %rax
	movq	-8(%rbp), %rcx
	movq	%rcx, (%rax)
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	jmp	.L2_1
.L2_7:
	movq	-8(%rbp), %rax
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE2:
	.size	find_free_space, .-find_free_space

	.text
	.type	get_block_pointer, @function
get_block_pointer:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$16, %rsp
.L3_0:
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rax
	add	$-24, %rax
	addq	$16, %rsp
	popq	%rbp
	ret
.LFE3:
	.size	get_block_pointer, .-get_block_pointer

	.text
	.global	mem_alloc
	.type	mem_alloc, @function
mem_alloc:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
.L4_0:
	movq	%rdi, -32(%rbp)
	movq	$0, -16(%rbp)
	movq	-32(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-32(%rbp), %rax
	movq	$16, %rcx
	movl	$0, %edx
	divq	%rcx
	mov	%rdx, %rcx
	cmpq	$0, %rcx
	setneb	%cl
	cmpb	$0, %cl
	jne	.L4_1
	jmp	.L4_2
.L4_1:
	movq	-8(%rbp), %rcx
	movq	%rcx, -8(%rbp)
	addq	$16, %rcx
	movq	-32(%rbp), %rdx
	movq	%rdx, %rax
	movq	$16, %rsi
	movl	$0, %edx
	divq	%rsi
	mov	%rdx, %rsi
	subq	%rsi, %rcx
	jmp	.L4_2
.L4_2:
	movq	head, %rcx
	cmpq	$0, %rcx
	setneb	%cl
	cmpb	$0, %cl
	jne	.L4_3
	jmp	.L4_6
.L4_3:
	movq	-8(%rbp), %rcx
	movq	%rcx, %rsi
	movq	$0, %rdi
	pushq	%rax
	call	request_space
	popq	%rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L4_4
	jmp	.L4_5
.L4_4:
	movq	$0, %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.L4_5:
	movq	-16(%rbp), %rcx
	movq	%rcx, head
	jmp	.L4_12
.L4_6:
	movq	head, %rcx
	movq	%rcx, -24(%rbp)
	movq	-8(%rbp), %rcx
	movq	%rcx, %rsi
	leaq	-24(%rbp), %rdi
	pushq	%rax
	call	find_free_space
	popq	%rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L4_7
	jmp	.L4_8
.L4_7:
	movq	-16(%rbp), %rax
	addq	$16, %rax
	movb	$0, (%rax)
	jmp	.L4_11
.L4_8:
	movq	-24(%rbp), %rax
	movq	-8(%rbp), %rcx
	movq	%rcx, %rsi
	movq	%rax, %rdi
	call	request_space
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L4_9
	jmp	.L4_10
.L4_9:
	movq	$0, %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.L4_10:
	jmp	.L4_11
.L4_11:
	jmp	.L4_12
.L4_12:
	movq	-16(%rbp), %rcx
	add	$24, %rcx
	movq	%rcx, %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.LFE4:
	.size	mem_alloc, .-mem_alloc

	.text
	.global	mem_copy
	.type	mem_copy, @function
mem_copy:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
.L5_0:
	movq	%rdi, -16(%rbp)
	movq	%rsi, -40(%rbp)
	movq	%rdx, -24(%rbp)
	movq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-40(%rbp), %rax
	movq	%rax, -32(%rbp)
	jmp	.L5_1
.L5_1:
	movq	-24(%rbp), %rax
	cmpq	$0, %rax
	seteb	%al
	cmpb	$0, %al
	jne	.L5_3
	jmp	.L5_2
.L5_2:
	movq	-8(%rbp), %rax
	movq	-24(%rbp), %rcx
	addq	%rcx, %rax
	movq	-32(%rbp), %rcx
	movq	-24(%rbp), %rdx
	addq	%rdx, %rcx
	movb	(%rcx), %cl
	movb	%cl, (%rax)
	movq	-24(%rbp), %rax
	movq	%rax, -24(%rbp)
	subq	$1, %rax
	jmp	.L5_1
.L5_3:
	addq	$48, %rsp
	popq	%rbp
	ret
.LFE5:
	.size	mem_copy, .-mem_copy

	.text
	.global	mem_free
	.type	mem_free, @function
mem_free:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$32, %rsp
.L6_0:
	movq	%rdi, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L6_1
	jmp	.L6_2
.L6_1:
	addq	$32, %rsp
	popq	%rbp
	ret
.L6_2:
	movq	-16(%rbp), %rax
	movq	%rax, %rdi
	call	get_block_pointer
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	addq	$16, %rax
	movb	$1, (%rax)
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L6_3
	jmp	.L6_6
.L6_3:
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	addq	$16, %rax
	movb	(%rax), %al
	cmpb	$0, %al
	setneb	%al
	cmpb	$0, %al
	jne	.L6_4
	jmp	.L6_5
.L6_4:
	movq	-8(%rbp), %rax
	movq	-8(%rbp), %rcx
	movq	(%rcx), %rcx
	movq	%rcx, (%rax)
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	(%rax), %rax
	movq	(%rax), %rax
	addq	%rcx, %rax
	addq	$24, %rax
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	-8(%rbp), %rcx
	addq	$8, %rcx
	movq	(%rcx), %rcx
	addq	$8, %rcx
	movq	(%rcx), %rcx
	movq	%rcx, (%rax)
	jmp	.L6_5
.L6_5:
	jmp	.L6_6
.L6_6:
	addq	$32, %rsp
	popq	%rbp
	ret
.LFE6:
	.size	mem_free, .-mem_free

	.text
	.global	mem_realloc
	.type	mem_realloc, @function
mem_realloc:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$48, %rsp
.L7_0:
	movq	%rdi, -24(%rbp)
	movq	%rsi, -32(%rbp)
	movq	-24(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L7_1
	jmp	.L7_2
.L7_1:
	movq	-32(%rbp), %rax
	movq	%rax, %rdi
	call	mem_alloc
	addq	$48, %rsp
	popq	%rbp
	ret
.L7_2:
	movq	-24(%rbp), %rcx
	movq	%rcx, %rdi
	pushq	%rax
	call	get_block_pointer
	popq	%rax
	movq	%rax, -8(%rbp)
	movq	-8(%rbp), %rax
	movq	(%rax), %rax
	movq	-32(%rbp), %rcx
	cmpq	%rax, %rcx
	setbeb	%al
	cmpb	$0, %al
	jne	.L7_3
	jmp	.L7_4
.L7_3:
	movq	-24(%rbp), %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.L7_4:
	movq	-32(%rbp), %rcx
	movq	%rcx, %rdi
	pushq	%rax
	call	mem_alloc
	popq	%rax
	movq	%rax, -16(%rbp)
	movq	-16(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L7_5
	jmp	.L7_6
.L7_5:
	movq	$0, %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.L7_6:
	movq	-16(%rbp), %rcx
	movq	-24(%rbp), %rsi
	movq	-8(%rbp), %rdx
	movq	(%rdx), %rdx
	movq	%rcx, %rdi
	pushq	%rax
	call	mem_copy
	popq	%rax
	movq	-24(%rbp), %rcx
	movq	%rcx, %rdi
	pushq	%rax
	call	mem_free
	popq	%rax
	movq	-16(%rbp), %rcx
	movq	%rcx, %rax
	addq	$48, %rsp
	popq	%rbp
	ret
.LFE7:
	.size	mem_realloc, .-mem_realloc

	.section	.rodata.cst8,"aM", @progbits, 8
	.p2align	3,0x0
.LCPI8_0:
	.string "1\n"
	.text
	.type	request_space, @function
request_space:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$64, %rsp
.L8_0:
	movq	%rdi, -24(%rbp)
	movq	%rsi, -40(%rbp)
	movq	-40(%rbp), %rax
	addq	$24, %rax
	movq	%rax, -48(%rbp)
	movq	$0, -8(%rbp)
	movq	$0, -32(%rbp)
	leaq	.LCPI8_0(%rip), %rax
	movq	$2, %rdx
	movq	%rax, %rsi
	movq	$1, %rdi
	call	write
	movq	$0, %rdi
	call	extend_heap
	movq	%rax, -16(%rbp)
	leaq	.LCPI8_0(%rip), %rax
	movq	$2, %rdx
	movq	%rax, %rsi
	movq	$1, %rdi
	call	write
	movq	-16(%rbp), %rax
	movq	%rax, -8(%rbp)
	movq	-48(%rbp), %rax
	movq	%rax, %rdi
	call	extend_heap
	movq	%rax, -32(%rbp)
	leaq	.LCPI8_0(%rip), %rax
	movq	$2, %rdx
	movq	%rax, %rsi
	movq	$1, %rdi
	call	write
	movq	-24(%rbp), %rax
	cmpq	$0, %rax
	setneb	%al
	cmpb	$0, %al
	jne	.L8_1
	jmp	.L8_2
.L8_1:
	movq	-24(%rbp), %rax
	addq	$8, %rax
	movq	-8(%rbp), %rcx
	movq	%rcx, (%rax)
	jmp	.L8_2
.L8_2:
	leaq	.LCPI8_0(%rip), %rax
	movq	$2, %rdx
	movq	%rax, %rsi
	movq	$1, %rdi
	call	write
	movq	-8(%rbp), %rax
	movq	-40(%rbp), %rcx
	movq	%rcx, (%rax)
	movq	-8(%rbp), %rax
	addq	$8, %rax
	movq	$0, (%rax)
	movq	-8(%rbp), %rax
	addq	$16, %rax
	movb	$0, (%rax)
	movq	-8(%rbp), %rax
	addq	$64, %rsp
	popq	%rbp
	ret
.LFE8:
	.size	request_space, .-request_space

