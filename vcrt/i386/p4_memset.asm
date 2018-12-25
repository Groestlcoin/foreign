INCLUDE vcruntime.inc

.CODE

OPTION LANGUAGE: C
.XMM

PUBLIC 	_VEC_memset
_VEC_memset    PROC
		test    eax, eax
		jnz     L_NotZero
		pxor    xmm0, xmm0
		jmp     L_Continue
L_NotZero:
		movd    xmm0, eax
		punpcklbw xmm0, xmm0
		punpcklwd xmm0, xmm0
		pshufd  xmm0, xmm0, 0
L_Continue:
		push    ebx
		push    ecx
		mov     ebx, ecx
		and     ebx, 0Fh
		test    ebx, ebx
		jnz     L_Notaligned
L_Aligned:
		mov     ebx, edx
		and     edx, 7Fh
		shr     ebx, 7
		jz      L_1a
L_1:
		movdqa  xmmword ptr [ecx], xmm0
		movdqa  xmmword ptr [ecx+10h], xmm0
		movdqa  xmmword ptr [ecx+20h], xmm0
		movdqa  xmmword ptr [ecx+30h], xmm0
		movdqa  xmmword ptr [ecx+40h], xmm0
		movdqa  xmmword ptr [ecx+50h], xmm0
		movdqa  xmmword ptr [ecx+60h], xmm0
		movdqa  xmmword ptr [ecx+70h], xmm0
		lea     ecx, [ecx+80h]
		dec     ebx
		jnz     L_1
L_1a:
		test    edx, edx
		jz      L_TrailReturn
		mov     ebx, edx
		shr     ebx, 4
		jz      L_Trailing
		jmp     L_2
L_2:
		movdqa  xmmword ptr [ecx], xmm0
		lea     ecx, [ecx+10h]
		dec     ebx
		jnz     L_2
L_Trailing:
		and     edx, 0Fh
		jz      L_TrailReturn
		mov     ebx, edx
		shr     edx, 2
		jz      L_TrailBytes
L_TrailDword:
		movd    dword ptr [ecx], xmm0
		lea     ecx, [ecx+4]
		dec     edx
		jnz     L_TrailDword
L_TrailBytes:
		and     ebx, 3
		jz      L_TrailReturn
L_TrailNextByte:
		mov     [ecx], al
		inc     ecx
		dec     ebx
		jnz     L_TrailNextByte
L_TrailReturn:
		pop     eax
		pop     ebx
		retn
L_Notaligned:
		neg     ebx
		add     ebx, 10h
		sub     edx, ebx
		push    edx
		mov     edx, ebx
		and     edx, 3
		jz      L_MovDword
L_Byte:
		mov     [ecx], al
		inc     ecx
		dec     edx
		jnz     L_Byte
L_MovDword:
		shr     ebx, 2
		jz      L_Adjustcnt
L_Dword:
		movd    dword ptr [ecx], xmm0
		lea     ecx, [ecx+4]
		dec     ebx
		jnz     L_Dword
L_Adjustcnt:
		pop     edx
		jmp     L_AligneD
_VEC_memset    ENDP


        END


