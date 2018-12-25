include ksamd64.inc

.CODE

EXTERN	_setjmpexused	:DQ


_setjmp	PROC
	mov		rax, _setjmpexused
	test	rax, rax
	jnz		SJ10
	mov		[rcx + JbFrame], rax
	mov		[rcx + JbRbx], rbx
	mov		[rcx + JbRbp], rbp
	mov		[rcx + JbRsi], rsi
	mov		[rcx + JbRdi], rdi
	mov		[rcx + JbR12], r12
	mov		[rcx + JbR13], r13
	mov		[rcx + JbR14], r14
	mov		[rcx + JbR15], r15
	lea		r8, [rsp + 8]
	mov		[rcx + JbRsp], r8
	lea		r8, [rsp]
	mov		[rcx + JbRip], r8
	stmxcsr	[rcx + JbMxCsr]
	fnstcw	[rcx + JbFpCsr]
	movdqa	[rcx + JbXmm6], xmm6
	movdqa	[rcx + JbXmm7], xmm7
	movdqa	[rcx + JbXmm8], xmm8
	movdqa	[rcx + JbXmm9], xmm9
	movdqa	[rcx + JbXmm10], xmm10
	movdqa	[rcx + JbXmm11], xmm11
	movdqa	[rcx + JbXmm12], xmm12
	movdqa	[rcx + JbXmm13], xmm13
	movdqa	[rcx + JbXmm14], xmm14
	movdqa	[rcx + JbXmm15], xmm15
	ret
SJ10:
	jmp		rax
_setjmp	ENDP

END


