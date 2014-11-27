	.file	"f34.c"
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
	subl	$12, %esp
	movl	8(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	12(%ebp)
	movl	%eax, %esi
	movl	%esi, -16(%ebp)
	movl	8(%ebp), %esi
	movl	-16(%ebp), %ebx
	imull	12(%ebp), %ebx
	subl	%ebx, %esi
	movl	%esi, %eax
	addl	$12, %esp
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
