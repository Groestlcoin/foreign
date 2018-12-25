INCLUDE el/x86x64.inc

EXTERN	_pi_by_2_to_61	:TBYTE

_CIcos PROC
	fcos
	fstsw	ax
	sahf
	jnp		lab_ret
	fld		_pi_by_2_to_61
	fxch
redux_loop:
	fprem1
	fstsw	ax
	sahf
	jp		redux_loop
	fstp	st(1)
	fcos
lab_ret:
	ret
_CIcos ENDP

END
