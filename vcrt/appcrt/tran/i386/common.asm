INCLUDE el/x86x64.inc

_twoToTOS PROC
	fld		st
	frndint
	fsub	st(1), st
	fxch
	f2xm1
	fld1
	fadd
	fscale
	fstp	st(1)
	ret
_twoToTOS ENDP

.CONST

_pi_by_2_to_61 DT   403EC90FDAA22168C235h			; PI * 2^61

PUBLIC	_pi_by_2_to_61

END

