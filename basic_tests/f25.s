	.file	"f25.c"
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
	subl	$8, %esp
	cmpl	$0, 8(%ebp)
	jne	.L1
	movl	$1, %eax
	jmp	.L3
	jmp	.L2
.L1:
	movl	8(%ebp), %esi
	subl	$1, %esi
	pushl	%esi
	call	f
	addl	$4, %esp
	movl	%eax, %esi
	movl	8(%ebp), %ebx
	subl	$1, %ebx
	pushl	%ebx
	call	f
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
	.size f, .-f
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
