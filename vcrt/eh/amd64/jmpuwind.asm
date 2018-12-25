include ksamd64.inc

.CODE

EXTERN RtlUnwindEx	:NEAR

_local_unwind	PROC FRAME
	sub		rsp, CONTEXT_FRAME_LENGTH+8
	.ENDPROLOG
	xor		r8, r8
	xor		r9, r9
	mov		[rsp+32], rsp		; OriginalContext
	mov		[rsp+40], r8		; HistoryTable
	call    RtlUnwindEx
	add		rsp, CONTEXT_FRAME_LENGTH+8
	ret
_local_unwind	ENDP

END

