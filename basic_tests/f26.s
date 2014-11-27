	.file	"f26.c"
	.text
	.globl	f
	.type	f, @function
f:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	pushl	$9
	pushl	$8
	pushl	$7
	call	g
	addl	$12, %esp
	pushl	%eax
	pushl	$6
	pushl	$5
	pushl	$4
	call	g
	addl	$12, %esp
	pushl	%eax
	pushl	$3
	pushl	$2
	pushl	$1
	call	g
	addl	$12, %esp
	pushl	%eax
	call	g
	addl	$12, %esp
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size f, .-f
	.globl	g
	.type	g, @function
g:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	movl	8(%ebp), %ebx
	addl	12(%ebp), %ebx
	addl	16(%ebp), %ebx
	movl	%ebx, %eax
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size g, .-g
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
