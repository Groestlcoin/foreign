;/*######   Copyright (c) 2015 Ufasoft  http://ufasoft.com  mailto:support@ufasoft.com,  Sergey Pavlov  mailto:dev@ufasoft.com      ####
;#                                                                                                                                     #
;# 		See LICENSE for licensing information                                                                                          #
;#####################################################################################################################################*/

;; 	secp256k1_fe_mul_inner
;; 	secp256k1_fe_sqr_inner

IFDEF __JWASM__
	OPTION LANGUAGE: C
ENDIF

.CODE

fma_52_add MACRO	d, s, m
	mov		rax, s
	mul		rdi
	add		rax, d
	adc		rdx, 0
	add		r8, rax
	adc		rdx, 0
	mov		d, r8
	and		d, rbp
	shrd	r8, rdx, 52
ENDM


secp256k1_fe_common PROC
	mul		rsi
	add		rax, r8
	adc		rdx, 0
	mov		r9, rax
	and		r9, rbp
	shrd	rax, rdx, 52
	mov		rsi, rax

	mov		rdi, 01000003D10h	; load constant
	xor		r8, r8

	fma_52_add	r10, r15		; t5  +t0
	fma_52_add	r11, rbx		; t6  +t1
	fma_52_add	r12, rcx		; t7  +t2
	fma_52_add	r13, r9			; t8  +t3

	mov		rax, rsi			; t9  +t4
	mul		rdi
	add		rax, r14
	adc		rdx, 0
	add		rax, r8
	adc		rdx, 0
	mov		r14, 0FFFFFFFFFFFFh	;!!! 48 bits
	and		r14, rax	
	shrd	rax, rdx, 48			;!!!

	shr		rdi, 4
	mul		rdi
	add		rax, r10
	adc		rdx, 0
	and		rbp, rax
	shrd	rax, rdx, 52
	add		rax, r11

	mov		rbx, [esp+8]			; r
	mov		[rbx+0*8], rbp
	mov		[rbx+1*8], rax
	mov		[rbx+2*8], r12
	mov		[rbx+3*8], r13
	mov		[rbx+4*8], r14

	ret
secp256k1_fe_common ENDP

fma_f8_f9 MACRO	s, m
	mov		rax, s
	mul		m
	add		r8, rax
	adc		r9, rdx
ENDM



	;;  Procedure ExSetMult
	;;  Register Layout:
	;;  INPUT: 	rdi	= a->n
	;; 	   	rsi  	= b->n
	;; 	   	rdx  	= r->a
	;; 
	;;  INTERNAL:	rdx:rax  = multiplication accumulator
	;; 		r9:r8    = c
	;; 		r10-r13  = t0-t3
	;; 		r14	 = b.n[0] / t4
	;; 		r15	 = b.n[1] / t5
	;; 		rbx	 = b.n[2] / t6
	;; 		rcx	 = b.n[3] / t7
	;; 		rbp	 = Constant 0FFFFFFFFFFFFFh / t8
	;; 		rsi	 = b.n / b.n[4] / t9

secp256k1_fe_mul_inner PROC PUBLIC USES RBX RCX RDI RSI R12 R13 R14 R15 ;, r:QWORD, a:QWORD, b:QWORD
	push	rbp	
IFDEF __JWASM__
	push	rdi								; r
	mov		rdi, rsi						; a
	mov		rsi, rdx						; b
ELSE
	push	rcx								; r
	mov		rdi, rdx						; a
	mov		rsi, r8							; b
ENDIF


	mov		r14, [rsi+8*0]	; preload b.n[0]. This will be the case until
				; b.n[0] is no longer needed, then we reassign
				; r14 to t4
	;; c=a.n[0] * b.n[0]
   	mov		rax, [rdi+0*8]	; load a.n[0]
	mov		rbp, 0FFFFFFFFFFFFFh
	mul		r14			; rdx:rax=a.n[0]*b.n[0]
	mov		r15, [rsi+1*8]
	mov		r10, rbp		; load modulus into target register for t0
	mov		r8, rax
	and		r10, rax		; only need lower qword of c
	shrd	r8, rdx, 52

	;; c+=a.n[0] * b.n[1] + a.n[1] * b.n[0]
	xor		r9, r9	
	fma_f8_f9	[rdi+0*8], r15
	fma_f8_f9	[rdi+1*8], r14
	mov		r11, rbp
	and		r11, r8
	shrd	r8, r9, 52

	mov		rbx, [rsi+2*8]
	;; c+=a.n[0 1 2] * b.n[2 1 0]
	xor		r9, r9	
	fma_f8_f9	[rdi+0*8], rbx
	fma_f8_f9	[rdi+1*8], r15
	fma_f8_f9	[rdi+2*8], r14
	mov		r12, rbp		
	and		r12, r8		
	shrd	r8, r9, 52

	mov		rcx, [rsi+3*8]
	;; c+=a.n[0 1 2 3] * b.n[3 2 1 0]
	xor		r9, r9	
	fma_f8_f9	[rdi+0*8], rcx
	fma_f8_f9	[rdi+1*8], rbx
	fma_f8_f9	[rdi+2*8], r15
	fma_f8_f9	[rdi+3*8], r14
	mov		r13, rbp             
	and		r13, r8
	shrd	r8, r9, 52

	mov		rsi, [rsi+4*8]	; load b.n[4] and destroy pointer
	;; c+=a.n[0 1 2 3 4] * b.n[4 3 2 1 0]
	xor		r9, r9	
	fma_f8_f9	[rdi+0*8], rsi
	fma_f8_f9	[rdi+1*8], rcx
	fma_f8_f9	[rdi+2*8], rbx
	fma_f8_f9	[rdi+3*8], r15
	fma_f8_f9	[rdi+4*8], r14
	mov		r14, rbp             ; load modulus into t4 and destroy a.n[0]
	and		r14, r8
	shrd	r8, r9, 52

	;; c+=a.n[1 2 3 4] * b.n[4 3 2 1]
	xor		r9, r9	
	fma_f8_f9	[rdi+1*8], rsi
	fma_f8_f9	[rdi+2*8], rcx
	fma_f8_f9	[rdi+3*8], rbx
	fma_f8_f9	[rdi+4*8], r15
	mov		r15, rbp		
	and		r15, r8
	shrd	r8, r9, 52

	;; c+=a.n[2 3 4] * b.n[4 3 2]
	xor		r9, r9	
	fma_f8_f9	[rdi+2*8], rsi
	fma_f8_f9	[rdi+3*8], rcx
	fma_f8_f9	[rdi+4*8], rbx
	mov		rbx, rbp		
	and		rbx, r8		
	shrd	r8, r9, 52

	;; c+=a.n[3 4] * b.n[4 3]
	xor		r9, r9	
	fma_f8_f9	[rdi+3*8], rsi
	fma_f8_f9	[rdi+4*8], rcx
	mov		rcx, rbp		
	and		rcx, r8		
	shrd	r8, r9, 52

	mov		rax, [rdi+4*8]		;; c+=a.n[4] * b.n[4]
	call	secp256k1_fe_common

	pop		rax
	pop		rbp
	ret

secp256k1_fe_mul_inner ENDP

	
	;;  PROC ExSetSquare
	;;  Register Layout:
	;;  INPUT: 	rdi	 = a.n
	;; 	   	rsi  	 = this.a
	;;  INTERNAL:	rdx:rax  = multiplication accumulator
	;; 		r9:r8    = c
	;; 		r10-r13  = t0-t3
	;; 		r14	 = a.n[0] / t4
	;; 		r15	 = a.n[1] / t5
	;; 		rbx	 = a.n[2] / t6
	;; 		rcx	 = a.n[3] / t7
	;; 		rbp	 = 0FFFFFFFFFFFFFh / t8
	;; 		rsi	 = a.n[4] / t9

secp256k1_fe_sqr_inner PROC PUBLIC USES RBX RCX RDI RSI R12 R13 R14 R15 ;, r:QWORD, a:QWORD
	push	rbp	
IFDEF __JWASM__
	push	rdi							; r
	mov		rdi, rsi					; a
ELSE
	push	rcx							; r
	mov		rdi, rdx					; a
ENDIF
	mov		rbp, 0FFFFFFFFFFFFFh
	
		;; c=a.n[0] * a.n[0]
   	mov		r14, [rdi+0*8]	; r14=a.n[0]
	mov		r10, rbp		; modulus 
	mov		rax, r14
	mul		rax
	mov		r15, [rdi+1*8]	; a.n[1]
	add		r14, r14		; r14=2*a.n[0]
	mov		r8, rax
	and		r10, rax		; only need lower qword
	shrd	r8, rdx, 52

		;; c+=2*a.n[0] * a.n[1]
	mov		rax, r14		; r14=2*a.n[0]
	mul		r15
	mov		rbx, [rdi+2*8]	; rbx=a.n[2]
	mov		r11, rbp 		; modulus
	add		r8, rax
	adc		rdx, 0
	and		r11, r8
	shrd	r8, rdx, 52
	
	;; c+=2*a.n[0]*a.n[2]+a.n[1]*a.n[1]
	xor		r9, r9	
	fma_f8_f9	r14, rbx
	fma_f8_f9	r15, r15
	mov		r12, rbp
	and		r12, r8
	shrd	r8, r9, 52

	add		r15, r15		; r15=a.n[1]*2
	mov		rcx, [rdi+3*8]	; rcx=a.n[3]
	;; c+=2*a.n[0]*a.n[3]+2*a.n[1]*a.n[2]
	xor		r9, r9	
	fma_f8_f9	r14, rcx
	fma_f8_f9	r15, rbx
	mov		r13, rbp		; modulus
	and		r13, r8
	shrd	r8, r9, 52

	mov		rsi, [rdi+4*8]	; rsi=a.n[4]
	;; c+=2*a.n[0]*a.n[4]+2*a.n[1]*a.n[3]+a.n[2]*a.n[2]
	xor		r9, r9	
	fma_f8_f9	r14, rsi
	fma_f8_f9	r15, rcx
	fma_f8_f9	rbx, rbx
	mov		r14, rbp
	and		r14, r8
	shrd	r8, r9, 52

	add		rbx, rbx		; rcx=2*a.n[2]
	;; c+=2*a.n[1]*a.n[4]+2*a.n[2]*a.n[3]
	xor		r9, r9	
	fma_f8_f9	r15, rsi
	fma_f8_f9	rbx, rcx
	mov		r15, rbp		; modulus
	and		r15, r8
	shrd	r8, r9, 52

	;; c+=2*a.n[2]*a.n[4]+a.n[3]*a.n[3]
	xor		r9, r9	
	fma_f8_f9	rbx, rsi
	fma_f8_f9	rcx, rcx
	mov		rbx, rbp		; modulus
	and		rbx, r8		; only need lower dword
	shrd	r8, r9, 52

	lea		rax, [2*rcx]			; c+=2*a.n[3]*a.n[4]
	mul		rsi
	add		r8, rax
	adc		rdx, 0
	mov		rcx, r8
	and		rcx, rbp
	shrd	r8, rdx, 52

	mov		rax, rsi		;; c+=a.n[4]*a.n[4]
	call	secp256k1_fe_common

	pop		rax
	pop		rbp
	ret
secp256k1_fe_sqr_inner ENDP

	END

	
