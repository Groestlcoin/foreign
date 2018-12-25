small32 equ 1
flat32  equ 1

include pversion.inc
include cmacros.inc
include exsup.inc

.CODE

OPTION LANGUAGE: C

ASSUME FS:NOTHING 

EXTERN	__security_cookie	:NEAR
EXTERN _except_handler4		:NEAR

_SEH_prolog4	PROC PUBLIC
	push	offset _except_handler4
	push	fs:0
	mov     eax, [esp+8+8]
	mov     [esp+8+8], ebp
	lea     ebp, [esp+8+8]
	sub     esp, eax
	push    ebx
	push    esi
	push    edi
	mov		eax, __security_cookie
	xor     [ebp-4], eax
	xor     eax, ebp
	push	eax
	mov     [ebp-18h], esp
	push    [ebp-8]
	mov     eax, [ebp-4]
	mov     dword ptr [ebp-4], 0FFFFFFFFh
	mov     [ebp-8], eax
	lea     eax, [ebp-10h]
	mov     fs:0, eax
	ret
_SEH_prolog4	ENDP

_SEH_epilog4	PROC PUBLIC
	mov     ecx, [ebp-10h]
    mov     fs:0, ecx
    pop     ecx
    pop     edi
    pop     edi
    pop     esi
    pop     ebx
	mov		esp, ebp
	pop     ebp
    push    ecx
	ret
_SEH_epilog4	ENDP


		END
