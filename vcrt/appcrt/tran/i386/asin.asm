INCLUDE el/x86x64.inc

_CIasin PROC
	fld1
	fadd	st, st(1)
	fld1
	fsub    st, st(2)
	fmulp
	fsqrt
	fpatan
	ret
_CIasin ENDP

asin PROC CDECL, x: QWORD
	fld		x
	call	_CIasin
	ret
asin ENDP


END

