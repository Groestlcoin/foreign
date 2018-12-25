INCLUDE el/x86x64.inc

.CODE

_CIacos PROC
	fld1
	fadd	st, st(1)
	fld1
	fsub    st, st(2)
	fmulp
	fsqrt
	fxch
	fpatan
	ret
_CIacos ENDP

acos PROC CDECL, x: QWORD
	fld		x
	call	_CIacos
	ret
acos ENDP

END
