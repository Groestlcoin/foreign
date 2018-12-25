include ksamd64.inc

.CODE

EXTERN RtlUnwindEx	:NEAR

longjmp	PROC
	sub		esp, CONTEXT_FRAME_LENGTH + 30h + 30h + 8			;  6 args of RtlUnwindEx() + EXCEPTION_RECORD(1 param)
	test	rdx, rdx
	jnz		LJ10
	inc		rdx
LJ10:
	xor		r10, r10
	cmp		[rcx + JbFrame], r10
	jne		LJ20
	mov		rax, rdx
	mov		rbx, [rcx + JbRbx]
	mov		rsi, [rcx + JbRsi]
	mov		rdi, [rcx + JbRdi]
	mov		r12, [rcx + JbR12]
	mov		r13, [rcx + JbR13]
	mov		r14, [rcx + JbR14]
	mov		r15, [rcx + JbR15]
	ldmxcsr	[rcx + JbMxCsr]
	fnclex
	fldcw	[rcx + JbFpCsr]
	movdqa	xmm6, [rcx + JbXmm6]
	movdqa	xmm7, [rcx + JbXmm7]
	movdqa	xmm8, [rcx + JbXmm8]
	movdqa	xmm9, [rcx + JbXmm9]
	movdqa	xmm10, [rcx + JbXmm10]
	movdqa	xmm11, [rcx + JbXmm11]
	movdqa	xmm12, [rcx + JbXmm12]
	movdqa	xmm13, [rcx + JbXmm13]
	movdqa	xmm14, [rcx + JbXmm14]
	movdqa	xmm15, [rcx + JbXmm15]
	mov		rdx, [rcx + JbRip]
	mov		rbp, [rcx + JbRbp]
	mov		rsp, [rcx + JbRsp]
	jmp		rdx
LJ20:
	mov		[rsp + 28h], r10											; HistoryTable
	mov		DWORD PTR [rsp + 30h + ErExceptionCode], STATUS_LONGJUMP
	mov		[rsp + 30h + ErExceptionFlags], r10d						; zero fields
	mov		[rsp + 30h + ErExceptionRecord], r10
	mov		[rsp + 30h + ErExceptionAddress], r10
	inc		r10d
	mov		[rsp + 30h + ErNumberParameters], r10d						; one parameter
	mov		[rsp + 30h + ErExceptionInformation], rcx					; &jmp_buf
	lea		rax, [rsp + 30h + 30h]
	mov		[rsp + 20h], rax											; OriginalContext
	mov		r9, rdx														; ReturnValue
	lea		r8, [rsp + 30h]												; ExceptionRecord
	mov		rdx, [rcx + JbRip]
	mov		rcx, [rcx + JbFrame]
	call    RtlUnwindEx
	jmp		LJ20
longjmp	ENDP

END

