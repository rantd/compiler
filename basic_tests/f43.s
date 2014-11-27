	.file	"f43.c"
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
	subl	$28, %esp
	movl	$1, %esi
	movl	$1, %ebx
	addl	$2, %ebx
	cmpl	%ebx, %esi
	setne	%al
	movzbl	%al, %esi
	movl	%esi, -16(%ebp)
	movl	$1, %esi
	addl	$2, %esi
	movl	$1, %ebx
	addl	$2, %ebx
	cmpl	%ebx, %esi
	sete	%al
	movzbl	%al, %esi
	movl	%esi, -20(%ebp)
	movl	$1, %esi
	addl	$2, %esi
	movl	$1, %ebx
	addl	$2, %ebx
	cmpl	%ebx, %esi
	setle	%al
	movzbl	%al, %esi
	movl	%esi, -24(%ebp)
	movl	$1, %esi
	movl	$-2, %ebx
	subl	%ebx, %esi
	movl	$-1, %ebx
	negl	%ebx
	addl	$2, %ebx
	cmpl	%ebx, %esi
	setge	%al
	movzbl	%al, %esi
	movl	%esi, -28(%ebp)
	movl	$1, %esi
	cmpl	$0, %esi
	sete	%al
	movzbl	%al, %esi
	movl	$2, %ebx
	cmpl	$0, %ebx
	sete	%al
	movzbl	%al, %ebx
	cmpl	%ebx, %esi
	sete	%al
	movzbl	%al, %esi
	movl	%esi, -32(%ebp)
	movl	-16(%ebp), %esi
	imull	$10000, %esi
	movl	-20(%ebp), %ebx
	imull	$1000, %ebx
	addl	%ebx, %esi
	movl	-24(%ebp), %ebx
	imull	$100, %ebx
	addl	%ebx, %esi
	movl	-28(%ebp), %ebx
	imull	$10, %ebx
	addl	%ebx, %esi
	addl	-32(%ebp), %esi
	movl	%esi, %eax
	addl	$28, %esp
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
