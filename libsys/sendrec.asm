
global _send, _receive, _send_rec, _sendrec


; The following definitions are from  ../h/com.h
SEND    equ     1
RECEIVE equ     2
BOTH	equ     3
SYSVEC	equ     32
        

%include "../h/model.h"
        

segment _TEXT
;========================================================================
;                           send and receive                              
;========================================================================
; send(), receive(), send_rec() all save bp, but destroy ax, bx, and cx.

_send:	mov cx,SEND		; send(dest, ptr)
	jmp L0

_receive:
	mov cx,RECEIVE		; receive(src, ptr)
	jmp L0

_sendrec:
_send_rec:
	mov cx,BOTH		; send_rec(srcdest, ptr)
	jmp L0

  L0:	push bp			; save bp
	mov bp,sp		; can't index off sp
	mov ax,[bp+4]		; ax = dest-src
	mov bx,[bp+6]		; bx = message pointer
	int SYSVEC		; trap to the kernel
	pop bp			; restore bp
	ret			; return
        
segment _DATABEG
        
segment _DATA

segment _DATAEND

segment _BSS

segment _BSSEND

;segment STACK

