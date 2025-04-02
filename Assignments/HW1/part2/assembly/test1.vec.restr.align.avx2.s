	.file	"test1.c"
	.text
	.p2align 4
	.globl	test1
	.type	test1, @function
test1:
.LFB0:
	.cfi_startproc
	pushq	%r13
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	movl	%ecx, %r13d
	pushq	%r12
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movq	%rsi, %r12
	pushq	%rbp
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	movq	%rdi, %rbp
	xorl	%edi, %edi
	pushq	%rbx
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rdx, %rbx
	subq	$8, %rsp
	.cfi_def_cfa_offset 48
	cmpl	$1024, %r13d
	sete	%dil
	xorl	%eax, %eax
	call	__builtin_assume@PLT
	testl	%r13d, %r13d
	jle	.L5
	movq	%rbx, %rax
	leaq	(%rbx,%r13,4), %rcx
	xorl	%edx, %edx
	.p2align 4,,10
	.p2align 3
.L3:
	vmovss	0(%rbp,%rdx), %xmm0
	vaddss	(%r12,%rdx), %xmm0, %xmm0
	addq	$4, %rax
	addq	$4, %rdx
	vmovss	%xmm0, -4(%rax)
	cmpq	%rcx, %rax
	jne	.L3
.L5:
	addq	$8, %rsp
	.cfi_def_cfa_offset 40
	popq	%rbx
	.cfi_def_cfa_offset 32
	popq	%rbp
	.cfi_def_cfa_offset 24
	popq	%r12
	.cfi_def_cfa_offset 16
	popq	%r13
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE0:
	.size	test1, .-test1
	.ident	"GCC: (Debian 12.2.0-14) 12.2.0"
	.section	.note.GNU-stack,"",@progbits
