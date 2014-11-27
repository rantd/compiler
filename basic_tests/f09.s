	.file	"f09.c"
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
	movl	$100, %eax
	jmp	.L3
	jmp	.L2
.L1:
	movl	$200, %eax
	jmp	.L3
.L2:
.L3:
	addl	$4, %esp
	popl	%ebx
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
	.size f, .-f
	.ident "GCC: (Ubuntu 4.3.3-5ubuntu4) 4.3.3"
	.section	.note.GNU-stack,"",@progbits
