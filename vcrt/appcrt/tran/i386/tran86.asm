.MODEL FLAT
.CODE
OPTION LANGUAGE: C

EXTERN	_CIsin		:NEAR
EXTERN	_CIcos		:NEAR
EXTERN	_CIexp		:NEAR
EXTERN	_CIlog		:NEAR
EXTERN	_CIlog10	:NEAR
EXTERN _87_exp10	:NEAR

sin PROC
	fld		QWORD PTR [esp + 4]
	jmp		_CIsin
sin ENDP

cos PROC
	fld		QWORD PTR [esp + 4]
	jmp		_CIcos
cos ENDP

exp PROC
	fld		QWORD PTR [esp + 4]
	jmp		_CIexp
exp ENDP

log PROC
	fld		QWORD PTR [esp + 4]
	jmp		_CIlog
log ENDP

log10 PROC
	fld		QWORD PTR [esp + 4]
	jmp		_CIlog10
log10 ENDP

log2 PROC
	fld1
	fld		QWORD PTR [esp + 4]
	fyl2x
	ret
log2 ENDP

exp10	PROC
	fld		QWORD PTR [esp + 4]
	jmp		_87_exp10
exp10	ENDP


END
