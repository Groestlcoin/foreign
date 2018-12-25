small32 equ 1
flat32  equ 1

include pversion.inc
include cmacros.inc
include exsup.inc

.CODE

OPTION LANGUAGE: C

ASSUME FS:NOTHING 

extrn _except_handler3:near

_SEH_prolog	PROC PUBLIC
	push	offset _except_handler3
	mov     eax, fs:0
	push    eax
	mov     eax, [esp+8+8]
	mov     [esp+8+8], ebp
	lea     ebp, [esp+8+8]
	sub     esp, eax
	push    ebx
	push    esi
	push    edi
	mov     eax, [ebp-8]
	mov     [ebp-18h], esp
	push    eax
	mov     eax, [ebp-4]
	mov     dword ptr [ebp-4], 0FFFFFFFFh
	mov     [ebp-8], eax
	lea     eax, [ebp-10h]
	mov     fs:0, eax
	retn
_SEH_prolog	ENDP

_SEH_epilog	PROC PUBLIC
	mov     ecx, [ebp-10h]
	mov     fs:0, ecx
	pop     ecx
	pop     edi
	pop     esi
	pop     ebx
	leave
	push    ecx
	retn
_SEH_epilog	ENDP


		END
