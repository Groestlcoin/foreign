    page    ,132
    title   memset - set sections of memory all to one byte
;***
;memset.asm - set a section of memory to all one byte
;
;   Copyright (c) 1985-2001, Microsoft Corporation. All rights reserved.
;
;Purpose:
;   contains the memset() routine
;
;*******************************************************************************

include ksamd64.inc
        subttl  "memset"
;***
;char *memset(dst, value, count) - sets "count" bytes at "dst" to "value"
;
;Purpose:
;   Sets the first "count" bytes of the memory starting
;   at "dst" to the character value "value".
;
;   Algorithm:
;   char *
;   memset (dst, value, count)
;       char *dst;
;       char value;
;       unsigned int count;
;       {
;       char *start = dst;
;
;       while (count--)
;           *dst++ = value;
;       return(start);
;       }
;
;Entry:
;   char *dst - pointer to memory to fill with value
;   char value - value to put in dst bytes
;   int count - number of bytes of dst to fill
;
;Exit:
;   returns dst, with filled bytes
;
;Uses:
;
;Exceptions:
;
;*******************************************************************************
    extrn   __favor:dword
    EXTRN   __ImageBase:BYTE

__FAVOR_ENFSTRG equ 1
__FAVOR_XMMLOOP equ 2	

        LEAF_ENTRY_ARG3 memset, _TEXT, buf:ptr byte, value:byte, count:dword

        OPTION PROLOGUE:NONE, EPILOGUE:NONE

        mov     r11, rcx                ; save destination address
        movzx   edx, dl                 ; set fill pattern
        cmp     r8, 16                  ; check if 16 bytes to fill
        jb      SetBytes15              ; if b, count < 16 so use jmp table
	
        ;
        ;       Enhanced Fast Strings supported?
        ;
        bt      __favor, __FAVOR_ENFSTRG
        jnc     mset05                  ; no, so jump
        ;
        ;       use Enhanced Fast Strings
        push    rdi                     ; set up for rep
        mov     rdi, rcx                ; set destination
        mov     eax, edx                ; set byte to move
        mov     rcx, r8                 ; set count
        rep     stosb                   ; store the bytes
        pop     rdi                     ; restore registers and exit
        jmp short mset60                ; Done, Exit
mset05:
        mov     r9, 0101010101010101h   ; replicate fill over 8 bytes
        imul    rdx, r9                 ;
	
        bt      __favor, __FAVOR_XMMLOOP ; test if we can use sse
        jc      msetxmm10               ; yes, go to SSE implemenation
;
; Use old non-SSE implementation
;
        cmp     r8,64                   ; check if 64 bytes to fill
        jb short mset20                 ; if b, less than 64 bytes
;
; Large block - fill alignment bytes.
;

        neg     rcx                     ; compute bytes to alignment
        and     ecx, 7                  ;
        jz      short mset10            ; if z, no alignment required
        sub     r8, rcx                 ; adjust remaining bytes by alignment
        mov     [r11], rdx              ; fill alignment bytes
mset10: add     rcx, r11                ; compute aligned destination address

;
; Attempt to fill 64-byte blocks
;

        mov     r9, r8                  ; copy count of bytes remaining
        and     r8, 63                  ; compute remaining byte count
        shr     r9, 6                   ; compute number of 64-byte blocks
        jnz    short mset80             ; if nz, 64-byte blocks to fill

;
; Fill 8-byte bytes.
;

mset20: mov     r9, r8                  ; copy count of bytes remaining
        and     r8, 7                   ; compute remaining byte count
        shr     r9, 3                   ; compute number of 8-byte blocks
        jz      short mset40            ; if z, no 8-byte blocks

        db      066h, 066h, 066h, 090h, 090h

mset30: mov     [rcx], rdx              ; fill 8-byte blocks
        add     rcx, 8                  ; advance to next 8-byte block
        dec     r9                      ; decrement loop count
        jnz     short mset30            ; if nz, more 8-byte blocks

;
; Fill residual bytes.
;

mset40: test    r8, r8                  ; test if any bytes to fill
        jz      short mset60            ; if z, no bytes to fill
mset50: mov     [rcx], dl               ; fill byte
        inc     rcx                     ; advance to next byte
        dec     r8                      ; decrement loop count
        jnz     short mset50            ; if nz, more bytes to fill
mset60: mov     rax, r11
        ret                             ; return

;
; Fill 64-byte blocks.
;

        align   16

        db      066h, 066h, 066h, 090h
        db      066h, 066h, 090h

mset80: mov     [rcx], rdx              ; fill 64-byte block
        mov     8[rcx], rdx             ;
        mov     16[rcx], rdx            ;
        add     rcx, 64                 ; advance to next block
        mov     (24 - 64)[rcx], rdx     ;
        mov     (32 - 64)[rcx], rdx     ;
        dec     r9                      ; decrement loop count
        mov     (40 - 64)[rcx], rdx     ;
        mov     (48 - 64)[rcx], rdx     ;
        mov     (56 - 64)[rcx], rdx     ;
        jnz     short mset80            ; if nz, more 64-byte blocks
        jmp     short mset20            ; finish in common code

	align	16
;
; Fill using SSE instructions - size must be 16 or more.
;
	; rdx has the byte to store replicated to all byte positions
	; rcx has the destination, can be overwritten
	; r11 has the destination, must be preserved for return value
	; r8  has the count 
msetxmm10:	
        ;; using sse, set up xmm with 16 copies of the byte to store
        movd    xmm0, rdx               ; bytes to store in bits [0:63]
        punpcklbw xmm0, xmm0            ; dup bytes to [127:64]
        test    cl, 15                  ; test if destination aligned
        jz      short msetxmm20         ; if z, destination aligned

	; Aligned stores are much faster on AMD hardware. We need to do an unaligned
	; store of (16 - (dest mod 16)) bytes, but it's faster to just store 16 bytes 
	; and then start the aligned loop as usual at ((dest - (dest mod 16)) + 16).
	; This results in (dest mod 16) bytes being stored twice. This is a lot faster
	; than a bunch of code to store maybe 8 then maybe 4 then maybe 2 then maybe 1
	; byte to achieve alignement. It can cause data breakpoints to trigger twice,
	; but they will hit here first and hopefully read this comment.

        movups  [rcx], xmm0	        ; store 16 unaligned from start
	mov     rax, rcx	        ; copy dest addr
	and     rax, 15                 ; rax  = (dest mod 16)
	add     rcx, 16                 ; dest = dest + (16 - (dest mod 16))
        sub     rcx, rax
	lea     r8, (-16)[r8 + rax]     ; len  = len  - (16 - (dest mod 16))

;
; Attempt to set 128-byte blocks.
;
msetxmm20:              
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 7                   ; compute number of 128-byte blocks
        jz      msetxmm40               ; if z, no 128-byte blocks to fill
        jmp     msetxmm30               ; go move 128-byte blocks
;
; Set 128-byte blocks
;
        align   16

msetxmm30:      
        movaps  [rcx],    xmm0
        movaps  [rcx+16], xmm0
        add     rcx, 128                ; advance destination address early
        movaps  [rcx-96], xmm0  
        movaps  [rcx-80], xmm0
        dec     r9                      ; dec block counter (set cc for jnz)
        movaps  [rcx-64], xmm0
        movaps  [rcx-48], xmm0
        movaps  [rcx-32], xmm0
        movaps  [rcx-16], xmm0
        jnz     msetxmm30               ; loop if more blocks

        and     r8, 127                 ; compute remaining byte count
;
; Attempt to set 16-byte blocks
;
msetxmm40:              
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 4                   ; compute number of 16-byte blocks
        jz      short msetxmm60
;
; Set 16-byte blocks
;
        
        ;
        ; This generates an 8-byte nop, which we execute once. This will change only if
        ; any of the code from msetxmm30 down is modified. The following loop thus is
        ; completely contained within one instruction decode buffer on AMD hardware.
        ;       
        align   16
        
msetxmm50:
        movaps  [rcx], xmm0
        add     rcx, 16
        dec     r9
        jnz     short msetxmm50
msetxmm60:
	
	and    r8, 15		        ; compute remaining length
	jz     msetxmm70	        ; skip over movups if done, we could just do it anyway
	
	; As at the start, we are going to do an unaligned store of 16 bytes which will overwrite
	; some bytes already stored. The math is easier, rcx+r8 is one byte past the end, just
	; back up 16 from there and store 16.
        movups  [rcx+r8-16], xmm0       ; write remainder, overwriting 16-r8 bytes we already wrote

msetxmm70:		
        mov     rax, r11                ; must return original dest that we saved in r11
        ret

;
; Jump table for fills of 15 bytes or fewer
;
	; Preconditions:
	; rdx has the byte to fill and has been zero extended (ready for imul)
	; rcx has dest
	; r8 has len, r8 < 16
	; r11 has the dest
SetBytes15:
        mov     r9, 0101010101010101h   ; replicate fill over 8 bytes
        imul    rdx, r9                 ;
        lea     r9, OFFSET __ImageBase
        mov     eax, [(IMAGEREL  MsetTab) + r9 +r8*4]
        add     r9, rax
        add     rcx, r8                 ; rcx is now 1 past last byte to set
        mov     rax, r11                ; set return value
        jmp     r9

MsetTab dd  IMAGEREL msetTab00
        dd  IMAGEREL msetTab01
        dd  IMAGEREL msetTab02
        dd  IMAGEREL msetTab03
        dd  IMAGEREL msetTab04
        dd  IMAGEREL msetTab05
        dd  IMAGEREL msetTab06
        dd  IMAGEREL msetTab07
        dd  IMAGEREL msetTab08
        dd  IMAGEREL msetTab09
        dd  IMAGEREL msetTab10
        dd  IMAGEREL msetTab11
        dd  IMAGEREL msetTab12
        dd  IMAGEREL msetTab13
        dd  IMAGEREL msetTab14
        dd  IMAGEREL msetTab15

        align
        
        ; preconditions:      
        ; rcx points 1 byte beyond end of bytes to set
        ; rax has the correct return value (the original dest)
        ; each byte of the rdx reg is set to the byte to store
msetTab15:
        mov     (-15)[rcx], rdx
        ; fallthrough to 7
msetTab07:    
        mov     (-7)[rcx], edx
        ;; fallthrough to 3
msetTab03:    
        mov     (-3)[rcx], dx
        ; fallthrough to 1
msetTab01:    
        mov     (-1)[rcx], dl
msetTab00:    
        ret
msetTab11:
        mov     (-11)[rcx], rdx
        jmp     short msetTab03 ; adds zero latency due to branch prediction
msetTab14:
        mov     (-14)[rcx], rdx
        ; fallthrough to 6
msetTab06:    
        mov     (-6)[rcx], edx
        ; fallthrough to 2     
msetTab02:    
        mov     (-2)[rcx], dx
        ret
msetTab13:    
        mov     (-13)[rcx], rdx
        ; fallthrough to 5     
msetTab05:    
        mov     (-5)[rcx], edx
        mov     (-1)[rcx], dl
        ret
msetTab12:
        mov     (-12)[rcx], rdx
        ; fallthrough to 4
msetTab04:    
        mov     (-4)[rcx], edx
        ret
msetTab10:
        mov     (-10)[rcx], rdx
        mov     (-2)[rcx], dx
        ret
msetTab09:
        mov     (-9)[rcx], rdx
        mov     (-1)[rcx], dl
        ret
msetTab08:
        mov     (-8)[rcx], rdx
        ret

        LEAF_END memset, _TEXT

	end
