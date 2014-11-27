	.file	"prime.c"
	.text
	.globl	check_prime
	.type	check_prime, @function
check_prime:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$16, %esp
	movl	$2, -4(%ebp)
	jmp	.L2
.L5:
	movl	8(%ebp), %eax
	cltd
	idivl	-4(%ebp)
	movl	%edx, %eax
	testl	%eax, %eax
	jne	.L3
	movl	$0, %eax
	jmp	.L4
.L3:
	addl	$1, -4(%ebp)
.L2:
	movl	-4(%ebp), %eax
	imull	-4(%ebp), %eax
	cmpl	8(%ebp), %eax
	jle	.L5
	movl	$1, %eax
.L4:
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE0:
	.size	check_prime, .-check_prime
	.globl	count_primes
	.type	count_primes, @function
count_primes:
.LFB1:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$20, %esp
	movl	$2, -8(%ebp)
	movl	$0, -4(%ebp)
	jmp	.L7
.L8:
	movl	-8(%ebp), %eax
	movl	%eax, (%esp)
	call	check_prime
	addl	%eax, -4(%ebp)
	addl	$1, -8(%ebp)
.L7:
	movl	-8(%ebp), %eax
	cmpl	8(%ebp), %eax
	jl	.L8
	movl	-4(%ebp), %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.size	count_primes, .-count_primes
	.globl	c0func
	.type	c0func, @function
c0func:
.LFB2:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	8(%ebp), %eax
	movl	%eax, (%esp)
	call	count_primes
	movl	%eax, (%esp)
	call	print_int
	call	print_line
	movl	$0, %eax
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.size	c0func, .-c0func
	.ident	"GCC: (Ubuntu 4.8.2-19ubuntu1) 4.8.2"
	.section	.note.GNU-stack,"",@progbits
