       page    ,132
        title   memcpy - Copy source memory bytes to destination
;***
;memcpy.asm - contains memcpy and memmove routines
;
;       Copyright (c) Microsoft Corporation. All rights reserved.
;
;Purpose:
;       memcpy() copies a source memory buffer to a destination buffer.
;       Overlapping buffers are not treated specially, so propogation may occur.
;       memmove() copies a source memory buffer to a destination buffer.
;       Overlapping buffers are treated specially, to avoid propogation.
;
;*******************************************************************************

include ksamd64.inc
        subttl  "memcpy"

;***
;memcpy - Copy source buffer to destination buffer
;
;Purpose:
;       memcpy() copies a source memory buffer to a destination memory buffer.
;       This routine does NOT recognize overlapping buffers, and thus can lead
;       to propogation.
;       For cases where propogation must be avoided, memmove() must be used.
;
;       Algorithm:
;
;       void * memcpy(void * dst, void * src, size_t count)
;       {
;               void * ret = dst;
;
;               /*
;                * copy from lower addresses to higher addresses
;                */
;               while (count--)
;                       *dst++ = *src++;
;
;               return(ret);
;       }
;
;memmove - Copy source buffer to destination buffer
;
;Purpose:
;       memmove() copies a source memory buffer to a destination memory buffer.
;       This routine recognize overlapping buffers to avoid propogation.
;       For cases where propogation is not a problem, memcpy() can be used.
;
;   Algorithm:
;
;       void * memmove(void * dst, void * src, size_t count)
;       {
;               void * ret = dst;
;
;               if (dst <= src || dst >= (src + count)) {
;                       /*
;                        * Non-Overlapping Buffers
;                        * copy from lower addresses to higher addresses
;                        */
;                       while (count--)
;                               *dst++ = *src++;
;                       }
;               else {
;                       /*
;                        * Overlapping Buffers
;                        * copy from higher addresses to lower addresses
;                        */
;                       dst += count - 1;
;                       src += count - 1;
;
;                       while (count--)
;                               *dst-- = *src--;
;                       }
;
;               return(ret);
;       }
;
;
;Entry:
;       void *dst = pointer to destination buffer
;       const void *src = pointer to source buffer
;       size_t count = number of bytes to copy
;
;Exit:
;       Returns a pointer to the destination buffer in AX/DX:AX
;
;Uses:
;       CX, DX
;
;Exceptions:
;*******************************************************************************
    extrn   __favor:dword
    EXTRN   __ImageBase:BYTE
    EXTRN   __memcpy_nt_iters:QWORD     ; defined in cpu_disp.c

__FAVOR_ENFSTRG equ 1
__FAVOR_XMMLOOP equ 2

        public memmove

        LEAF_ENTRY_ARG3 memcpy, _TEXT, dst:ptr byte, src:ptr byte, count:dword 

        OPTION PROLOGUE:NONE, EPILOGUE:NONE

        memmove = memcpy

        mov     r11, rcx                ; save destination address
        mov     r10, rdx                ; save source address
        cmp     r8, 16                  ; if 16 bytes or less
        jbe     MoveBytes16             ; go move them quick
        sub     rdx, rcx                ; compute offset to source buffer
        jae     mcpy00aa                ; if above or equal, go move up
        mov     rax, r10                ; else check that src+count < dst
        add     rax, r8                 ; src + count
        cmp     rcx, rax                ; (src + count) < dst
        jl      mmov10                  ; no, buffers overlap go move downward
mcpy00aa:
                                        ; check for ENFSTRG (enhanced fast strings)
        bt      __favor, __FAVOR_ENFSTRG
        jnc     mcpy00b                 ; no jump
        ; use Enhanced Fast Strings
        ; but first align the destination dst to 16 byte alignment
        push    rdi                     ; need rdi rsi
        push    rsi
        mov     rdi, rcx                ; move destination adr
        mov     rsi, r10                ; move source adr
        mov     rcx, r8                 ; retrive length
        rep     movsb
        pop     rsi
        pop     rdi
        mov     rax, r11                ; set destination address
        ret                             ; done

mcpy00b:
        bt      __favor, __FAVOR_XMMLOOP ; test if using xmm loop
        jc      mcpyxmm10               ; yes, go do it using sse

;
; Move alignment bytes.
;
mcpy00a:
        test    cl, 7                   ; test if destination aligned
        jz      short mcpy20            ; if z, destination aligned
        test    cl, 1                   ; test if byte move needed
        jz      short mcpy00            ; if z, byte move not needed
        mov     al, [rcx + rdx]         ; move byte
        dec     r8                      ; decrement byte count
        mov     [rcx], al               ;
        inc     rcx                     ; increment destination address
mcpy00: test    cl, 2                   ; test if word move needed
        jz      short mcpy10            ; if z, word move not needed
        mov     ax, [rcx + rdx]         ; move word
        sub     r8, 2                   ; reduce byte count
        mov     [rcx], ax               ;
        add     rcx, 2                  ; advance destination address
mcpy10: test    cl, 4                   ; test if dword move needed
        jz      short mcpy20            ; if z, dword move not needed
        mov     eax, [rcx + rdx]        ; move dword
        sub     r8, 4                   ; reduce byte count
        mov     [rcx], eax              ;
        add     rcx, 4                  ; advance destination address

;
; Attempt to move 32-byte blocks.
;

mcpy20: mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 5                   ; compute number of 32-byte blocks
        jnz     mcpy70                  ; if nz, 32-byte blocks to fill

;
; Move 8-byte blocks.
;

mcpy25: mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 3                   ; compute number of 8-byte blocks
        jz      short mcpy40            ; if z, no 8-byte blocks
mcpy30: mov     rax, [rcx + rdx]        ; move 8-byte blocks
        mov     [rcx], rax              ;
        add     rcx, 8                  ; advance destination address
        dec     r9                      ; decrement loop count
        jnz     short mcpy30            ; if nz, more 8-byte blocks
        and     r8, 7                   ; compute remaining byte count

;
; Test for residual bytes.
;

mcpy40: test    r8, r8                  ; test if any bytes to move
        jnz     short mcpy50            ; if nz, residual bytes to move
        mov     rax, r11                ; set destination address
        ret                             ;

;
; Move residual bytes.
;

        align   16

mcpy50: lea     rdx, [rcx + rdx]        ; set up source
        mov     r10, rcx                ; and destination addresses
        jmp     MoveBytes16a            ; go move the bytes

MoveBytes16:
         mov    r10, r11                ; mov destination address to r10
MoveBytes16a:
         lea    r9, OFFSET __ImageBase
         mov    eax, [(IMAGEREL  MoveSmall) + r9 +r8*4]
         add    rax, r9
         jmp    rax

MoveSmall dd  IMAGEREL MoveSmall0 ;
          dd  IMAGEREL MoveSmall1 ;
          dd  IMAGEREL MoveSmall2 ;
          dd  IMAGEREL MoveSmall3 ;
          dd  IMAGEREL MoveSmall4 ;
          dd  IMAGEREL MoveSmall5 ;
          dd  IMAGEREL MoveSmall6 ;
          dd  IMAGEREL MoveSmall7 ;
          dd  IMAGEREL MoveSmall8 ;
          dd  IMAGEREL MoveSmall9 ;
          dd  IMAGEREL MoveSmall10
          dd  IMAGEREL MoveSmall11
          dd  IMAGEREL MoveSmall12
          dd  IMAGEREL MoveSmall13
          dd  IMAGEREL MoveSmall14
          dd  IMAGEREL MoveSmall15
          dd  IMAGEREL MoveSmall16


MoveSmall0::
        mov     rax, r11                ; set destination address
        ret

MoveSmall1::
        movzx   rax,byte ptr[rdx]       ; get first byte from source
        mov     [r10],al                ; write second byte to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall2::
        movzx    rax,word ptr[rdx]      ; get two byte from source
        mov     [r10],ax                ; write two bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall3::
        movzx   rax,byte ptr[rdx]       ; get one byte from source
        movzx   rcx,word ptr[rdx+1]     ; get two bytes from source
        mov     [r10],al                ; write one byte to destination
        mov     [r10+1],cx              ; write two bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall4::
        mov     eax,[rdx]               ; get four bytes from source
        mov     [r10],eax               ; write four bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall5::
        movzx   rax, byte ptr [rdx]     ; get one byte from source
        mov     ecx, dword ptr[rdx+1]   ; get four bytes from source
        mov     [r10],al                ; write one byte to destination
        mov     [r10+1],ecx             ; write four bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall6::
        movzx   rax, word ptr [rdx]     ; get two bytes from source
        mov     ecx,dword ptr[rdx+2]    ; get four bytes from source
        mov     [r10],ax                ; write two bytes to destination
        mov     [r10+2],ecx             ; write four bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall7::
        movzx   rax, byte ptr [rdx]     ; get first byte from source
        movzx   rcx, word ptr [rdx+1]   ; get two bytes from source
        mov     edx,  dword ptr[rdx+3]  ; get four bytes from source
        mov     byte ptr [r10],al       ; write one byte to destination
        mov     [r10+1],cx              ; write two bytes to destination
        mov     [r10+3],edx             ; write four bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall8::
        mov     rax, [rdx]              ; get eight bytes from source
        mov     [r10],rax               ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall9::
        movzx   rax,byte ptr[rdx]       ; get byte from source
        mov     rcx, [rdx+1]            ; get eight bytes from source
        mov     [r10],al                ; write byte to destination
        mov     [r10+1], rcx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall10::
        movzx   rax,word ptr[rdx]       ; get two bytes from source
        mov     rcx, [rdx+2]            ; get eight bytes from source
        mov     [r10],ax                ; write two bytes to destination
        mov     [r10+2], rcx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall11::
        movzx   rax,byte ptr[rdx]       ; get first byte from source
        movzx   rcx,word ptr[rdx+1]     ; get two bytes from source
        mov     rdx, qword ptr [rdx+3]  ; get eight bytes from source
        mov     [r10],al                ; write byte to destination
        mov     [r10+1],cx              ; write two bytes to destination
        mov     [r10+3], rdx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall12::
        mov     eax,dword ptr[rdx]      ; get four bytes from source
        mov     rcx,[rdx+4]             ; get eight bytes from source
        mov     [r10],eax               ; write four bytes to destination
        mov     [r10+4], rcx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall13::
        movzx   rax, byte ptr [rdx]     ; get byte from source
        mov     ecx, dword ptr[rdx+1]   ; get four bytes from source
        mov     rdx, qword ptr [rdx+5]  ; get eight bytes from source
        mov     [r10],al                ; write byte to destination
        mov     [r10+1],ecx             ; write four bytes  to destination
        mov     [r10+5], rdx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall14::
        movzx   rax, word ptr [rdx]     ; get two bytes from source
        mov     ecx, dword ptr[rdx+2]   ; get four bytes from source
        mov     rdx, qword ptr [rdx+6]  ; get eight bytes from source
        mov     [r10],ax                ; write two bytes to destination
        mov     [r10+2],ecx             ; write four bytes to destination
        mov     [r10+6], rdx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall15::
        movzx   r8, byte ptr [rdx]      ; get byte from source
        movzx   rax, word ptr [rdx+1]   ; get two bytes from source
        mov     ecx, dword ptr[rdx+3]   ; get four bytes from source
        mov     rdx, qword ptr [rdx+7]  ; get eight bytes from source
        mov     byte ptr [r10],r8b      ; write byte to destination
        mov     word ptr [r10+1],ax     ; write two bytes to destination
        mov     [r10+3], ecx            ; write four bytes to destination
        mov     [r10+7], rdx            ; write eight bytes to destination
        mov     rax, r11                ; set destination address
        ret
MoveSmall16::
        movdqu  xmm0, xmmword ptr [rdx] ; get sixteen byte from source
        movdqu  xmmword ptr [r10], xmm0 ; write sixteen byte to destination
        mov     rax, r11                ; set destination address
        ret

;
; Move 32 byte blocks
;

        align   16

mcpy70: mov     rax, [rcx + rdx]        ; move 32-byte block
        mov     r10, 8[rcx + rdx]       ;
        add     rcx, 32                 ; advance destination address
        mov     (-32)[rcx], rax         ;
        mov     (-24)[rcx], r10         ;
        mov     rax, (-16)[rcx + rdx]   ;
        mov     r10, (-8)[rcx + rdx]    ;
        dec     r9                      ;
        mov     (-16)[rcx], rax         ;
        mov     (-8)[rcx], r10          ;
        jnz     short mcpy70            ; if nz, more 32-byte blocks
        and     r8, 31                  ; compute remaining byte count
        jmp     mcpy25                  ;

;
; Memcpy up using SSE instructions.
;
; Preconditions:
;       destination in rcx (destructable) and r11 (must preserve for return value)
;       source in r10
;       length in r8, must be greater than 16
;       offset from dest to src in rdx
;       source addr > dest addr or else buffers don't overlap
;
mcpyxmm10:
        cmp     r8, 32                  ; check for length <= 32 (we know its > 16)
        jbe     Move17_32               ; go handle lengths 17-32 as a special case
;
; Aligned stores are much faster on AMD hardware, so start by moving however many
; bytes must be moved so updated dst is 16-byte aligned. We need to copy
; (16 - (dest mod 16)) bytes, but it's faster to just do an unaligned copy of 16
; bytes and then start the aligned loop as usual at ((dest - (dest mod 16)) + 16).
; This results in (dest mod 16) bytes being copied twice. This is a lot faster
; than a bunch of code to copy maybe 1 then maybe 2 then maybe 4 then maybe 8 
; bytes to achieve dst alignement.
;
; We know the src address is greater than the dst, but not by how much. In the
; case where the difference is less than 16 we must be careful about the bytes
; that will be stored twice. We must do both loads before either store, or the
; second load of those bytes will get the wrong values. We handle this by
; loading the last 16 bytes that can be stored at an aligned address, but 
; deferring the store of those bytes to the remainder code, so it can load the
; remainder before storing the deferred bytes. Since either or both of the two 
; loops can be skipped, the preconditions needed by the remainder  code must 
; also apply to the loops. These conditions are:
;  - r8 is the count remaining, not including the deferred bytes
;  - [rcx + rdx] and [rcx] as usual point to the src and dst where the number
;    number of bytes given by r8 should be copied from and to.
;  - xmm0 holds the 16 deferred bytes that need to be stored at (-16)[rcx]
;
        test    cl, 15                  ; test if destination aligned
        jnz     short mcpyxmm20         ; if nz, alignment move needed
;
; dst is already aligned, just set up preconditions for loops or remainder code
;
        movups  xmm0, [rcx + rdx]       ; load deferred bytes
        add     rcx, 16
        sub     r8, 16
        jmp     mcpyxmm30               ; go try 128-byte blocks
;
; Move alignment bytes.
;
mcpyxmm20:
        movups  xmm1, [rcx + rdx]       ; load first 16 bytes unaligned
        add     rcx, 32
        and     cl, 0F0h                ; rcx is 16 bytes past first 16-byte align point
        movups  xmm0, (-16)[rcx + rdx]  ; load deferred-store bytes
        movups  [r11], xmm1             ; now safe to store 16 unaligned at start
        mov     rax, rcx
        sub     rax, r11                ; rax is num bytes from start to rcx
        sub     r8, rax                 ; update count remaining
;
; See if we can move any 128-byte blocks.
;
mcpyxmm30:
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 7                   ; compute number of 128-byte blocks
        jz      short mcpyxmm60         ; if z jump around to 2nd loop
        movaps  (-16)[rcx], xmm0        ; going into 1st loop, ok to store deferred bytes
        cmp     r9, __memcpy_nt_iters   ; threshold defined by cpu_disp.c
        jg      mcpynt20                ; long enough so non-temporal worth it, jump into nt loop
        jmp     short mcpyxmm50         ; too short for non-temporal, jump into regular loop

;
; Move 128-byte blocks
;
        align   16
;
; When possible, non-mov instructions are put between a load and store
; so their execution can overlap the store. 
; The jnz is likewise moved earlier to come before the last store pair.
; Pairs of loads/stores are used to overlap cache latencies.
; movups and movaps are equally fast on aligned storage, we use movaps
; to document movs that we *know* are going to be aligned, movups otherwise.
; xmm0 must be preloaded before jumping into this loop, and the last
; store must be deferred (and the bytes to store left in xmm0) for the
; following loop and/or the remainder code.
;
mcpyxmm40:      
        movaps  (-32)[rcx], xmm0        ; store 7th chunk from prior iteration
        movaps  (-16)[rcx], xmm1        ; store 8th chunk from prior iteration
mcpyxmm50:                              ; enter loop here with xmm0 preloaded.
        movups  xmm0, [rcx + rdx]       ; load first 16 byte chunk
        movups  xmm1, 16[rcx + rdx]     ; load 2nd 16 byte chunk
        add     rcx, 128                ; advance destination address
        movaps  (-128)[rcx], xmm0       ; store first 16 byte chunk
        movaps  (-112)[rcx], xmm1       ; store 2nd 16 byte chunk
        movups  xmm0, (-96)[rcx + rdx]  ; load 3rd chunk
        movups  xmm1, (-80)[rcx + rdx]  ; load 4th chunk
        dec     r9                      ; dec block counter (set cc for jnz)
        movaps  (-96)[rcx], xmm0        ; store 3rd chunk
        movaps  (-80)[rcx], xmm1        ; store 4th chunk
        movups  xmm0, (-64)[rcx + rdx]  ; load 5th chunk
        movups  xmm1, (-48)[rcx + rdx]  ; load 6th chunk
        movaps  (-64)[rcx], xmm0        ; store 5th chunk
        movaps  (-48)[rcx], xmm1        ; store 6th chunk
        movups  xmm0, (-32)[rcx + rdx]  ; load 7th chunk
        movups  xmm1, (-16)[rcx + rdx]  ; load 8th chunk
        jnz     mcpyxmm40               ; loop if more blocks

mcpyxmm55:                              ; non-temporal codepath rejoins here
        movaps  (-32)[rcx], xmm0        ; store 7th chunk from final iteration
        and     r8, 127                 ; compute remaining byte count
        movaps  xmm0, xmm1              ; 8th chunk becomes deferred bytes
;
; See if we have any 16-byte blocks left to move
; 
mcpyxmm60:      
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 4                   ; compute number of 16-byte blocks
        jz      short mcpyxmm80         ; on z, no 16-byte blocks, skip 2nd loop

        align   16

mcpyxmm70:      
        movaps  (-16)[rcx], xmm0        ; the first time through this is the 
                                        ; store of the deferred bytes from above
        movups  xmm0, [rcx + rdx]       ; load a block
        add     rcx, 16                 ; advance dest addr (store is deferred)
        dec     r9
        jnz     mcpyxmm70

mcpyxmm80:      
        and     r8, 15                  ; compute remaining byte count
        jz      short mcpyxmm90         ; if z, no remainder bytes to move
;
; Handle remainder bytes.
;
; As at the start, we are going to do an unaligned copy of 16 bytes which will double-write
; some bytes.  We must not touch rcx or xmm0 because they have what we need to store the
; deferred block. We use rax to point to the first byte after the end of the buffer and
; back up from there. Note rax is pointing to an address we must not read or write!
;
        lea     rax, [rcx+r8]           ; make rax point one past the end
        movups  xmm1, (-16)[rax + rdx]  ; load last 16 bytes of source buffer
        movups  (-16)[rax], xmm1        ; write last 16 bytes, including 16-r8 bytes
                                        ; from the last aligned block which we are about to
                                        ; overstore with identical values
mcpyxmm90:
        movaps  (-16)[rcx], xmm0        ; store the last deferred aligned block
        mov     rax, r11                ; we must return the original destination address
        ret                             ;
;
; Move 128-byte blocks non-temporal
;
        align   16
        nop
        align   16
;
; non-temporal is exactly the same as the regular xmm loop above, except the movaps
; stores are movntps and we use prefetchnta. We are prefetching in two places, each
; prefetch gets 64 bytes about half an iteration ahead of time (about 10 instructions
; lead time). When we come to the end of the memcpy, we'll be prefetching bytes
; beyond the buffer we need to copy from, which may not be valid bytes. This is         
; not illegal; if the memory address is invalid it does not trap, the hardware treats
; illegal prefetches as nops.
;
        
mcpynt10:
        movntps (-32)[rcx], xmm0        ; store 7th chunk from prior iteration
        movntps (-16)[rcx], xmm1        ; store 8th chunk from prior iteration
mcpynt20:                               ; enter loop here with xmm0 preloaded.
        prefetchnta [rcx + rdx + 512]   ; prefetch several cache lines ahead
        movups  xmm0, [rcx + rdx]       ; load first 16 byte chunk
        movups  xmm1, 16[rcx + rdx]     ; load 2nd 16 byte chunk
        add     rcx, 128                ; advance destination address
        movntps (-128)[rcx], xmm0       ; store first 16 byte chunk
        movntps (-112)[rcx], xmm1       ; store 2nd 16 byte chunk
        movups  xmm0, (-96)[rcx + rdx]  ; load 3rd chunk
        movups  xmm1, (-80)[rcx + rdx]  ; load 4th chunk
        dec     r9                      ; dec block counter (set cc for jnz)
        movntps (-96)[rcx], xmm0        ; store 3rd chunk
        movntps (-80)[rcx], xmm1        ; store 4th chunk
        movups  xmm0, (-64)[rcx + rdx]  ; load 5th chunk
        movups  xmm1, (-48)[rcx + rdx]  ; load 6th chunk
        prefetchnta [rcx + rdx + 576]   ; prefetch several cache lines ahead
        movntps (-64)[rcx], xmm0        ; store 5th chunk
        movntps (-48)[rcx], xmm1        ; store 6th chunk
        movups  xmm0, (-32)[rcx + rdx]  ; load 7th chunk
        movups  xmm1, (-16)[rcx + rdx]  ; load 8th chunk
        jnz     mcpynt10                ; loop if more blocks
        sfence
        jmp     mcpyxmm55               ; rejoin regular memcpy codepath

;
; When using xmm registers, handle lengths 17-32 as a special case.
; This allows the regular code to assume that there will always be enough
; bytes for the "deferred" block of 16. Also any case that can be handled
; with just two stores is handled with just two stores, the regular code
; will always do 3 stores for unaligned moves that have a remainder.
; No assumptions are made here about buffer alignment or overlap.
; We load the entire string to be moved in 2 xmm registers before storing
; anything, so this works for any arrangement of overlapping buffers.
;
; dst is in rcx (can modify) and r11 (must preserve for return value)
; src is in r10 (should preserve for consistency)
; rdx is the offset from the dst to the source, so rcx + rdx is the src
; r8 is the length, and is known to be 17 <= r8 <= 32
;
; When length < 32 the first 16 bytes includes some of the last 16 bytes
; and we will store (length - 32) bytes twice. (E.g. in the worst case
; of len 17 we are storing the middle 15 bytes of the buffer twice).
; This is still much faster than doing logic and branching with 1, 2, 4
; and 8 byte conditional copies.
;
        align   16

Move17_32:
        movups  xmm0, [r10]             ; load first 16 bytes of src
        lea     rcx, (-16)[rcx + r8]    ; make rcx point to last 16 bytes of dst
        movups  xmm1, [rcx + rdx]       ; load last 16 bytes of src
        movups  [r11], xmm0             ; store first 16 bytes of dst
        movups  [rcx], xmm1             ; store last 16 bytes of dst
        mov     rax, r11                ; set destination address
        ret

;
; The source address is less than the destination address.
;

        align   16

        db      066h, 066h, 066h, 090h
        db      066h, 066h, 066h, 090h
        db      066h, 090h

mmov10: 
        bt      __favor, __FAVOR_XMMLOOP ; test if using xmm loop
        jc      mmovxmm10               ; yes, go use xmm method
        add     rcx, r8                 ; compute ending destination address

;
; Move alignment bytes.
;

        test    cl, 7                   ; test if destination aligned
        jz      short mmov30            ; if z, destination aligned
        test    cl, 1                   ; test if byte move needed
        jz      short mmov15            ; if z, byte move not needed
        dec     rcx                     ; decrement destination address
        mov     al, [rcx + rdx]         ; move byte
        dec     r8                      ; decrement byte count
        mov     [rcx], al               ;
mmov15: test    cl, 2                   ; test if word move needed
        jz      short mmov20            ; if z, word move not needed
        sub     rcx, 2                  ; reduce destination address
        mov     ax, [rcx + rdx]         ; move word
        sub     r8, 2                   ; reduce byte count
        mov     [rcx], ax               ;
mmov20: test    cl, 4                   ; test if dword move needed
        jz      short mmov30            ; if z, dword move not needed
        sub     rcx, 4                  ; reduce destination address
        mov     eax, [rcx + rdx]        ; move dword
        sub     r8, 4                   ; reduce byte count
        mov     [rcx], eax              ;

;
; Attempt to move 32-byte blocks
;

mmov30: mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 5                   ; compute number of 32-byte blocks
        jnz     short mmov90            ; if nz, 32-byte blocks to fill

;
; Move 8-byte blocks.
;

mmov40: mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 3                   ; compute number of 8-byte blocks
        jz      short mmov60            ; if z, no 8-byte blocks
mmov50: sub     rcx, 8                  ; reduce destination address
        mov     rax, [rcx + rdx]        ; move 8-byte blocks
        dec     r9                      ; decrement loop count
        mov     [rcx], rax              ;
        jnz     short mmov50            ; if nz, more 8-byte blocks
        and     r8, 7                   ; compute remaining byte count

;
; Test for residual bytes.
;

mmov60: test    r8, r8                  ; test if any bytes to move
        jnz     short mmov70            ; if nz, residual bytes to move
        mov     rax, r11                ; set destination address
        ret                             ;

;
; Move residual bytes.
;

        align   16

mmov70: sub     rcx, r8
        mov     r10, rcx
        lea     rdx, [rcx + rdx]
        jmp     MoveBytes16a

;
; Move 32 byte blocks
;

        align   16

mmov90: mov     rax, (-8)[rcx + rdx]    ; move 32-byte block
        mov     r10, (-16)[rcx + rdx]   ;
        sub     rcx, 32                 ; reduce destination address
        mov     24[rcx], rax            ;
        mov     16[rcx], r10            ;
        mov     rax, 8[rcx + rdx]       ;
        mov     r10, [rcx + rdx]        ;
        dec     r9                      ;
        mov     8[rcx], rax             ;
        mov     [rcx], r10              ;
        jnz     short mmov90            ; if nz, more 32-byte blocks
        and     r8, 31                  ; compute remaining byte count
        jmp     mmov40                  ;

;
; Move bytes down using SSE registers. The source address is less than 
; the destination address and the buffers overlap. We will do everything back-to-front. 
;
; Preconditions:
;       destination is r11 (must preserve for return value) and rcx
;       source in r10 (must preserve for remainder move)
;       length in r8, must have been verified to be greater than 16
;       offset from dest to src in rdx
;       source addr < dest addr and the buffers overlap
;

mmovxmm10:
        cmp     r8, 32                  ; check for length <= 32 (we know its > 16)
        jbe     Move17_32               ; go handle lengths 17-32 as a special case
        add     rcx, r8                 ; make rcx point one past the end of the dst buffer
;
; Aligned stores using movaps or movups are faster on AMD hardware than unaligned
; stores using movups. To achieve 16-byte dest alignment, we do an unaligned move
; of the last 16 bytes of the buffers, then reduce rcx only by the amount necessary
; to achieve alignment. This results in some bytes getting copied twice, unless we're
; already aligned.
; 
; We know the src address is less than the dst, but not by exactly how much. In the
; case where the difference is less than 16 we must be careful about the bytes
; that will be stored twice. We must do both loads before either store, or the
; second load of those bytes will get the wrong values. We handle this by
; deferring the store of 16 aligned bytes to the remainder code, so it can load the
; remainder before storing the deferred bytes. Since either or both of the two 
; loops can be skipped, the preconditions needed by the remainder  code must 
; also apply to the loops. These conditions are:
;  - r8 is the count remaining, not including the deferred bytes
;  - [rcx] points one past the end of the remainder bytes
;  - rdx is the offset from the dst to the source
;  - xmm0 holds the 16 deferred bytes that need to be stored at [rcx]
;
        test    cl, 15                  ; test if dest aligned
        jnz     short mmovxmm20         ; if nz, alignment move needed
;
; dst is already aligned, just set up preconditions for loops or remainder code
;
        sub     rcx, 16                 ; reduce dst addr
        movups  xmm0, [rcx + rdx]       ; load deferred bytes
        sub     r8, 16
        jmp     mmovxmm30               ; go try 128-byte blocks
;
; Move alignment bytes.
;
mmovxmm20:
        sub     rcx, 16                 ; reduce destination address first
        movups  xmm1, [rcx + rdx]       ; load last 16 bytes for unaligned store
        mov     rax, rcx                ; save unaligned store address
        and     cl, 0F0h                ; rcx is deferred store address
        movups  xmm0, [rcx + rdx]       ; load deferred-store bytes
        movups  [rax], xmm1             ; now safe to do unaligned store
        mov     r8, rcx                 ; easier to recalc r8 using rcx-r11 ...
        sub     r8, r11                 ; ... than calc how much to subtract from r8

;
; See if we can move any 128-byte blocks.
;
mmovxmm30:
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 7                   ; compute number of 128-byte blocks
        jz      short mmovxmm60         ; if z jump around to 2nd loop
        movaps  [rcx], xmm0             ; going into 1st loop, ok to store deferred bytes
        jmp     short mmovxmm50         ; jump into 1st loop
;
; Move 128-byte blocks
;
        align   16

mmovxmm40:
        movaps  (128-112)[rcx], xmm0    ; store 7th chunk from prior iteration
        movaps  (128-128)[rcx], xmm1    ; store 8th chunk from prior iteration
mmovxmm50:
        movups  xmm0, (-16)[rcx + rdx]      ; load first 16 byte chunk
        movups  xmm1, (-32)[rcx + rdx]      ; load 2nd 16 byte chunk
        sub     rcx, 128                    ; reduce destination address
        movaps  (128-16)[rcx], xmm0         ; store first 16 byte chunk
        movaps  (128-32)[rcx], xmm1         ; store 2nd 16 byte chunk
        movups  xmm0, (128-48)[rcx + rdx]   ; load 3rd chunk
        movups  xmm1, (128-64)[rcx + rdx]   ; load 4th chunk
        dec     r9                          ; dec block counter (set cc for jnz)
        movaps  (128-48)[rcx], xmm0         ; store 3rd chunk
        movaps  (128-64)[rcx], xmm1         ; store 4th chunk
        movups  xmm0, (128-80)[rcx + rdx]   ; load 5th chunk
        movups  xmm1, (128-96)[rcx + rdx]   ; load 6th chunk
        movaps  (128-80)[rcx], xmm0         ; store 5th chunk
        movaps  (128-96)[rcx], xmm1         ; store 6th chunk
        movups  xmm0, (128-112)[rcx + rdx]  ; load 7th chunk
        movups  xmm1, (128-128)[rcx + rdx]  ; load 8th chunk
        jnz     short mmovxmm40             ; loop if more blocks

        movaps  (128-112)[rcx], xmm0    ; store 7th chunk from final iteration
        and     r8, 127                 ; compute remaining byte count
        movaps  xmm0, xmm1              ; 8th chunk becomes deferred bytes
;
; See if we have any 16-byte blocks left to move
; 
mmovxmm60:        
        mov     r9, r8                  ; copy count of bytes remaining
        shr     r9, 4                   ; compute number of 16-byte blocks
        jz      short mmovxmm80         ; if z, no 16-byte blocks

        align   16

mmovxmm70:        
        movaps  [rcx], xmm0             ; the first time through this is the 
                                        ; store of the deferred bytes from above
        sub     rcx, 16                 ; reduce dest addr
        movups  xmm0, [rcx + rdx]       ; load a block
        dec     r9
        jnz     mmovxmm70

mmovxmm80:        
        and     r8, 15                  ; compute remaining byte count
        jz      short mmovxmm90         ; if z, no residual bytes to move

;
; Handle remainder bytes.
;
; As at the start, we are going to do an unaligned copy of 16 bytes which will double-write
; some bytes.  We must not touch rcx or xmm0 because they have what we need to store the
; deferred block. But unlike for mcpyxmm code above, we have r10 and r11 we can just use
; to copy the lowest 16 bytes.
;
        movups  xmm1, [r10]             ; load lowest 16 bytes, which includes remainder
        movups  [r11], xmm1             ; store lowest 16 bytes, which includes remainder

mmovxmm90:
        movaps  [rcx], xmm0             ; store deferred bytes
        mov     rax, r11                ; we must return destination address
        ret                             ;

        LEAF_END memcpy, _TEXT

        end
