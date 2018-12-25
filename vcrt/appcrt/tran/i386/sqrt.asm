INCLUDE el/x86x64.inc

_CIsqrt PROC
	fsqrt
	ret
_CIsqrt ENDP

sqrt PROC CDECL, x: QWORD
	fld		x
	call	_CIsqrt
	ret
sqrt ENDP


END
