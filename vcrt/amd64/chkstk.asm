INCLUDE el/x86x64.inc

        TITLE   "Runtime Stack Checking"
;++
;
; Copyright (c) Microsoft Corporation. All rights reserved.
;
; You may only use this code if you agree to the terms of the Windows Research Kernel Source Code License agreement (see License.txt).
; If you do not agree to the terms, do not use the code.
;
;
; Module Name:
;
;   chkstk.s
;
; Abstract:
;
;   This module implements runtime stack checking.
;
;--

include ksamd64.inc

        subttl  "Check Stack"
;++
;
; ULONG64
; __chkstk (
;     VOID
;     )
;
; Routine Description:
;
;   This function provides runtime stack checking for local allocations
;   that are more than a page and for storage dynamically allocated with
;   the alloca function. Stack checking consists of probing downward in
;   the stack a page at a time. If the current stack commitment is exceeded,
;   then the system will automatically attempts to expand the stack. If the
;   attempt succeeds, then another page is committed. Otherwise, a stack
;   overflow exception is raised. It is the responsibility of the caller to
;   handle this exception.
;
;   N.B. This routine is called using a non-standard calling sequence since
;        it is typically called from within the prologue. The allocation size
;        argument is in register rax and it must be preserved. Registers r10
;        and r11 are used by this function, but are preserved.
;
;        The typical calling sequence from the prologue is:
;
;        mov    rax, allocation-size    ; set requested stack frame size
;        call   __chkstk                ; check stack page allocation
;        sub    rsp, rax                ; allocate stack frame
;
; Arguments:
;
;   None.
;
; Implicit Arguments:
;
;   Allocation (rax) - Supplies the size of the allocation on the stack.
;
; Return Value:
;
;   The allocation size is returned as the function value.
;
;--


__chkstk	PROC
	push	r11
	push	r10
	xor		r11, r11
	lea		r10, [rsp+24]
	sub		r10, rax
	cmovb	r11, r10			; if (CF) sp=nullptr
	mov		r11, gs:[10h]			; StackLimit
	cmp		r10, r11
	jae		cs20
	and		r10w, NOT(PAGE_SIZE-1)
cs10:
	sub		r11, PAGE_SIZE			; PageSize
	mov		BYTE PTR [r11], 0
	cmp		r10, r11
	jne		cs10
cs20:
	pop		r10
	pop		r11
	ret
__chkstk	ENDP

END
