INCLUDE el/x86x64.inc

.XMM

extern __isa_available: DWORD

public _ftol2_sse

_ftol2_sse:
		cmp	__isa_available, 1
		jz	_ftol2

_ftol2_pentium4 PROC PUBLIC
		local	q: QWORD
		
		fstp		q
		cvttsd2si 	eax, [q]
		ret
_ftol2_pentium4  ENDP

_ftol2	PROC PUBLIC
		local	f: DWORD
		local	n: QWORD

		fld		st
		fst		f
		fistp	n
		fild	n
		mov		eax, dword ptr n
		test	eax, eax
		jnz		arg_is_not_integer_QnaN
		mov		edx, dword ptr n+4
		test	edx, 7FFFFFFFh
		jnz		arg_is_not_integer_QnaN
		fstp	f
		fstp	f
		jmp		localexit
arg_is_not_integer_QnaN:
		fsubp
		mov		edx, f
		fstp	f
		test	edx, edx
		mov		edx, dword ptr n+4
		jns		positive
		xor		f, 80000000h
		add		f, 7FFFFFFFh
		adc		eax, 0
		adc		edx, 0
		jmp		localexit
positive:
		add		f, 7FFFFFFFh
		sbb		eax, 0
		sbb		edx, 0
localexit:
		ret
_ftol2	ENDP


	END
