INCLUDE el/x86x64.inc

_CIatan PROC
	fld1
	fpatan
	ret
_CIatan ENDP

_CIatan2 PROC
	fpatan
	ret
_CIatan2 ENDP

atan PROC CDECL, x: QWORD
	fld		x
	fld1
	fpatan
	ret
atan ENDP

atan2 PROC CDECL, y: QWORD, x: QWORD
	fld		y
	fld		x
	fpatan
	ret
atan2 ENDP

END

