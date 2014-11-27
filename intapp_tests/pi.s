	.file	"pi.c"
	.text
	.globl	get_random
	.type	get_random, @function
get_random:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	movl	8(%ebp), %eax
	imull	$1103515245, %eax, %eax
	addl	$12345, %eax
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	get_random, .-get_random
	.globl	c0func
	.type	c0func, @function
c0func:
.LFB1:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$56, %esp
	movl	$0, -32(%ebp)
	movl	16(%ebp), %eax
	movl	%eax, -28(%ebp)
	movl	$0, -24(%ebp)
	jmp	.L4
.L6:
	movl	-28(%ebp), %eax
	movl	%eax, (%esp)
	call	get_random
	movl	%eax, -28(%ebp)
	movl	-28(%ebp), %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, -20(%ebp)
	movl	-28(%ebp), %eax
	movl	%eax, (%esp)
	call	get_random
	movl	%eax, -28(%ebp)
	movl	-28(%ebp), %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, -16(%ebp)
	movl	-28(%ebp), %eax
	movl	%eax, (%esp)
	call	get_random
	movl	%eax, -28(%ebp)
	movl	-28(%ebp), %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, -12(%ebp)
	movl	-20(%ebp), %eax
	imull	-20(%ebp), %eax
	movl	%eax, %edx
	movl	-16(%ebp), %eax
	imull	-16(%ebp), %eax
	addl	%eax, %edx
	movl	-12(%ebp), %eax
	imull	-12(%ebp), %eax
	addl	%eax, %edx
	movl	12(%ebp), %eax
	imull	12(%ebp), %eax
	cmpl	%eax, %edx
	jg	.L5
	addl	$1, -24(%ebp)
.L5:
	addl	$1, -32(%ebp)
.L4:
	movl	-32(%ebp), %eax
	cmpl	8(%ebp), %eax
	jl	.L6
	movl	-24(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	cltd
	idivl	8(%ebp)
	movl	%eax, (%esp)
	call	print_int
	movl	$46, (%esp)
	call	print_char
	movl	-24(%ebp), %edx
	movl	%edx, %eax
	addl	%eax, %eax
	addl	%edx, %eax
	addl	%eax, %eax
	cltd
	idivl	8(%ebp)
	movl	%edx, %eax
	movl	%eax, (%esp)
	call	print_int
	call	print_line
	movl	$0, %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	c0func, .-c0func
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
