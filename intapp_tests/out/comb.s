	.file	"comb.c"
	.text
	.globl	mod_pow
	.type	mod_pow, @function
mod_pow:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$16, %esp
	movl	$1, -16(%ebp)
	movl	8(%ebp), %ecx
	movl	%ecx, -20(%ebp)
	jmp	.L2
.L1:
	movl	$2, %ecx
	movl	12(%ebp), %eax
	cltd
	idivl	%ecx
	cmpl	$1, %edx
	jne	.L5
	movl	-16(%ebp), %esi
	imull	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -16(%ebp)
.L5:
	movl	-20(%ebp), %esi
	imull	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -20(%ebp)
	movl	12(%ebp), %esi
	movl	%esi, %eax
	movl	$2, %ecx
	cltd
	idivl	%ecx
	movl	%eax, %esi
	movl	%esi, 12(%ebp)
.L2:
	cmpl	$0, 12(%ebp)
	jg	.L1
.L3:
	movl	-16(%ebp), %eax
	addl	$16, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size mod_pow, .-mod_pow
	.globl	comb
	.type	comb, @function
comb:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$28, %esp
	movl	12(%ebp), %eax
	cmpl	%eax, 8(%ebp)
	jle	.L10
	movl	$0, %eax
	jmp	.L31
.L10:
	movl	12(%ebp), %eax
	cmpl	%eax, 8(%ebp)
	jne	.L13
	movl	$1, %eax
	jmp	.L31
.L13:
	cmpl	$0, 8(%ebp)
	jne	.L16
	movl	$1, %eax
	jmp	.L31
.L16:
	movl	$1, -32(%ebp)
	movl	$1, -16(%ebp)
	jmp	.L20
.L19:
	movl	-32(%ebp), %esi
	imull	-16(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -32(%ebp)
	movl	8(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jne	.L23
	movl	-32(%ebp), %ecx
	movl	%ecx, -20(%ebp)
.L23:
	movl	12(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jne	.L26
	movl	-32(%ebp), %ecx
	movl	%ecx, -24(%ebp)
.L26:
	movl	-16(%ebp), %esi
	movl	12(%ebp), %ebx
	subl	8(%ebp), %ebx
	cmpl	%ebx, %esi
	jne	.L29
	movl	-32(%ebp), %ecx
	movl	%ecx, -28(%ebp)
.L29:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L20:
	movl	12(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jle	.L19
.L21:
	movl	-24(%ebp), %ecx
	movl	%ecx, -32(%ebp)
	movl	-32(%ebp), %esi
	movl	16(%ebp), %ebx
	pushl	%ebx
	movl	16(%ebp), %ebx
	subl	$2, %ebx
	pushl	%ebx
	movl	-20(%ebp), %ebx
	pushl	%ebx
	call	mod_pow
	addl	$12, %esp
	imull	%eax, %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -32(%ebp)
	movl	-32(%ebp), %esi
	movl	16(%ebp), %ebx
	pushl	%ebx
	movl	16(%ebp), %ebx
	subl	$2, %ebx
	pushl	%ebx
	movl	-28(%ebp), %ebx
	pushl	%ebx
	call	mod_pow
	addl	$12, %esp
	imull	%eax, %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -32(%ebp)
	movl	-32(%ebp), %eax
	jmp	.L31
.L31:
	addl	$28, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size comb, .-comb
	.globl	c0func
	.type	c0func, @function
c0func:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$28, %esp
	movl	8(%ebp), %ecx
	movl	%ecx, -16(%ebp)
	movl	12(%ebp), %ecx
	movl	%ecx, -20(%ebp)
	movl	$1, -32(%ebp)
	jmp	.L35
.L34:
	movl	-16(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -24(%ebp)
	movl	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -28(%ebp)
	movl	-32(%ebp), %esi
	movl	16(%ebp), %ebx
	pushl	%ebx
	movl	-28(%ebp), %ebx
	pushl	%ebx
	movl	-24(%ebp), %ebx
	pushl	%ebx
	call	comb
	addl	$12, %esp
	imull	%eax, %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%edx, %esi
	movl	%esi, -32(%ebp)
	movl	-16(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%eax, %esi
	movl	%esi, -16(%ebp)
	movl	-20(%ebp), %esi
	movl	%esi, %eax
	cltd
	idivl	16(%ebp)
	movl	%eax, %esi
	movl	%esi, -20(%ebp)
	movl	-20(%ebp), %esi
	addl	-16(%ebp), %esi
	cmpl	$0, %esi
	jne	.L38
	jmp	.L36
.L38:
.L35:
	jmp	.L34
.L36:
	movl	-32(%ebp), %esi
	pushl	%esi
	call	print_int
	addl	$4, %esp
	call	print_line
	movl	$0, %eax
	addl	$28, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size c0func, .-c0func
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
