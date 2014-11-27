	.file	"fib.c"
	.text
	.globl	fib
	.type	fib, @function
fib:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$8, %esp
	cmpl	$2, 8(%ebp)
	jge	.L1
	movl	$1, %eax
	jmp	.L3
	jmp	.L2
.L1:
	movl	8(%ebp), %esi
	subl	$1, %esi
	pushl	%esi
	call	fib
	addl	$4, %esp
	movl	%eax, %esi
	movl	8(%ebp), %ebx
	subl	$2, %ebx
	pushl	%ebx
	call	fib
	addl	$4, %esp
	addl	%eax, %esi
	movl	%esi, %eax
	jmp	.L3
.L2:
.L3:
	addl	$8, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size fib, .-fib
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
	subl	$4, %esp
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	fib
	addl	$4, %esp
	pushl	%eax
	call	print_int
	addl	$4, %esp
	call	print_line
	movl	$0, %eax
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size c0func, .-c0func
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
