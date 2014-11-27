	.file	"f36.c"
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
	movl	$1, %esi
	movl	$2, %ebx
	imull	$3, %ebx
	addl	%ebx, %esi
	movl	$4, %ebx
	movl	%ebx, %eax
	movl	$2, %ecx
	cltd
	idivl	%ecx
	movl	%eax, %ebx
	addl	%ebx, %esi
	movl	$6, %ebx
	movl	%ebx, %eax
	movl	$3, %ecx
	cltd
	idivl	%ecx
	movl	%edx, %ebx
	addl	%ebx, %esi
	movl	$7, %ebx
	movl	%ebx, %eax
	movl	$10, %ecx
	cltd
	idivl	%ecx
	movl	%eax, %ebx
	addl	%ebx, %esi
	movl	$8, %ebx
	movl	%ebx, %eax
	movl	$5, %ecx
	cltd
	idivl	%ecx
	movl	%edx, %ebx
	addl	%ebx, %esi
	subl	$9, %esi
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
