	.file	"f39.c"
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
	pushl	%esi
	subl	$16, %esp
	movl	$1, -16(%ebp)
	movl	$10, -20(%ebp)
	movl	-20(%ebp), %esi
	addl	-16(%ebp), %esi
	movl	%esi, -16(%ebp)
	movl	-16(%ebp), %eax
	addl	$16, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size f, .-f
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
