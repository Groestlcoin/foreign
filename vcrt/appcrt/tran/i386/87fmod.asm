INCLUDE el/x86x64.inc

.CODE

_CIfmod PROC
	fxch
remloop:
	fprem
	fstsw	ax
	sahf
	jp		remloop
	fstp	st(1)
	ret
_CIfmod ENDP

fmod PROC CDECL, x: QWORD, y: QWORD
	fld		x
	fld		y	
	call	_CIfmod
	ret
fmod ENDP

END
