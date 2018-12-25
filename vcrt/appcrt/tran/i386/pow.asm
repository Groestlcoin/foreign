INCLUDE el/x86x64.inc

EXTERN	_twoToTOS	:NEAR

_CIpow PROC
	fxch
	fyl2x
	jmp		_twoToTOS
_CIpow ENDP

pow PROC CDECL, x: QWORD, y:QWORD
	fld		x
	fld		y	
	call	_CIpow
	ret
pow ENDP

END
