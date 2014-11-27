	.file	"ackermann.c"
	.text
	.globl	ack
	.type	ack, @function
ack:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	cmpl	$0, 8(%ebp)
	jne	.L1
	movl	12(%ebp), %ebx
	addl	$1, %ebx
	movl	%ebx, %eax
	jmp	.L6
.L1:
	cmpl	$0, 12(%ebp)
	jne	.L4
	pushl	$1
	movl	8(%ebp), %ebx
	subl	$1, %ebx
	pushl	%ebx
	call	ack
	addl	$8, %esp
	jmp	.L6
.L4:
	movl	12(%ebp), %ebx
	subl	$1, %ebx
	pushl	%ebx
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	ack
	addl	$8, %esp
	pushl	%eax
	movl	8(%ebp), %ebx
	subl	$1, %ebx
	pushl	%ebx
	call	ack
	addl	$8, %esp
	jmp	.L6
.L6:
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size ack, .-ack
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
	subl	$12, %esp
	movl	$0, -16(%ebp)
	jmp	.L10
.L9:
	movl	-16(%ebp), %esi
	pushl	%esi
	movl	8(%ebp), %esi
	pushl	%esi
	call	ack
	addl	$8, %esp
	pushl	%eax
	call	print_int
	addl	$4, %esp
	call	print_line
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L10:
	movl	12(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jl	.L9
.L11:
	movl	$0, %eax
	addl	$12, %esp
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
