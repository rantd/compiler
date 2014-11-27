	.file	"f37.c"
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
	movl	$4, %ebx
	movl	$2, %esi
	movl	%ebx, %eax
	cltd
	idivl	%esi
	movl	%eax, %ebx
	movl	$6, %esi
	movl	%esi, %eax
	movl	$3, %ecx
	cltd
	idivl	%ecx
	movl	%edx, %esi
	addl	%esi, %ebx
	addl	$7, %ebx
	movl	%ebx, %eax
	movl	$10, %ecx
	cltd
	idivl	%ecx
	movl	%eax, %ebx
	movl	$1, %esi
	addl	$2, %esi
	imull	$3, %esi
	addl	%ebx, %esi
	movl	$8, %ebx
	movl	%ebx, %eax
	movl	$5, %ecx
	cltd
	idivl	%ecx
	movl	%edx, %ebx
	subl	$9, %ebx
	addl	%ebx, %esi
	movl	%esi, %eax
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
