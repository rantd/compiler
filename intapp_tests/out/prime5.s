	.file	"prime5.c"
	.text
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
	subl	$44, %esp
	movl	$0, -32(%ebp)
	movl	$0, -36(%ebp)
	movl	$0, -40(%ebp)
	movl	$0, -44(%ebp)
	movl	$0, -48(%ebp)
	movl	$2, -16(%ebp)
	jmp	.L2
.L1:
	movl	$2, -20(%ebp)
	movl	$0, -24(%ebp)
	movl	$0, -28(%ebp)
	jmp	.L6
.L5:
	movl	-16(%ebp), %eax
	cltd
	idivl	-20(%ebp)
	cmpl	$0, %edx
	jne	.L9
	movl	$1, -28(%ebp)
	jmp	.L7
.L9:
	movl	-20(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -20(%ebp)
.L6:
	movl	-20(%ebp), %esi
	imull	-20(%ebp), %esi
	cmpl	-16(%ebp), %esi
	jle	.L5
.L7:
	cmpl	$0, -28(%ebp)
	jne	.L12
	movl	-36(%ebp), %ecx
	movl	%ecx, -32(%ebp)
	movl	-40(%ebp), %ecx
	movl	%ecx, -36(%ebp)
	movl	-44(%ebp), %ecx
	movl	%ecx, -40(%ebp)
	movl	-48(%ebp), %ecx
	movl	%ecx, -44(%ebp)
	movl	-16(%ebp), %ecx
	movl	%ecx, -48(%ebp)
	movl	-36(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$2, %ebx
	cmpl	%ebx, %esi
	jne	.L15
	movl	-40(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$6, %ebx
	cmpl	%ebx, %esi
	jne	.L18
	movl	-44(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$8, %ebx
	cmpl	%ebx, %esi
	jne	.L21
	movl	-48(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$12, %ebx
	cmpl	%ebx, %esi
	jne	.L24
	movl	-32(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-36(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-40(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-44(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-48(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_line
.L24:
.L21:
.L18:
.L15:
	movl	-36(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$4, %ebx
	cmpl	%ebx, %esi
	jne	.L27
	movl	-40(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$6, %ebx
	cmpl	%ebx, %esi
	jne	.L30
	movl	-44(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$10, %ebx
	cmpl	%ebx, %esi
	jne	.L33
	movl	-48(%ebp), %esi
	movl	-32(%ebp), %ebx
	addl	$12, %ebx
	cmpl	%ebx, %esi
	jne	.L36
	movl	-32(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-36(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-40(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-44(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_space
	movl	-48(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_line
.L36:
.L33:
.L30:
.L27:
.L12:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L2:
	movl	8(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jl	.L1
.L3:
	addl	$44, %esp
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
