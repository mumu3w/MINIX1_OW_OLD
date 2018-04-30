
global _end, _brksize, _edata, _environ, kamikaze
extern _main, _exit

%include "../h/model.h"

STACKSIZE       EQU     5120	; default stack is 5 Kb (2K words)


; This is the C run-time start-off routine.  It's job is to take the
; arguments as put on the stack by EXEC, and to parse them and set them up the
; way main expects them.
;
segment _TEXT
..start:
$main:
        mov     ax,DGROUP       ; force relocation of data & bss
	mov	bx,sp		; set stackframe pointer (ds=ss)
	mov	cx,[bx]		; get argc
	add	bx,2		; point at next parameter
	mov	ax,cx
	inc	ax		; calculate envp
	shl	ax,1
	add	ax,bx
	mov	[_environ],ax	; save envp
	push	ax		; stack envp
	push	bx		; stack argv
	push	cx		; stack argc

	call	_main		; call main (arc,argv,envp)

	add	sp,6
	push	ax		; stack program-termination status
	call	_exit		; this will never return
        
        ; DEBUG from here on

kamikaze:       int 3
                ret
                
segment _DATABEG
_brksize:	DW	_end+2  ; dynamic changeable end of bss
_environ:	DW	0       ; save environment pointer here
  
segment _DATA

segment _DATAEND                ; DATAEND holds nothing. The label just
_edata:                         ; tells us where the initialized data
                                ; (.data) ends.
segment _BSS
        
segment _BSSEND                 ; BSSEND holds nothing. The label in
_end:                           ; the segment just tells us where
                                ; the data+bss ends.
segment STACK
;        resb STACKSIZE         ; add stack segment to bss
