
global _get_base, _get_size, _get_tot_mem
extern _brksize, _end


%include "../h/model.h"


segment _TEXT
;========================================================================
;                           utilities                                     
;========================================================================

_get_base:			; return click at which prog starts
	mov ax,ds
	ret

_get_size:                      ; return prog size in bytes [text+data+bss]
	mov ax,_end             ; end is label at end of bss
	ret

; Find out how much memory the machine has, including vectors, kernel MM, etc.
_get_tot_mem:
	cli
	push es
	push di
	mov ax,8192		; start search at 128K [8192 clicks]
	sub di,di
y1:	mov es,ax 
        mov word[es:di],0A5A4h  ; write random bit pattern to memory
	xor bx,bx
	mov bx,[es:di]		; read back pattern just written
	cmp bx,0A5A4h		; compare with expected value
	jne y2			; if different, no memory present
	add ax,4096		; advance counter by 64K
	cmp ax,0A000h		; stop seaching at 640K
	jne y1
y2:	pop di
	pop es
	sti
	ret
        
segment _DATABEG
        
segment _DATA

segment _DATAEND

segment _BSS

segment _BSSEND

;segment STACK
