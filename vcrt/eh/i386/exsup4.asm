small32 equ 1
flat32  equ 1

include pversion.inc
include cmacros.inc
include exsup.inc

OPTION LANGUAGE: C

EXTERN	_NLG_Notify					:NEAR
EXTERN	__security_cookie			:NEAR
EXTERN	SYSCALL @__security_check_cookie@4	:NEAR

EXTERN	API_RtlUnwind@16			:NEAR

.CODE

;; static void __cdecl _local_unwind4(uintptr_t *pSecCheck, EXCEPTION_REGISTRATION *xreg, int tryLevel)
_local_unwind4 PROC PRIVATE, pSecCheck:DWORD, xreg:DWORD, tryLevel:DWORD
   	push	ebx
   	push	esi
   	push	edi
   	mov		edx, pSecCheck
   	mov		eax, xreg
   	mov		eax, tryLevel
   	push	ebp
   	push	edx
   	push	eax
   	push	ecx
   	push	ecx
   	push	offset _unwind_handler4
   	push	dword ptr fs:[0]
   	mov		eax, __security_cookie
   	xor		eax, esp
   	mov		[esp+1Ch-14], eax
   	mov		fs:[0], esp
_lu_top:
   	mov		eax, [esp+1Ch+14h]
   	mov		ebx, [eax+8]
   	mov		ecx, [esp+1Ch+10h]
   	xor		ebx, [ecx]
   	mov		esi, [eax+0Ch]
   	cmp		esi, -2
   	jz		_lu_done
   	mov		edx, [esp+1Ch+18h]
   	cmp		edx, -2
   	jz		loc_0_54
   	cmp		esi, edx
   	jbe		_lu_done
loc_0_54:
   	lea		esi, [esi+esi*2]
   	lea		ebx, [ebx+esi*4+10h]
   	mov		ecx, [ebx]
   	mov		[eax+0Ch], ecx
   	cmp		dword ptr [ebx+4], 0
   	jnz		_lu_top
   	push	101h
   	mov		eax, [ebx+8]
   	; call	_NLG_Call
   	call    eax
   	jmp		_lu_top
_lu_done:
   	pop		dword ptr fs:0
   	add		esp, 18h
   	pop		edi
   	pop		esi
   	pop		ebx
   	ret
_local_unwind4 ENDP

;; int __cdecl _unwind_handler4(EXCEPTION_RECORD *xr, EXCEPTION_REGISTRATION *xreg, CONTEXT *ctx, EXCEPTION_REGISTRATION *dispatcher)
_unwind_handler4 PROC PRIVATE, xr:DWORD, xreg:DWORD, ctx:DWORD, dispatcher:DWORD
	mov		ecx, xr
	test	[ecx].exception_flags, EXCEPTION_UNWIND_CONTEXT
	mov		eax, DISPOSITION_CONTINUE_SEARCH
	jz		@@ret
	mov		eax, xreg
	mov		ecx, [eax+8]										; int(xreg->scopetable)
	xor		ecx, eax
	call	@__security_check_cookie@4
	push	ebp
	mov		ebp, [eax+18h]										; sizeof(EXCEPTION_REGISTRATION)-4
	INVOKE	_local_unwind4, [eax+14h], [eax+10h], [eax+0Ch]		; pSecCheck, _ebp, tryLevel
	pop		ebp
	mov		eax, xreg
	mov		edx, dispatcher
	mov		[edx], eax											; dispatcher->prev = xreg;
	mov		eax, DISPOSITION_COLLIDED_UNWIND
@@ret:
	ret
_unwind_handler4 ENDP



OPTION LANGUAGE: SYSCALL

;; int __fastcall _EH4_CallFilterFunc(PFN_FilterFunc pfnFilter, BYTE *pb)
@_EH4_CallFilterFunc@8	PROC
	push	ebp
	push	esi
	push	edi
	push	ebx
	mov		ebp, edx		; pb
	xor		eax, eax
	xor		ebx, ebx
	xor		edx, edx
	xor		esi, esi
	xor		edi, edi
	call	ecx					; pfnFilter
	pop		ebx
	pop		edi
	pop		esi
	pop		ebp
	retn
@_EH4_CallFilterFunc@8	ENDP


;; __declspec(noreturn) void __fastcall _EH4_TransferToHandler(PFN_SpecificHandler pfn, BYTE *pb)
@_EH4_TransferToHandler@8	PROC SYSCALL
	mov		ebp, edx		;	pb
	mov		esi, ecx		; pfn
	mov		eax, ecx		; pfn
	push	1
	call	_NLG_Notify
	xor		eax, eax
	xor		ebx, ebx
	xor		ecx, ecx
	xor		edx, edx
	xor		edi, edi
	jmp		esi
@_EH4_TransferToHandler@8	ENDP

;!!!P
;; void __fastcall _EH4_GlobalUnwind2(PEXCEPTION_REGISTRATION_RECORD EstablisherFrame, PEXCEPTION_RECORD ExceptionRecord)
@_EH4_GlobalUnwind2@8 PROC SYS
	push    ebp
	mov     ebp, esp
	push    ebx
	push    esi
	push    edi
	push    0 			; ReturnValue
	push    edx             	; ExceptionRecord
	push    offset ReturnPoint 	; TargetIp
	push    ecx             	; TargetFrame
	call    API_RtlUnwind@16
ReturnPoint:
	pop     edi
        pop     esi
        pop     ebx
        pop     ebp
        retn
@_EH4_GlobalUnwind2@8 ENDP
;!!!P

;; void __fastcall _EH4_LocalUnwind(EXCEPTION_REGISTRATION *xreg, int tryLevel, BYTE *pb, uintptr_t *pSecCheck)
@_EH4_LocalUnwind@16	PROC SYSCALL
	push	ebp
	mov		ebp, [esp+8]								; pb
	INVOKE	_local_unwind4, [esp+20], ecx, edx			; pSecCheck, xreg,  tryLevel
	pop		ebp
	ret	8
@_EH4_LocalUnwind@16	ENDP




END
