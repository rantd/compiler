	.file	"f48.c"
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
	subl	$20, %esp
	movl	$0, -16(%ebp)
	movl	$0, -20(%ebp)
	jmp	.L2
.L1:
	cmpl	$70, -16(%ebp)
	jl	.L5
	movl	$0, -24(%ebp)
	jmp	.L9
.L8:
	cmpl	$50, -24(%ebp)
	jl	.L12
	jmp	.L10
.L12:
	movl	-20(%ebp), %esi
	addl	-24(%ebp), %esi
	movl	%esi, -20(%ebp)
	movl	-24(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -24(%ebp)
.L9:
	movl	-16(%ebp), %eax
	cmpl	%eax, -24(%ebp)
	jl	.L8
.L10:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
	jmp	.L2
.L5:
	movl	-20(%ebp), %esi
	addl	-16(%ebp), %esi
	movl	%esi, -20(%ebp)
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L2:
	cmpl	$100, -16(%ebp)
	jl	.L1
.L3:
	movl	-20(%ebp), %eax
	addl	$20, %esp
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
