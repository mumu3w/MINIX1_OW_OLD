
global setjmp, longjmp
extern exit


; 	struct jmpbuf {
;		int bp;
;	 	int sp;
;	 	int ret-addr
; 	}

JMPERR  EQU     -99             ; call exit(JMPERR) when jmp-error

%include "../h/model.h"
        
segment _TEXT

setjmp:
	mov	bx,sp
	mov	ax,[bx]		; ret-addr.
	mov	bx,[bx+2]	; addr of jmp-struct
	mov	[bx],bp
	mov	[bx+2],sp
	mov	[bx+4],ax
	xor	ax,ax
	ret
        
longjmp:
	mov     bp,sp		; set new frame pointer to sp
	mov	bx,[bp+4]	; get address of jmp-structure
	mov	ax,[bp+6]	; get ret-code
	or	ax,ax
	jne	L1
	inc	ax		; return code may not be zero
    L1:	mov	sp,[bx+2]
	mov	bp,[bx]
	or	bp,bp		; test if last frame-pointer (error)
	jne	L2		; else execute the longjmp
	mov	ax,JMPERR
	push	ax
	call	exit		; should never return
	hlt
    L2:	mov	bx,[bx+4]
	jmp	[bx]

segment _DATABEG
        
segment _DATA

segment _DATAEND

segment _BSS

segment _BSSEND

;segment STACK
