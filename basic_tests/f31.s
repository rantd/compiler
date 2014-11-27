	.file	"f31.c"
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
	cmpl	$0, 12(%ebp)
	jge	.L1
	cmpl	$0, 8(%ebp)
	jge	.L4
	movl	12(%ebp), %ebx
	negl	%ebx
	pushl	%ebx
	movl	8(%ebp), %ebx
	negl	%ebx
	pushl	%ebx
	call	g
	addl	$8, %esp
	jmp	.L9
	jmp	.L5
.L4:
	movl	12(%ebp), %ebx
	negl	%ebx
	pushl	%ebx
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	g
	addl	$8, %esp
	movl	%eax, %ebx
	negl	%ebx
	movl	%ebx, %eax
	jmp	.L9
.L5:
	jmp	.L2
.L1:
	cmpl	$0, 8(%ebp)
	jge	.L7
	movl	12(%ebp), %ebx
	pushl	%ebx
	movl	8(%ebp), %ebx
	negl	%ebx
	pushl	%ebx
	call	g
	addl	$8, %esp
	movl	%eax, %ebx
	negl	%ebx
	movl	%ebx, %eax
	jmp	.L9
	jmp	.L8
.L7:
	movl	12(%ebp), %ebx
	pushl	%ebx
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	g
	addl	$8, %esp
	jmp	.L9
.L8:
.L2:
.L9:
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
	subl	$20, %esp
	movl	12(%ebp), %eax
	cmpl	%eax, 8(%ebp)
	jge	.L12
	movl	$0, %eax
	jmp	.L18
	jmp	.L13
.L12:
	movl	12(%ebp), %ecx
	movl	%ecx, -16(%ebp)
	movl	$1, -20(%ebp)
	movl	8(%ebp), %esi
	subl	12(%ebp), %esi
	movl	%esi, -24(%ebp)
	jmp	.L16
.L15:
	movl	-24(%ebp), %esi
	subl	-16(%ebp), %esi
	movl	%esi, -24(%ebp)
	movl	-16(%ebp), %esi
	addl	-16(%ebp), %esi
	movl	%esi, -16(%ebp)
	movl	-20(%ebp), %esi
	addl	-20(%ebp), %esi
	movl	%esi, -20(%ebp)
.L16:
	movl	-16(%ebp), %eax
	cmpl	%eax, -24(%ebp)
	jg	.L15
.L17:
	movl	-20(%ebp), %esi
	movl	12(%ebp), %ebx
	pushl	%ebx
	movl	-24(%ebp), %ebx
	pushl	%ebx
	call	g
	addl	$8, %esp
	addl	%eax, %esi
	movl	%esi, %eax
	jmp	.L18
.L13:
.L18:
	addl	$20, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size g, .-g
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
