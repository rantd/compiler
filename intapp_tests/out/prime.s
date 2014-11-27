	.file	"prime.c"
	.text
	.globl	check_prime
	.type	check_prime, @function
check_prime:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$12, %esp
	movl	$2, -16(%ebp)
	jmp	.L2
.L1:
	movl	8(%ebp), %eax
	cltd
	idivl	-16(%ebp)
	cmpl	$0, %edx
	jne	.L5
	movl	$0, %eax
	jmp	.L7
.L5:
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L2:
	movl	-16(%ebp), %esi
	imull	-16(%ebp), %esi
	cmpl	8(%ebp), %esi
	jle	.L1
.L3:
	movl	$1, %eax
	jmp	.L7
.L7:
	addl	$12, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size check_prime, .-check_prime
	.globl	count_primes
	.type	count_primes, @function
count_primes:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	pushl	%esi
	subl	$16, %esp
	movl	$2, -16(%ebp)
	movl	$0, -20(%ebp)
	jmp	.L11
.L10:
	movl	-20(%ebp), %esi
	movl	-16(%ebp), %ebx
	pushl	%ebx
	call	check_prime
	addl	$4, %esp
	addl	%eax, %esi
	movl	%esi, -20(%ebp)
	movl	-16(%ebp), %esi
	addl	$1, %esi
	movl	%esi, -16(%ebp)
.L11:
	movl	8(%ebp), %eax
	cmpl	%eax, -16(%ebp)
	jl	.L10
.L12:
	movl	-20(%ebp), %eax
	addl	$16, %esp
	popl	%esi
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size count_primes, .-count_primes
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
	subl	$4, %esp
	movl	8(%ebp), %ebx
	pushl	%ebx
	call	count_primes
	addl	$4, %esp
	pushl	%eax
	call	print_int
	addl	$4, %esp
	call	print_line
	movl	$0, %eax
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size c0func, .-c0func
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
