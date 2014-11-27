	.file	"f24.c"
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
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	g
	addl	$4, %esp
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
	pushl	%esi
	subl	$8, %esp
	movl	8(%ebp), %esi
	pushl	%esi
	call	h
	addl	$4, %esp
	movl	%eax, %esi
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	h
	addl	$4, %esp
	addl	%eax, %esi
	movl	%esi, %eax
	addl	$8, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size g, .-g
	.globl	h
	.type	h, @function
h:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$8, %esp
	movl	8(%ebp), %esi
	pushl	%esi
	call	i
	addl	$4, %esp
	movl	%eax, %esi
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	i
	addl	$4, %esp
	addl	%eax, %esi
	movl	%esi, %eax
	addl	$8, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size h, .-h
	.globl	i
	.type	i, @function
i:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$4, %esp
	movl	$1, %eax
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size i, .-i
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
