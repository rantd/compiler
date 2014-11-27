	.file	"f51.c"
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
	movl	$0, -16(%ebp)
	movl	$0, -20(%ebp)
	jmp	.L2
.L1:
	movl	$4, %ecx
	movl	-16(%ebp), %eax
	cltd
	idivl	%ecx
	cmpl	$0, %edx
	jne	.L5
	movl	-20(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -20(%ebp)
.L5:
	movl	$4, %ecx
	movl	-16(%ebp), %eax
	cltd
	idivl	%ecx
	cmpl	$1, %edx
	jne	.L8
	movl	-20(%ebp), %esi
	addl	$100, %esi
	movl	%esi, -20(%ebp)
.L8:
	movl	$4, %ecx
	movl	-16(%ebp), %eax
	cltd
	idivl	%ecx
	cmpl	$2, %edx
	jne	.L11
	movl	-20(%ebp), %esi
	addl	$10000, %esi
	movl	%esi, -20(%ebp)
	jmp	.L12
.L11:
	movl	-20(%ebp), %esi
	addl	$1000000, %esi
	movl	%esi, -20(%ebp)
.L12:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L2:
	cmpl	$100, -16(%ebp)
	jl	.L1
.L3:
	movl	-20(%ebp), %eax
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
