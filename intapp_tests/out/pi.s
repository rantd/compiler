	.file	"pi.c"
	.text
	.globl	get_random
	.type	get_random, @function
get_random:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	imull	$1103515245, %ebx
	addl	$12345, %ebx
	movl	%ebx, %eax
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size get_random, .-get_random
	.globl	c0func
	.type	c0func, @function
c0func:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$32, %esp
	movl	$0, -16(%ebp)
	movl	16(%ebp), %ecx
	movl	%ecx, -20(%ebp)
	movl	$0, -24(%ebp)
	jmp	.L4
.L3:
	movl	-20(%ebp), %esi
	pushl	%esi
	call	get_random
	addl	$4, %esp
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, %esi
	movl	%esi, -28(%ebp)
	movl	-20(%ebp), %esi
	pushl	%esi
	call	get_random
	addl	$4, %esp
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, %esi
	movl	%esi, -32(%ebp)
	movl	-20(%ebp), %esi
	pushl	%esi
	call	get_random
	addl	$4, %esp
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	12(%ebp)
	movl	%edx, %esi
	movl	%esi, -36(%ebp)
	movl	-28(%ebp), %esi
	imull	-28(%ebp), %esi
	movl	-32(%ebp), %ebx
	imull	-32(%ebp), %ebx
	addl	%ebx, %esi
	movl	-36(%ebp), %ebx
	imull	-36(%ebp), %ebx
	addl	%ebx, %esi
	movl	12(%ebp), %ebx
	imull	12(%ebp), %ebx
	cmpl	%ebx, %esi
	jg	.L7
	movl	-24(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -24(%ebp)
.L7:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L4:
	movl	8(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jl	.L3
.L5:
	movl	-24(%ebp), %esi
	imull	$6, %esi
	movl	%esi, %eax
	cltd
	idivl	8(%ebp)
	movl	%eax, %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	pushl	$46
	call	print_char
	addl	$4, %esp
	movl	-24(%ebp), %esi
	imull	$6, %esi
	movl	%esi, %eax
	cltd
	idivl	8(%ebp)
	movl	%edx, %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_line
	movl	$0, %eax
	addl	$32, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size c0func, .-c0func
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
