	.file	"f28.c"
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
	movl	$0, -16(%ebp)
	cmpl	$0, 12(%ebp)
	jge	.L1
	movl	8(%ebp), %esi
	negl	%esi
	movl	%esi, 8(%ebp)
	movl	12(%ebp), %esi
	negl	%esi
	movl	%esi, 12(%ebp)
.L1:
	jmp	.L5
.L4:
	movl	$2, %ecx
	movl	12(%ebp), %eax
	cltd
	idivl	%ecx
	cmpl	$1, %edx
	jne	.L8
	movl	-16(%ebp), %esi
	addl	8(%ebp), %esi
	movl	%esi, -16(%ebp)
.L8:
	movl	8(%ebp), %esi
	addl	8(%ebp), %esi
	movl	%esi, 8(%ebp)
	movl	12(%ebp), %esi
	movl	%esi, %eax
	movl	$2, %ecx
	cltd
	idivl	%ecx
	movl	%eax, %esi
	movl	%esi, 12(%ebp)
.L5:
	cmpl	$0, 12(%ebp)
	jne	.L4
.L6:
	movl	-16(%ebp), %eax
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
